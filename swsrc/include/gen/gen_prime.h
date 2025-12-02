#ifndef GEN_PRIME_HEADER
#define GEN_PRIME_HEADER

#include <share/types.h>

// 2^{ 8*(bytes) - 1 } < result < 2^{ 8*(bytes) }
intxx gen_odd_intxx(usize bytes);

// based on gen_odd_intxx
intxx get_prime_intxx(usize bytes);

#endif // GEN_PRIME_HEADER
