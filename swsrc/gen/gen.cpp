#include <iostream>
#include <gmpxx.h>

int main()
{
    mpz_class bigNum("123456789012345678901234567890");
    std::cout << bigNum << std::endl;
}
