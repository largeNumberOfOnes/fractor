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

class HeteroFractor : public FractorBase
{
private:
    int fd;
    bool use_cpu;

public:
    bool handle
    (
        const intxx &semiprime,
        intxx &left,
        intxx &right
    ) override;

    HeteroFractor
    (
        const std::string &dev,
        uint32 baud_rate,
        bool use_cpu
    );

    ~HeteroFractor() override;
};

#endif // FRACTORS_HEADER
