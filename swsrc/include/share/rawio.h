#ifndef RAWIO_HEADER
#define RAWIO_HEADER

#include <share/types.h>

#define RAWIO_ORDER 1
#define RAWIO_ENDIAN 1

// writes (size, number) in big-endian
void raw_write(const intxx &num, uint32_t size);

// reads (size, number) in big-endian
void raw_read(intxx &num, uint32_t &size);

#endif // RAWIO_HEADER
