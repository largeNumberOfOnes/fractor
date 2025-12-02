#ifndef RAWIO_HEADER
#define RAWIO_HEADER

#include <cstddef>
#include <gmpxx.h>

#define RAWIO_ORDER 1
#define RAWIO_ENDIAN 1

// writes (size, number) in big-endian
void raw_write(const mpz_class &num, uint32_t size);

// read (size, number) in big-endian
void raw_read(mpz_class &num, uint32_t &size);

#endif // RAWIO_HEADER
