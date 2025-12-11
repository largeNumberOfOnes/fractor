#include <algs/factor_ecm.h>
#include <algs/factor_qs.h>
#include <share/require.h>
#include <fr/fractors.h>
#include <fr/fpgaio.h>
#include <fr/comio.h>

bool QSFractor::handle
(
    const intxx &semiprime,
    intxx &left,
    intxx &right
)
{
    std::vector<intxx> result = factor_QS(semiprime);
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
    std::vector<intxx> result = factor_ECM(semiprime);
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
    std::cout << use_cpu << std::endl;

    return false;
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
