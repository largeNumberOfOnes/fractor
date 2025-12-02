#include <share/require.h>
#include <share/rawio.h>
#include <share/types.h>

void raw_write(const mpz_class &num, uint32 size)
{
    uint32 full_size = size + sizeof(size);
    char *buffer = new char[full_size];
    require(buffer, "Can't allocate memory to convert num into bytes.");
    buffer[0] = (size >> 24) & 0xff;
    buffer[1] = (size >> 16) & 0xff;
    buffer[2] = (size >> 8)  & 0xff;
    buffer[3] = (size >> 0)  & 0xff;
    mpz_export
    (
        buffer + sizeof(size),
        nullptr,
        RAWIO_ORDER,
        1,                  // item size (bytes)
        RAWIO_ENDIAN,
        0,                  // extra bits
        num.get_mpz_t()
    );
    std::cout.write(buffer, full_size);
    delete[] buffer;
}

void raw_read(mpz_class &num, uint32 &size)
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
    delete[] buffer;
}
