#include "algs/factor_qs.h"

#include <unordered_map>
#include <utility>
#include <cassert>
#include <vector>
#include <cmath>
#include <set>

#include <gmpxx.h>

#include "share/types.h"

template<typename T>
using Matrix = std::vector<std::vector<T>>;

using FactorBase = std::vector<int32>;
using Factors = std::unordered_map<int32, int32>;
using SmoothNumber = std::tuple<intxx, intxx, Factors>;

intxx sqrt_intxx(const intxx& n)
{
    intxx sqrt_n;
    mpz_sqrt(sqrt_n.get_mpz_t(), n.get_mpz_t());
    return sqrt_n;
}

int legendre_symbol(const intxx& a, const intxx& p)
{
    return mpz_legendre(a.get_mpz_t(), p.get_mpz_t());
}

std::vector<int32> sieve_of_eratosthenes(int32 limit)
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
    for (int i = 3; i <= limit; ++i)
    {
        if (is_prime[i])
        {
            primes.push_back(i);
        }
    }

    return primes;
}

std::vector<int32> find_factor_base(const intxx& n, int32 B)
{
    std::vector<int32> primes = sieve_of_eratosthenes(B);
    std::vector<int32> factor_base;
    for (int32 p : primes)
    {
        if (legendre_symbol(n, p) == 1)
        {
            factor_base.push_back(p);
        }
    }
    return factor_base;
}

intxx integer_power(intxx a, int32 power)
{
    intxx b = a;
    for (int32 q = 1; q < power; ++q)
    {
        b *= a;
    }
    return b;
}

intxx field_sqrt(intxx n, int32 p)
{
    assert(2 < p && p % 2 == 1); // DEV
    assert(integer_power(n, (p - 1) / 2) % p == 1); // DEV

    if (p % 4 == 3)
    {
        return integer_power(n, (p + 1) / 4) % p;
    }

    int32 s = 0;
    int32 q = p - 1;
    while (q % 2 == 0)
    {
        ++s;
        q >>= 1;
    }

    int32 z = 2;
    while ((integer_power(z, (p - 1) / 2) % p) != p - 1)
    {
        ++z;
    }

    intxx c = integer_power(z, q) % p;
    intxx r = integer_power(n, (q + 1) / 2) % p;
    intxx t = integer_power(n, q) % p;

    int32 m = s;
    while (t != 1)
    {
        int i = 1;
        intxx temp = t;
        for (; i < m; ++i)
        {
            temp = (temp * temp) % p;
            if (temp == 1)
            {
                break;
            }
        }
        if (i == 0)
        {
            break;
        }

        intxx b = integer_power(c, 1 << (m - i - 1)) % p;
        r = (r * b) % p;
        t = (t * b * b) % p;
        c = (b * b) % p;
        m = i;
    }

    return r;
}

// Q(x) = (x + m)^2 - n = 0 (mod p) => (x + m)^2 = n (mod p), m = sqrt_n
std::vector<intxx> find_Qx_roots(
    const intxx& n,
    const intxx& sqrt_n,
    int32 p
)
{
    if (p == 2)
    {
        if (n % 2 == 1)
        {
            return {0};
        }
        else
        {
            return {};
        }
    }

    if (legendre_symbol(n, p) != 1)
    {
        return {};
    }

    intxx t = field_sqrt(n % p, p);

    intxx root1 = (t - sqrt_n)  % p;
    intxx root2 = (-t - sqrt_n) % p;

    if (root1 == root2)
    {
        return {root1};
    }
    return {root1, root2};
}

Matrix<int32> gaussian_elimination_mod2(const Matrix<int32>& matrix)
{
    usize m = matrix.size();
    usize n = matrix[0].size();

    Matrix<int32> A{matrix};
    Matrix<int32> row_ops(m, std::vector<int32>{});

    for (int col = 0; col < n; ++col)
    {
        int pivot = -1;
        for (int row = col; row < m; ++row)
        {
            if (A[row][col] == 1)
            {
                pivot = row;
                break;
            }
        }
        if (pivot == -1)
        {
            continue;
        }

        if (pivot != col)
        {
            std::swap(A[col], A[pivot]);
            std::swap(row_ops[col], row_ops[pivot]);
        }

        for (int row = 0; row < m; ++row)
        {
            if (row != col and A[row][col] == 1)
            {
                for (int c = 0; c < n; ++c)
                {
                    A[row][c] ^= A[col][c];
                }
                row_ops[row].push_back(col);
            }
        }
    }

    Matrix<int32> dependencies;
    for (int q = 0; q < m; ++q)
    {
        bool cond = true;
        for (auto x : A[q])
        {
            if (x != 0)
            {
                cond = false;
                break;
            }
        }
        if (cond)
        {
            std::set<int32> dep{q};
            for (auto op : row_ops[q])
            {
                dep.insert(op);
            }
            std::vector<int32> lis;
            for (int32 op : dep)
            {
                lis.push_back(op);
            }
            dependencies.push_back(std::move(lis));
        }
    }

    return dependencies;
}

