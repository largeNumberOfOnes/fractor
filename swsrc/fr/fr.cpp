#include <share/rawio.h>
#include <iostream>

int main()
{
    mpz_class bigNum;
    uint32_t size;
    raw_read(bigNum, size);

    std::cerr << "out: " << bigNum << std::endl;
}