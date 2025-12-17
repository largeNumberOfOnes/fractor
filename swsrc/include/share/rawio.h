#ifndef RAWIO_HEADER
#define RAWIO_HEADER

#include <share/types.h>

#define RAWIO_ORDER 1
#define RAWIO_ENDIAN 1

// writes (size, number) in big-endian
void raw_write(const intxx &num, uint32 size);

// reads (size, number) in big-endian
void raw_read(intxx &num, uint32 &size);

// writes only num in big-endian to buffer
void raw_bwrite(byte *buffer, const intxx &num);

// leading zeros
void raw_bwrite(byte *buffer, const intxx &num, uint32 size);

// reads only size bytes in big-endian from buffer to num
void raw_bread(const byte *buffer, intxx &num, uint32 size);

#endif // RAWIO_HEADER
