#ifndef FACTOR_ECM_HEADER
#define FACTOR_ECM_HEADER

#include <atomic>
#include <vector>

#include "share/types.h"

enum class FactorEcmError
{
    success, // Success
    no_found, // Too small B and С
};

struct Curve
{
    intxx x0;
    intxx y0;
    intxx a;
    intxx b;
};

// Return random curve coefficients
Curve generate_curve(const intxx& n);

// ret is empty list if no factors found
// B, C, curve_num values ​​for which the factorization was found
struct FactorECMReturn
{
    std::vector<intxx> ret;
    int32 B;
    int32 C;
    int32 attempts;
    FactorEcmError error;
};

// Factorize a number using the elliptic curve factorization method
// B -- Upper limit
// C -- Curves count
// procs -- processors count
// The variable "stop" is checked at each loop of the algorithm.
//     If it is true, the loop is terminated. After the algorithm is
//     completed, the variable will be true
FactorECMReturn factor_ECM_parm(
    const intxx& n,
    int32 B,
    int32 C,
    int32 procs,
    std::atomic<bool>& stop,
    bool verbose
);

// Factorize a number using the elliptic curve factorization method
FactorECMReturn factor_ECM_auto(
    const intxx& n,
    int32 procs,
    std::atomic<bool>& stop,
    bool verbose
);

// Factorize a number using the elliptic curve factorization method
// Parameters will be selected based on the length of the number
// The variable is checked at each loop of the algorithm. If it is true,
//     the loop is terminated. After the algorithm is completed, the
//     variable will be true
// May returns empty list if no factors found
std::vector<intxx> factor_ECM(const intxx& n);

// Factorize a number using the elliptic curve factorization method
// Parameters will be selected based on the length of the number
// procs -- processors count
// The variable is checked at each loop of the algorithm. If it is true,
//     the loop is terminated. After the algorithm is completed, the
//     variable will be true
// May returns empty list if no factors found
std::vector<intxx> factor_ECM_mt(
    const intxx& n,
    int32 procs,
    std::atomic<bool>& stop
);

#endif // FACTOR_ECM_HEADER
