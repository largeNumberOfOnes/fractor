#include <algs/factor_ecm.h>
#include <algs/factor_qs.h>
#include <share/require.h>
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

    raw_bwrite(buffer, semiprime, fpgaio::num_size);
    comio::send_packet(fd, fpgaio::CMD_SET_N, fpgaio::num_size, buffer);
    comio::receive_packet(fd, &recv_cmd, buffer);
    require(recv_cmd == fpgaio::RSP_ACK, "No ACK for SET_N");

    // todo: what ?
    comio::send_packet(fd, fpgaio::CMD_SET_ECM, 0, 0);
    comio::receive_packet(fd, &recv_cmd, buffer);
    require(recv_cmd == fpgaio::RSP_ACK, "No ACK for SET_ECM");

    for(int i = 0; i < fpgaio::max_curves; i++)
    {
        intxx A24 = 0;
        // todo: what ?
        comio::send_packet(fd, fpgaio::CMD_CURVE, 0, buffer);
        comio::receive_packet(fd, &recv_cmd, buffer);

        if(stop)
        {
            comio::send_packet(fd, fpgaio::CMD_ABORT, 0, buffer);
            break;
        }

        if(recv_cmd == fpgaio::RSP_FACTOR)
        {
            if(success.exchange(true))
                break;
            raw_bread(buffer, Z, fpgaio::num_size);
            left = gcd(Z, semiprime);
            stop.store(true);
            right = semiprime / left;
        }
    }

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
