#include <algs/factor_ecm.h>
#include <algs/factor_qs.h>
#include <share/require.h>
#include <fr/fractors.h>
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

    // init

    for(int i = 0; i < fpgaio::max_curves; i++)
    {
        // com

        if(stop)
        {
            // abort
            break;
        }

        if(1)
        {
            if(success.exchange(true))
                return;
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
