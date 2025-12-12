#include <fr/fractors.h>
#include <share/rawio.h>
#include <cxxopts.hpp>
#include <iostream>
#include <iomanip>
#include <csignal>
#include <cmath>

namespace statistics
{
    static usize count = 0;
    static double sum = 0;
    static double square_sum = 0;

    void add_measurement(double value)
    {
        count++;
        sum += value;
        square_sum += value * value;
    }

    void show()
    {
        if(count == 0)
            return;

        double m = sum / count;
        double d = std::sqrt(square_sum / count - m*m);
        std::cout << "Mean:      " << m << " ms" << std::endl;
        std::cout << "Deviation: " << d << " ms" << std::endl;
    }

    void sigint_handler(int sig)
    {
        std::cout << std::endl;
        show();
        exit(0);
    }
}

int main(int argc, char **argv)
{
    std::signal(SIGINT, statistics::sigint_handler);

    usize output_width      = 25;
    bool verify             = false;
    bool show_time          = false;
    std::string com_port    = "/dev/ttyUSB0";
    FractorBase *fractor    = nullptr;
    uint32 baud_rate        = 115200;

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
            )
            (
                "p,port",
                "set com port for FPGA",
                cxxopts::value<std::string>()->default_value(
                    com_port
                )
            )
            (
                "b,baud",
                "set baud rate for com port",
                cxxopts::value<uint32>()->default_value(
                    std::to_string(baud_rate)
                )
            )
            (
                "m,mode",
                "set mode: qs|ecm|hw|share",
                cxxopts::value<std::string>()
            )
            (
                "n,nproc",
                "set number of software computing processes",
                cxxopts::value<uint32>()->default_value("1")
            );

        cxxopts::ParseResult flags = options.parse(argc, argv);

        if(flags.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if(flags.count("width"))
            output_width = flags["width"].as<usize>();

        verify      = flags.count("verify");
        show_time   = flags.count("time");

        if(flags.count("port"))
            com_port = flags["port"].as<std::string>();

        if(flags.count("baud"))
            baud_rate = flags["baud"].as<uint32>();

        if(flags.count("mode"))
        {
            std::string mode_str = flags["mode"].as<std::string>();
            if(mode_str == "qs")
            {
                fractor = new QSFractor();
            }
            else if(mode_str == "ecm")
            {
                fractor = new ECMFractor();
            }
            else if(mode_str == "hw")
            {
                fractor = new HeteroFractor(com_port, baud_rate, false);
            }
            else if(mode_str == "share")
            {
                fractor = new HeteroFractor(com_port, baud_rate, true);
            }
            else
            {
                std::cerr << "Incorrect mode option" << std::endl;
                return 1;
            }
        }
        else
        {
            std::cerr << "Mode option has no default value" << std::endl;
            return 1;
        }
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        std::cerr << "Args error: " << e.what() << std::endl;
        return 1;
    }

    usize half_output_width = (output_width + 1) / 2;
    intxx semiprime     = 0;
    intxx first         = 0;
    intxx second        = 0;
    intxx left          = 0;
    intxx right         = 0;
    uint32 size         = 0;
    uint32 factor_size  = 0;

    std::cout << std::right;
    while(std::cin.peek() != EOF)
    {
        raw_read(semiprime, size);

        auto start_time = std::chrono::high_resolution_clock::now();
        bool success = fractor->handle(semiprime, left, right);
        auto end_time = std::chrono::high_resolution_clock::now();
        using duration = std::chrono::duration<double, std::milli>;
        duration elapsed = end_time - start_time;
        if(show_time)
            statistics::add_measurement(elapsed.count());

        if(!success)
        {
            std::cerr << "Can't factor number(" << size;
            std::cerr << " bytes): " << semiprime << std::endl;
            statistics::show();
            return -1;
        }

        if(verify)
        {
            raw_read(first, factor_size);
            raw_read(second, factor_size);

            bool check_1 = (left == first) && (right == second);
            bool check_2 = (right == first) && (left == second);
            if(!(check_1 | check_2))
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
                std::cerr << std::setw(half_output_width) << left;
                std::cerr << " * " << std::setw(half_output_width);
                std::cerr << right << std::endl;
                statistics::show();
                return -1;
            }
        }

        std::cout << std::setw(output_width) << semiprime << " = ";
        std::cout << std::setw(half_output_width) << left << " * ";
        std::cout << std::setw(half_output_width) << right;

        if(show_time)
        {
            std::cout << "   + " << std::setw(10); 
            std::cout << static_cast<uint32>(elapsed.count()) << " ms";
        }

        std::cout<< std::endl;
    }
    statistics::show();
}
