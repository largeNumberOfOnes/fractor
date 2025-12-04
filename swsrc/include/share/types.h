#ifndef TYPES_HEADER
#define TYPES_HEADER

#include <type_traits>
#include <inttypes.h>
#include <cstddef>

#include <gmpxx.h>

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using float32 = float;
using float64 = double;
static_assert(sizeof(float32) == 4, "float32 must be 4 bytes");
static_assert(sizeof(float64) == 8, "float64 must be 8 bytes");

using byte  = std::byte;
using usize = uint64;

using intxx = mpz_class;
using floatxx = mpf_class;

constexpr usize EXPANDABLE_ARRAY = 0;

#endif // TYPES_HEADER
