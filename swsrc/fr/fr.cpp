#include <algs/factor_qs.h>
#include <share/rawio.h>
#include <cxxopts.hpp>
#include <iostream>
#include <iomanip>

int main(int argc, char **argv)
{
    usize output_width      = 25;
    bool verify             = false;
    bool show_delta_time    = false;

    try
    {
        cxxopts::Options options(
            "fr",
            "Utility for factorization of semiprime numbers"
        );

        options.add_options()
            ("h,help", "show help")
            ("v,verify", "check prime factors")
            ("t,time", "show time of each factorization")
            (
                "w,width",
                "set output width", 
                cxxopts::value<usize>()->default_value(
                    std::to_string(output_width)
                )
            );

        cxxopts::ParseResult flags = options.parse(argc, argv);

        if(flags.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if(flags.count("width"))
            output_width = flags["width"].as<usize>();

        verify          = flags.count("verify");
        show_delta_time = flags.count("time");
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        std::cerr << "Args error: " << e.what() << std::endl;
        return 1;
    }

    usize half_output_width = (output_width + 1) / 2;


    int32 qs_B = 1000;
    int32 qs_M = 5000;

    intxx semiprime     = 0;
    intxx first         = 0;
    intxx second        = 0;
    uint32 size         = 0;
    uint32 factor_size  = 0;

    FactorQsError qs_error;

    std::vector<intxx> result;
    std::cout << std::right;
    while(std::cin)
    {
        raw_read(semiprime, size);
        auto start_time = std::chrono::high_resolution_clock::now();
        result = factor_QS_parm(semiprime, qs_B, qs_M, qs_error);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

        if(result.size() < 2)
        {
            std::cerr << "Can't factor number(" << size;
            std::cerr << " bytes): " << semiprime << std::endl;
            return -1;
        }

        if(verify)
        {
            raw_read(first, factor_size);
            raw_read(second, factor_size);

            bool check_1 = (result[0] == first) && (result[1] == second);
            bool check_2 = (result[1] == first) && (result[0] == second);
            if(check_1 | check_2)
            {

            }
            else
            {
                std::cerr << "Bad factorization:" << std::endl;
                std::cerr << "    input(" << size << " bytes): ";
                std::cerr << std::setw(output_width);
                std::cerr << semiprime << std::endl;
                std::cerr << "    expected: ";
                std::cerr << std::setw(half_output_width) << first;
                std::cerr << " * " << std::setw(half_output_width);
                std::cerr << second << std::endl;
                std::cerr << "    given:    ";
                std::cerr << std::setw(half_output_width) << result[0];
                std::cerr << " * " << std::setw(half_output_width);
                std::cerr << result[1] << std::endl;
                return 0;
            }
        }

        std::cout << std::setw(output_width) << semiprime << " = ";
        std::cout << std::setw(half_output_width) << result[0] << " * ";
        std::cout << std::setw(half_output_width) << result[1];
        
        if(show_delta_time)
            std::cout << "   + " << std::setw(10); 
            std::cout << static_cast<uint32>(elapsed.count()) << " ms";

        std::cout<< std::endl;
    }
}
