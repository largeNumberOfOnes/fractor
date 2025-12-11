#ifndef FRACTORS_HEADER
#define FRACTORS_HEADER

#include <fr/fractor_base.h>

class QSFractor : public FractorBase
{
public:
    bool handle
    (
        const intxx &semiprime,
        intxx &left,
        intxx &right
    ) override;
};

class ECMFractor : public FractorBase
{
public:
    bool handle
    (
        const intxx &semiprime,
        intxx &left,
        intxx &right
    ) override;
};

#endif // FRACTORS_HEADER
