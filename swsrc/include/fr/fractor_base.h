#ifndef FRACTOR_BASE_HEADER
#define FRACTOR_BASE_HEADER

#include <share/types.h>

class FractorBase
{
protected:
    int32 nproc = 1;

public:
    // returns true if success
    virtual bool handle
    (
        const intxx &semiprime,
        intxx &left,
        intxx &right
    ) = 0;

    virtual ~FractorBase() = default;

    void set_nproc(int32 nproc)
    {
        this->nproc = nproc;
    }
};

#endif // FRACTOR_BASE_HEADER
