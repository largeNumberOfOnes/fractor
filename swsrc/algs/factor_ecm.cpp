#include "algs/factor_ecm.h"

#include <iostream>
#include <optional>
#include <vector>
#include <thread>
#include <mutex>

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

static intxx div(const intxx& a, const intxx& b)
{
    intxx q = a / b;
    intxx r = a % b;
    if (r != 0 && ((a < 0) ^ (b < 0))) {
        q -= 1;
    }
    return q;
}

static intxx mod(const intxx& a, const intxx& b)
{
    intxx r = a % b;
    if (r != 0 && ((a < 0) ^ (b < 0))) {
        r += b;
    }
    return r;
}

static std::optional<intxx> mod_inverse(const intxx& a, const intxx& n)
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
        intxx quotient = div(old_r, r);
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

// Returns intxx size in bits
static usize intxx_size(const intxx& val) {
    return mpz_sizeinbase(val.get_mpz_t(), 2);
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

Curve generate_curve(const intxx& n) {
    static IntxxRandomGenerator generator;
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
    return {x0, y0, a, b};
}

static std::vector<intxx> factor(
    const intxx& n,
    intxx k,
    Curve vals
) {
    EllipticCurve curve{vals.a, vals.b, n};
    EllipticCurve::Point P{vals.x0, vals.y0};

    intxx del = 0;
    auto Q = curve.multiply(std::move(k), P, &del);
    if (!Q.has_value())
    {
        if (del != 0 && del != 1)
        {
            if (del != 0 && del != 1)
            {
                return {del, n / del};
            }
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

    return {};
}

std::vector<intxx> factor_ECM_parm(
    const intxx& n,
    int32 B,
    int32 C,
    int32 procs,
    std::atomic<bool>& stop,
    bool verbose,
    FactorEcmError& error_code
)
{
    error_code = FactorEcmError::success;

    std::mutex m{};
    std::vector<intxx> ret;
    int32 count = C;

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
    if (verbose)
    {
        std::cout << "Factorization with ECM\n"
                  << "  of n = " << n << " \n"
                  << "  of size " << intxx_size(n) / 8 << " bytes\n"
                  << "  B = " << B
                  << "  k = " << k
                  << "  size of k =  " << intxx_size(k) / 8 << " bytes"
                  << "  numbers of procs = " << procs
                  << std::endl;
    }

    auto task = [
        &n = std::as_const(n),
        &k = std::as_const(k),
        count = count,
        &stop = stop,
        &m = m,
        &ret = ret
    ]()
    {
        for (int curve_num = 0; curve_num < count; ++curve_num)
        {
            Curve vals = generate_curve(n);
            std::vector<intxx> lret = factor(n, k, std::move(vals));

            std::lock_guard<std::mutex> g{m};
            if (stop.load())
            {
                return;
            }
            if (!lret.empty())
            {
                ret = std::move(lret);
                stop.store(true);
                return;
            }

        }
    };

    std::vector<std::thread> threads;
    for (int q = 0; q < procs; ++q)
    {
        threads.push_back(std::thread(task));
    }
    for (int q = 0; q < procs; ++q)
    {
        threads[q].join();
    }
    if (ret.empty())
    {
        error_code = FactorEcmError::no_found;
    }
    if (verbose)
    {
        std::cout << "Found {";
        for (const auto& it : ret)
        {
            std::cout << it << " ";
        }
        std::cout << "}" << std::endl;
    }
    return ret;
}

std::vector<intxx> factor_ECM_mt(
    const intxx& n,
    int32 procs,
    std::atomic<bool>& stop
)
{
    FactorEcmError error_code;
    constexpr int attempts = 14;
    constexpr int32 B = 2000;
    constexpr int32 C = 10;
    for (int q = 0; q < attempts; ++q)
    {
        std::vector<intxx> ret = factor_ECM_parm(
            n,
            B << q,
            C << q,
            procs,
            stop,
            false,
            error_code
        );
        if (!ret.empty())
        {
            return ret;
        }
    }
    return {};
}

std::vector<intxx> factor_ECM(const intxx& n)
{
    FactorEcmError error_code;
    constexpr int attempts = 14;
    constexpr int32 B = 2000;
    constexpr int32 C = 10;
    constexpr int32 procs = 1;
    std::atomic<bool> stop{false};
    for (int q = 0; q < attempts; ++q)
    {
        std::vector<intxx> ret = factor_ECM_parm(
            n,
            B << q,
            C << q,
            procs,
            stop,
            false,
            error_code
        );
        if (!ret.empty())
        {
            return ret;
        }
    }
    return {};
}
