#ifndef FACTOR_QS_HEADER
#define FACTOR_QS_HEADER

#include <vector>

#include "share/types.h"

enum class FactorQsError {
    no_smoots, // Not enough smooth numbers
    no_deps, // No linear relationships found
};

// Factorize a number using the quadratic sieve method
// B -- Smoothness boundary
// M -- Sieve interval size
std::vector<intxx> factor_QS_parm(
    const intxx& n,
    int32 B,
    int32 M,
    FactorQsError& error_code
);

// Factorize a number using the quadratic sieve method
// Parameters will be selected based on the length of the number
// Error code omitted
std::vector<intxx> factor_QS(intxx n);

#endif // FACTOR_QS_HEADER
