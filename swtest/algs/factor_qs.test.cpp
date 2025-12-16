#include "algs/factor_qs.h"

#include <iostream>
#include <vector>

#include "share/types.h"

void print_array(const std::vector<intxx>& arr)
{
    std::cout << "{\n";
    for (const auto& it : arr) {
        std::cout << "    " << it << ",\n";
    }
    std::cout << "  }" << std::endl;
}

bool comp_vec(
    const std::vector<intxx>& a,
    const std::vector<intxx>& b
)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (const auto& it : a)
    {
        int count_a = 0;
        int count_b = 0;
        for (usize q = 0; q < a.size(); ++q)
        {
            if (a[q] == it)
            {
                ++count_a;
            }
            if (b[q] == it)
            {
                ++count_b;
            }
        }
        if (count_a != count_b) {
            return false;
        }
    }
    return true;
}

void test1()
{
    const std::vector<
        std::pair<intxx, std::vector<intxx>>
    > test_data {
        {159, {53, 3}},
        { 77, {11, 7}},
        {10967535067, {104729, 104723}},
    };

    for (const auto& [n, ans] : test_data)
    {
        std::vector<intxx> ret = factor_QS(n);
        if (!comp_vec(ret, ans))
        {
            std::cout << "Error in test" << std::endl;
            std::cout << "  n = " << n << std::endl;
            std::cout << "  ans = ";
                print_array(ans);
            std::cout << "  ret = ";
                print_array(ret);
            break;
        }
    }
}

void test2() {
    const std::vector<
        // n, ans, (B, M)
        std::tuple<intxx, std::vector<intxx>, std::pair<int32, int32>>
    > test_data {
        {intxx{"1279111203059"}, {1273471, 1004429}, {100000, 100000}},
        // {
        //     intxx{"123456789012345678901234567890123456789"},
        //     {1273471, 1004429},
        //     {100000, 1000000}
        // },
    };

    for (const auto& [n, ans, pair] : test_data)
    {
        FactorQsError error_code; // DEV [should not ignored]
        constexpr int32 procs = 1;
        constexpr bool verbose = true;
        std::vector<intxx> ret =
            factor_QS_parm(n, pair.first, pair.second, procs, verbose, error_code);
        if (!comp_vec(ret, ans))
        {
            std::cout << "n = " << n << std::endl;
            std::cout << "ans = ";
                print_array(ans);
            std::cout << "ret = ";
                print_array(ret);
            break;
        }
    }
}

int main()
{
    test1();
    // test2();

    return 0;
}
