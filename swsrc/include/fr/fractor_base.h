#ifndef FRACTOR_BASE_HEADER
#define FRACTOR_BASE_HEADER

#include <share/types.h>

class FractorBase
{
public:
    // returns true if success
    virtual bool handle
    (
        const intxx &semiprime,
        intxx &left,
        intxx &right
    ) = 0;

    virtual ~FractorBase() = default;
};

#endif // FRACTOR_BASE_HEADER
