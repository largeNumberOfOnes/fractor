#ifndef GEN_PRIME_HEADER
#define GEN_PRIME_HEADER

#include <share/types.h>

#define GEN_PRIME_TEST_ITERATIONS_COUNT 25

// initializes state with the current time
void init_state(gmp_randstate_t &state);

// generates num inplace
// 2^{ 8*bytes - 1 } < num < 2^{ 8*bytes }
void gen_odd_intxx_inplace(intxx &num, usize bits, gmp_randstate_t &state);

// based on gen_odd_intxx_inplace
intxx gen_odd_intxx(usize bits, gmp_randstate_t &state);

// check whether a number is prime by 
// GEN_PRIME_TEST_ITERATIONS_COUNT iterations 
// of the Miller-Rabin test
bool is_prime(intxx &num);

// based on gen_odd_intxx_inplace
intxx gen_prime_intxx(usize bits, gmp_randstate_t &state);

#endif // GEN_PRIME_HEADER
