#include "algs/factor_ecm.h"

#include <iostream>
#include <vector>

#include "share/types.h"

void print_array(const std::vector<intxx>& arr)
{
    std::cout << "{\n";
    for (const auto& it : arr)
    {
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
        if (count_a != count_b)
        {
            return false;
        }
    }
    return true;
}

void test()
{
    const std::vector<
        std::pair<intxx, std::vector<intxx>>
    > test_data {
        {159, {53, 3}},
        { 15, { 5, 3}},
        { 77, {11, 7}},
        {8051, {97, 83}},
        {1649, {17, 97}},
        {10967535067, {104729, 104723}},
        {1279111203059, {1273471, 1004429}},
    };

    for (const auto& [n, ans] : test_data)
    {
        std::vector<intxx> ret = factor_ECM(n);
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

int main()
{
    test();

    return 0;
}
