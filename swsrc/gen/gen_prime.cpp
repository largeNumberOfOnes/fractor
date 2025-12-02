#include <gen/gen_prime.h>
#include <ctime>

void init_state(gmp_randstate_t &state)
{
    gmp_randinit_default(state);
    unsigned long seed = static_cast<unsigned long>(std::time(nullptr));
    gmp_randseed_ui(state, seed);
}

void gen_odd_intxx_inplace(intxx &num, usize bits, gmp_randstate_t &state)
{
    mpz_urandomb(num.get_mpz_t(), state, bits);
    mpz_setbit(num.get_mpz_t(), bits - 1);
    mpz_setbit(num.get_mpz_t(), 0);
}

intxx gen_odd_intxx(usize bits, gmp_randstate_t &state)
{
    intxx num;
    gen_odd_intxx_inplace(num, bits, state);
    return num;
}

bool is_prime(intxx &num)
{
    return mpz_probab_prime_p(num.get_mpz_t(), GEN_PRIME_TEST_ITERATIONS_COUNT);
}

intxx gen_prime_intxx(usize bits, gmp_randstate_t &state)
{
    intxx candidate;

    do gen_odd_intxx_inplace(candidate, bits, state);
    while(!is_prime(candidate));

    return candidate;
}