Factors factor_over_base(intxx num, const FactorBase& factor_base)
{
    Factors factors;
    intxx temp = abs(num);

    if (num < 0)
    {
        factors[-1] = 1;
    }

    for (auto p : factor_base)
    {
        if (p == -1)
        {
            continue;
        }
        while (temp % p == 0)
        {
            factors[p] = factors.count(p) ? factors[p] + 1 : 0;
            temp /= p;
        }
    }

    if (temp != 1)
    {
        return {};
    }

    return factors;
}

std::vector<SmoothNumber> find_smooth_numbers(
    intxx n, int32 B, int32 M, const FactorBase& factor_base
)
{
    intxx sqrt_n = sqrt_intxx(n);
    std::vector<float64> sieve_array(2 * M + 1, 0.0);

    for (auto p : factor_base)
    {
        float64 log_p = std::log(p);
        auto roots = find_Qx_roots(n, sqrt_n, p);
        for (const auto& root : roots)
        {
            intxx start_ = (M + root) % p;
            int32 start = start_.get_si();
            for (int32 q = start; q < 2 * M + 1; q += p)
            {
                sieve_array[q] += log_p;
            }
        }
    }

    float64 threshold = std::log(B) * 1.5;
    std::vector<SmoothNumber> smooth_numbers;
    for (int32 x = -M; x < M + 1; ++x)
    {
        int32 idx = x + M;
        intxx Q_x = (x + sqrt_n) * (x + sqrt_n) - n;

        if (Q_x == 0)
        {
            continue;
        }
        float64 val = std::abs(
            sieve_array[idx] - std::log(std::abs(Q_x.get_d()))
        );

        if (val < threshold)
        {
            auto factors = factor_over_base(Q_x, factor_base);
            if (factors.size())
            {
                smooth_numbers.emplace_back(
                    x,
                    std::move(Q_x),
                    std::move(factors)
                );
            }
        }
    }

    return smooth_numbers;
}

Matrix<int32> build_exponent_matrix(
    const FactorBase& factor_base,
    const std::vector<SmoothNumber>& smooth_numbers
)
{
    Matrix<int32> matrix;
    for (const auto& [x, Qx, factors] : smooth_numbers)
    {
        std::vector<int32> vec;
        if (factors.count(-1))
        {
            vec.push_back(factors.at(-1) % 2);
        }
        else
        {
            vec.push_back(0);
        }

        for (auto p : factor_base)
        {
            if (p == 2)
            {
                continue;
            }
            vec.push_back(
                factors.count(p) ? factors.at(p) % 2 : 0
            );
        }

        matrix.push_back(vec);
    }

    return matrix;
}

intxx find_devider(
    const intxx& n,
    const std::vector<SmoothNumber>& smooth_numbers,
    const Matrix<int32>& dependencies
)
{
    intxx sqrt_n = sqrt_intxx(n);

    for (const auto& dep_indices : dependencies)
    {
        intxx X = 1;
        std::unordered_map<int32, int32> Y_factors;

        for (int32 idx : dep_indices)
        {
            const auto& [x, Q_x, factors] = smooth_numbers[idx];
            X = (X * (x + sqrt_n)) % n;

            for (const auto& [key, value] : factors)
            {
                if (key == -1)
                {
                    continue;
                }
                if (Y_factors.count(key))
                {
                    Y_factors[key] = 0;
                }
                Y_factors[key] += value;
            }
        }

        bool all_even = true;
        for (const auto& [key, value] : Y_factors)
        {
            if (value % 2 != 0) {
                all_even = true;
                break;
            }
        }
        if (!all_even)
        {
            continue;
        }

        intxx Y = 1;
        for (const auto& [key, value] : Y_factors)
        {
            Y = (Y * (integer_power(key, value / 2) % n)) % n;
        }

        intxx d1 = gcd(X - Y, n);
        intxx d2 = gcd(X + Y, n);

        if (d1 != 1 && d1 != n)
        {
            return d1;
        }
        else if (d2 != 1 && d2 != n)
        {
            return d2;
        }
    }

    return 1;
}

std::vector<intxx> factor_QS_parm(
    const intxx& n,
    int32 B,
    int32 M,
    FactorQsError& error_code
)
{
    FactorBase factor_base = find_factor_base(n, B);

    std::vector<SmoothNumber> smooth_numbers =
                            find_smooth_numbers(n, B, M, factor_base);
    if (smooth_numbers.size() < factor_base.size() + 10)
    {
        error_code = FactorQsError::no_smoots;
        return {};
    }

    Matrix<int32> matrix = build_exponent_matrix(
        factor_base,
        smooth_numbers
    );

    Matrix<int32> dependencies = gaussian_elimination_mod2(matrix);
    if (!dependencies.size())
    {
        error_code = FactorQsError::no_deps;
        return {};
    }

    intxx d = find_devider(n, smooth_numbers, dependencies);

    return {d, n / d};
}

std::vector<intxx> factor_QS(intxx n)
{
    int32 B = 1000;
    int32 M = 5000;
    FactorQsError error_code;
    return factor_QS_parm(n, B, M, error_code);
}
