#include <cstddef>
#include <inttypes.h>
#include <type_traits>
#include <gmpxx.h>

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using float32 = float;  // DEV [not garantued]
using float64 = double; // DEV [not garantued]

using byte  = std::byte;
using usize = uint64;

using intxx = mpz_class;

constexpr usize EXPANDABLE_ARRAY = 0;

