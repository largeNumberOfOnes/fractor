#include <share/require.h>
#include <share/rawio.h>

void raw_write(const intxx &num, uint32 size)
{
    uint32 full_size = size + sizeof(size);
    char *buffer = new char[full_size];
    require(buffer, "Can't allocate memory to convert num into bytes.");
    buffer[0] = (size >> 24) & 0xff;
    buffer[1] = (size >> 16) & 0xff;
    buffer[2] = (size >> 8)  & 0xff;
    buffer[3] = (size >> 0)  & 0xff;
    raw_bwrite(reinterpret_cast<byte *>(buffer) + sizeof(size), num);
    std::cout.write(buffer, full_size);
    delete[] buffer;
}

void raw_read(intxx &num, uint32 &size)
{
    char size_buffer[4];
    std::cin.read(size_buffer, 4);
    size =  (static_cast<uint32>(size_buffer[3]) << 0) +
            (static_cast<uint32>(size_buffer[2]) << 8) +
            (static_cast<uint32>(size_buffer[1]) << 16) +
            (static_cast<uint32>(size_buffer[0]) << 24);
    char *buffer = new char[size];
    require(buffer, "Can't allocate memory to read num.");
    std::cin.read(buffer, size);
    raw_bread(reinterpret_cast<byte *>(buffer), num, size);
    delete[] buffer;
}

void raw_bwrite(byte *buffer, const intxx &num)
{
    mpz_export
    (
        buffer,
        nullptr,
        RAWIO_ORDER,
        1,                  // item size (bytes)
        RAWIO_ENDIAN,
        0,                  // extra bits
        num.get_mpz_t()
    );
}

void raw_bwrite(byte *buffer, const intxx &num, uint32 size)
{
    usize bits = mpz_sizeinbase(num.get_mpz_t(), 2);
    usize bytes = (bits + 7) / 8;
    require(bytes < size, "Try to write too long number");

    usize delta = size - bytes;
    std::memset(buffer, 0, delta);
    raw_bwrite(buffer + delta, num);
}

void raw_bread(const byte *buffer, intxx &num, uint32 size)
{
    mpz_import
    (
        num.get_mpz_t(),
        size,
        RAWIO_ORDER,
        1,                  // item size (bytes)
        RAWIO_ENDIAN,
        0,                  // extra bits
        buffer
    );
}
