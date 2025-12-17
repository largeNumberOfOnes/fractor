#include <algs/factor_ecm.h>
#include <algs/factor_qs.h>
#include <share/require.h>
#include <gen/gen_prime.h>
#include <fr/fractors.h>
#include <share/rawio.h>
#include <fr/fpgaio.h>
#include <fr/comio.h>
#include <thread>

bool QSFractor::handle
(
    const intxx &semiprime,
    intxx &left,
    intxx &right
)
{
    std::vector<intxx> result = factor_QS_mt(semiprime, nproc);
    if(result.size() != 2)
        return false;

    left = result[0];
    right = result[1];
    return true;
}

bool ECMFractor::handle
(
    const intxx &semiprime,
    intxx &left,
    intxx &right
)
{
    std::atomic<bool> stop{false};
    std::vector<intxx> result = factor_ECM_mt(semiprime, nproc, stop);
    if(result.size() != 2)
        return false;

    left = result[0];
    right = result[1];
    return true;
}

bool HeteroFractor::handle
(
    const intxx &semiprime,
    intxx &left,
    intxx &right
)
{
    std::atomic<bool> success{false};
    std::atomic<bool> stop{false};
    std::vector<intxx> result;
    std::thread soft_thread;

    if(use_cpu)
    {
        soft_thread = std::thread(
        [
            &semiprime,
            &stop,
            &result,
            &success,
            &left,
            &right
        ]
        (int32 nproc)
        {
            std::vector<intxx> soft_result = factor_ECM_mt
            (
                semiprime,
                nproc,
                stop
            );
            if(soft_result.size() == 2)
            {
                if(success.exchange(true))
                    return;
                left    = result[0];
                right   = result[1];
            }
        }, nproc);
    }

    byte buffer[fpgaio::io_buffer_size] = {};
    byte recv_cmd{0};
    intxx Z = 0;

    comio::send_packet(fd, fpgaio::CMD_PING, 0, buffer);
    comio::receive_packet(fd, &recv_cmd, buffer);
    require(recv_cmd == fpgaio::RSP_PING, "Incorrect ping response");

    intxx d512 = 1;
    intxx d32 = 1;
    d512 <<= 512;
    d32  <<= 32;
    intxx d512N;
    intxx inv32;
    gmp_randstate_t state;
    init_state(state);
    mpz_mod(d512N.get_mpz_t(), d512.get_mpz_t(), semiprime.get_mpz_t());
    mpz_invert(inv32.get_mpz_t(), semiprime.get_mpz_t(), d32.get_mpz_t());
    raw_bwrite(buffer, semiprime, fpgaio::num_size);
    raw_bwrite(buffer + fpgaio::num_size, d32 - inv32, fpgaio::num_size);
    raw_bwrite(buffer + 2 * fpgaio::num_size, d512N, fpgaio::num_size);
    comio::send_packet
    (
        fd,
        fpgaio::CMD_SET_N,
        3 * fpgaio::num_size,
        buffer
    );
    comio::receive_packet(fd, &recv_cmd, buffer);
    require(recv_cmd == fpgaio::RSP_ACK, "No ACK for SET_N");

    int fpga_curves = 0;
    intxx A24 = 0;
    intxx x = 0;
    while(!stop)
    {
        while(fpga_curves < fpgaio::max_ladders)
        {
            mpz_urandomb(A24.get_mpz_t(), state, fpgaio::num_size * 8);
            mpz_urandomb(x.get_mpz_t(), state, fpgaio::num_size * 8);
            raw_bwrite
            (
                buffer,
                A24 % semiprime,
                fpgaio::num_size
            );
            raw_bwrite
            (
                buffer + fpgaio::num_size,
                x % semiprime,
                fpgaio::num_size
            );
            comio::send_packet
            (
                fd,
                fpgaio::CMD_CURVE,
                2 * fpgaio::num_size,
                buffer
            );
            fpga_curves++;
        }

        if(fpga_curves > 0)
            comio::receive_packet(fd, &recv_cmd, buffer);
        else
            break;
        if(recv_cmd == fpgaio::RSP_FACTOR)
        {
            if(success.exchange(true))
                break;
            raw_bread(buffer, Z, fpgaio::num_size);
            left = gcd(Z, semiprime);
            stop.store(true);
            right = semiprime / left;
        }
        fpga_curves--;
    }

    comio::send_packet(fd, fpgaio::CMD_ABORT, 0, buffer);
    if(use_cpu)
        soft_thread.join();
    return success;
}

HeteroFractor::HeteroFractor
(
    const std::string &dev,
    uint32 baud_rate,
    bool use_cpu
) : use_cpu(use_cpu)
{
    fd = comio::open(dev, baud_rate);
}

HeteroFractor::~HeteroFractor()
{
    comio::close(fd);
}
