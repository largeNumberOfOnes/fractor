#include "algs/factor_ecm.h"

#include <optional>
#include <vector>

#include <gmpxx.h>

#include "share/types.h"

class IntxxRandomGenerator
{
    private:
        gmp_randstate_t state;

    public:
        IntxxRandomGenerator(int32 seed = 7)
        {
            gmp_randinit_mt(state);
            gmp_randseed_ui(state, seed);
        }

        intxx generate(const intxx& min, const intxx& max)
        {
            intxx result;
            intxx wide = max - min;
            mpz_urandomm(result.get_mpz_t(), state, wide.get_mpz_t());
            result += min;
            return result;
        }
};

intxx div(const intxx& a, const intxx& b)
{
    intxx q = a / b;
    intxx r = a % b;
    if (r != 0 && ((a < 0) ^ (b < 0))) {
        q -= 1;
    }
    return q;
}

intxx mod(const intxx& a, const intxx& b)
{
    intxx r = a % b;
    if (r != 0 && ((a < 0) ^ (b < 0))) {
        r += b;
    }
    return r;
}

std::optional<intxx> mod_inverse(const intxx& a, const intxx& n)
{
    if (gcd(a, n) != 1)
    {
        return std::nullopt;
    }

    intxx old_r = a; intxx r = n;
    intxx old_s = 1; intxx s = 0;
    intxx old_t = 0; intxx t = 1;

    while (r != 0)
    {
        // intxx quotient = old_r / r;
        intxx quotient = div(old_r, r); // DEV
        // std::cout << "s " << s << std::endl
        //           << "old_s " << old_s << std::endl
        //           << "quotient " << quotient << std::endl;
        // old_r = std::exchange(r, old_r - quotient * r);
        // old_s = std::exchange(s, old_s - quotient * s);
        // old_t = std::exchange(t, old_t - quotient * t);
        auto fn = +[](intxx& old, intxx& val, intxx& quotient)
        {
            intxx temp_old = val;
            intxx temp_var = old - quotient * val;
            old = temp_old;
            val = temp_var;
        };
        fn(old_r, r, quotient);
        fn(old_s, s, quotient);
        fn(old_t, t, quotient);
    }

    return mod(old_s, n);
}

static std::vector<int32> sieve_of_eratosthenes(int32 limit)
{
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;

    for (int q = 2; q <= limit; ++q)
    {
        if (is_prime[q])
        {
            for (int w = 2 * q; w <= limit; w += q)
            {
                is_prime[w] = false;
            }
        }
    }

    std::vector<int> primes;
    for (int i = 2; i <= limit; ++i)
    {
        if (is_prime[i])
        {
            primes.push_back(i);
        }
    }

    return primes;
}

class EllipticCurve
{
    intxx a;
    intxx b;
    intxx n;

    public:
        EllipticCurve(intxx a, intxx b, intxx n)
            : a(std::move(a))
            , b(std::move(b))
            , n(std::move(n))
        {
            this->a %= this->n;
            this->b %= this->n;
        }

        using Point = std::tuple<intxx, intxx>;

        // May return 'std::nullopt' not only in case of an error, but
        //     also when a divisor is found. In this case, it is stored in
        //     '*del'. The 'del' parameter may be equal to 'std::nullptr'.
        //     In this case, 'std::nullopt' will still be returned when a
        //     divisor is found, but the 'del' parameter will be ignored.
        std::optional<Point> add(
            const std::optional<Point>& P,
            const std::optional<Point>& Q,
            intxx* del
        )
        {
            if (!P.has_value())
            {
                return Q;
            }
            if (!Q.has_value())
            {
                return P;
            }

            const auto& [x1, y1] = P.value();
            const auto& [x2, y2] = Q.value();
            if (x1 == x2 && (y1 + y2) % n == 0)
            {
                return std::nullopt;
            }

            intxx numerator;
            intxx denominator;
            if (P == Q)
            {
                numerator = (3 * x1 * x1 + a) % n;
                denominator = (2 * y1) % n;
            }
            else
            {
                numerator = (y2 - y1) % n;
                denominator = (x2 - x1) % n;
            }
            if (denominator == 0)
            {
                return std::nullopt;
            }

            auto inv = mod_inverse(denominator, n);
            if (!inv.has_value())
            {
                if (del)
                {
                    *del = gcd(denominator, n);
                }
                return std::nullopt;
            }

            intxx lam = (numerator * inv.value()) % n;
            intxx x3 = (lam * lam - x1 - x2) % n;
            intxx y3 = (lam * (x1 - x3) - y1) % n;

            return Point{x3, y3};
        }

    // For return value policy look at 'add' method.
    std::optional<Point> multiply(intxx k, const Point& P, intxx* del)
    {
        std::optional<Point> result;
        std::optional<Point> current = P;


        while (0 < k)
        {
            if ((k & 1) != 0)
            {
                result = add(result, current, del);
            }
            current = add(current, current, del);
            k >>= 1;
        }

        return result;
    }
};

std::vector<intxx> factor_ECM_parm(
    const intxx& n,
    int32 B,
    int32 C,
    FactorEcmError& error_code
)
{
    error_code = FactorEcmError::success;
    IntxxRandomGenerator generator;
    for (int curve_num = 0; curve_num < C; ++curve_num)
    {
        intxx x0;
        intxx y0;
        intxx a;
        intxx b;
        while (true)
        {
            x0 = generator.generate(0, n - 1);
            y0 = generator.generate(0, n - 1);
            a  = generator.generate(0, n - 1);
            b = (y0 * y0 - x0 * x0 * x0 - a * x0) % n;
            intxx disc = (4 * a * a * a + 27 * b * b) % n;
            if (gcd(disc, n) == 1)
            {
                break;
            }
        }

        EllipticCurve curve{a, b, n};
        EllipticCurve::Point P{x0, y0};

        intxx k = 1;
        std::vector<int32> primes = sieve_of_eratosthenes(B);
        for (int32 p : primes)
        {
            int32 power = p;
            while (power <= B)
            {
                k *= p;
                power *= p;
            }
        }

        intxx del = 0;
        auto Q = curve.multiply(k, P, &del);
        if (!Q.has_value())
        {
            if (del != 0 && del != 1)
            {
                return {del, n / del};
            }
            else
            {
                // Special case: point at infinity
                // It may mean that the order of the point divides k
                // Further verification is required
                // And he knows what needs to be done...
                // pass // DEV
            }
        }
    }

    error_code = FactorEcmError::no_found;
    return {};
}

std::vector<intxx> factor_ECM(const intxx& n)
{
    FactorEcmError error_code;
    int32 B = 2000;
    int32 C = 10;
    return factor_ECM_parm(n, B, C, error_code);
}
