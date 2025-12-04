#include <algs/factor_qs.h>
#include <share/rawio.h>
#include <iostream>
#include <iomanip>

int main()
{
    usize output_width = 25;
    usize half_output_width = (output_width + 1) / 2;
    int32 qs_B = 10000;
    int32 qs_M = 50000;

    intxx semiprime     = 0;
    intxx first         = 0;
    intxx second        = 0;
    uint32_t size       = 0;

    FactorQsError qs_error;

    std::vector<intxx> result;
    while(std::cin)
    {
        raw_read(semiprime, size);
        result = factor_QS_parm(semiprime, qs_B, qs_M, qs_error);

        if(result.size() < 2)
        {
            std::cerr << "Can't factor number(" << size;
            std::cerr << " bytes): " << semiprime << std::endl;
            return - 1;
        }

        std::cout << std::right;
        std::cout << std::setw(output_width) << semiprime << " = ";
        std::cout << std::setw(half_output_width) << result[0] << " * ";
        std::cout << std::setw(half_output_width) << result[1] << std::endl;
    }
}
