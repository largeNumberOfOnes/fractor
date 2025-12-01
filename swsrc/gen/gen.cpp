#include <iostream>
#include <cstddef>
#include <gmpxx.h>
#include <time.h>

// writes (size, number) in big-endian
void raw_write(const mpz_class &num, uint32_t size)
{
    uint32_t full_size = size + sizeof(size);
    char *buffer = new char[full_size];
    buffer[0] = (size >> 24) & 0xff;
    buffer[1] = (size >> 16) & 0xff;
    buffer[2] = (size >> 8)  & 0xff;
    buffer[3] = (size >> 0)  & 0xff;
    mpz_export(buffer + sizeof(size), nullptr, 1, 1, 1, 0, num.get_mpz_t());
    std::cout.write(buffer, full_size);
}

int main()
{
    size_t size = 16;
    gmp_randclass gmp_rand(gmp_randinit_default);
    gmp_rand.seed(time(nullptr));
    mpz_class bigNum = gmp_rand.get_z_bits(8*size);

    raw_write(bigNum, size);
}
