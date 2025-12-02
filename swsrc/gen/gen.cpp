#include <gen/gen_prime.h>
#include <share/rawio.h>
#include <iostream>

int main()
{
    size_t size = 10;
    gmp_randstate_t state;
    init_state(state);
    intxx bigNum = gen_prime_intxx(size, state);

    std::cerr << "in: " << bigNum << std::endl;
    raw_write(bigNum, size);
}
