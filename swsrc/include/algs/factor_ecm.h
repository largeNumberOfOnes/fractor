#ifndef FACTOR_QS_HEADER
#define FACTOR_QS_HEADER

#include <vector>

#include "share/types.h"

enum class FactorEcmError {
    success, // Success
    no_found, // Too small B and С
    special_case, // Unrecognized error
};

// Factorize a number using the elliptic curve factorization method
// B -- Upper limit
// C -- Curves count
// May returns empty list if no factors found
std::vector<intxx> factor_ECM_parm(
    const intxx& n,
    int32 B,
    int32 C,
    FactorEcmError& error_code
);

// Factorize a number using the elliptic curve factorization method
// Parameters will be selected based on the length of the number
// May returns empty list if no factors found
// Error code omitted
std::vector<intxx> factor_ECM(const intxx& n);

#endif // FACTOR_QS_HEADER
