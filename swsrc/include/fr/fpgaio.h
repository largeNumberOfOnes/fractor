#ifndef FPGA_IO_HEADER
#define FPGA_IO_HEADER

#include <share/types.h>

namespace fpgaio
{
    constexpr int32 max_curves      = 10;

    constexpr byte CMD_PING         = byte{0x01};
    constexpr byte CMD_SET_N        = byte{0x02};
    constexpr byte CMD_SET_ECM      = byte{0x03};
    constexpr byte CMD_CURVE        = byte{0x04};
    constexpr byte CMD_ABORT        = byte{0x05};
    constexpr byte CMD_STATUS       = byte{0x06};

    //RSP

    // exit(0) if exception
    void ping();
    void set_n(const intxx &n);
    void abort();
}

#endif // FPGA_IO_HEADER
