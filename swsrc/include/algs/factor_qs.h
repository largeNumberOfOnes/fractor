#ifndef FACTOR_QS_HEADER
#define FACTOR_QS_HEADER

#include <vector>

#include "share/types.h"

enum class FactorQsError {
    success, // Success
    no_smoots, // Not enough smooth numbers
    no_deps, // No linear relationships found
};

// Factorize a number using the quadratic sieve method
// B -- Smoothness boundary
// M -- Sieve interval size
// May returns empty list if no factors found
std::vector<intxx> factor_QS_parm(
    const intxx& n,
    int32 B,
    int32 M,
    bool verbose,
    FactorQsError& error_code
);

// Factorize a number using the quadratic sieve method
// Parameters will be selected based on the length of the number
// May returns empty list if no factors found
// Error code omitted, verbose = false
std::vector<intxx> factor_QS(const intxx& n);

#endif // FACTOR_QS_HEADER
