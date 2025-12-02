#ifndef REQUIRE_CONDITION_HEADER
#define REQUIRE_CONDITION_HEADER

#include <iostream>

inline void require(bool condition, const char *message)
{
    if(condition)
        return;

    std::cerr << message << std::endl;
    exit(0);
}

#endif // REQUIRE_CONDITION_HEADER
