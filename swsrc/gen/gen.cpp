#include <share/rawio.h>
#include <time.h>

int main()
{
    size_t size = 16;
    gmp_randclass gmp_rand(gmp_randinit_default);
    gmp_rand.seed(time(nullptr));
    mpz_class bigNum = gmp_rand.get_z_bits(8*size);

    raw_write(bigNum, size);
}
