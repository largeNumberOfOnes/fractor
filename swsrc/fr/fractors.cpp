#include <algs/factor_ecm.h>
#include <algs/factor_qs.h>
#include <share/require.h>
#include <fr/fractors.h>

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
