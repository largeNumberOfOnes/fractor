#include <gen/gen_prime.h>
#include <share/rawio.h>
#include <cxxopts.hpp>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

int main(int argc, char **argv)
{
    usize min_bytes = 0;
    usize max_bytes = 0;
    usize count = 0;
    bool verify = false;
    usize delay = 0;

    try
    {
        cxxopts::Options options("gen", "Utility for generation of semiprime numbers");

        options.add_options()
            ("h,help", "show help")
            ("min", "...", cxxopts::value<usize>())
            ("max", "...", cxxopts::value<usize>())
            ("v,verify", "save prime factors")
            ("c,count", "set count (default: inf)", cxxopts::value<usize>())
            ("d,delay", "delay in ms", cxxopts::value<usize>()->default_value("0"));
        options.parse_positional({"min", "max"});
        options.positional_help("min_bytes [max_bytes]");

        cxxopts::ParseResult result = options.parse(argc, argv);

        if(result.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if(result.count("min"))
            min_bytes = result["min"].as<usize>();
        else
            throw cxxopts::exceptions::missing_argument("min_bytes");

        if(result.count("max"))
            max_bytes = result["max"].as<usize>();
        else
            max_bytes = min_bytes;

        verify = result.count("verify");

        if(result.count("count"))
            count = result["count"].as<usize>();
        else
            count = -1; // inf

        if(result.count("delay"))
            delay = result["delay"].as<usize>();
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        std::cerr << "Args error: " << e.what() << std::endl;
        return 1;
    }

    gmp_randstate_t state;
    init_state(state);
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<usize> size_dist(min_bytes, max_bytes);
    std::chrono::milliseconds chrono_delay(delay);

    while(count > 0)
    {
        usize size = size_dist(gen);

        intxx first = gen_prime_intxx(4 * size, state);
        intxx second = gen_prime_intxx(4 * size, state);
        intxx semiprime = first * second;

        raw_write(semiprime, size);
        if(verify)
        {
            usize factor_size = (size + 1) / 2;
            raw_write(first, factor_size);
            raw_write(second, factor_size);
        }

        count--;

        if((count > 0) && (delay > 0))
        {
            std::cout.flush();
            std::this_thread::sleep_for(chrono_delay);
        }
    }
}
