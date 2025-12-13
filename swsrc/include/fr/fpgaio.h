#ifndef FPGA_IO_HEADER
#define FPGA_IO_HEADER

#include <share/types.h>

namespace fpgaio
{
    constexpr int32 max_curves      = 10;
    constexpr int32 io_buffer_size  = 32;
    constexpr int32 num_size        = 32;

    constexpr byte CMD_PING         = byte{0x01};
    constexpr byte CMD_SET_N        = byte{0x02};
    constexpr byte CMD_SET_ECM      = byte{0x03};
    constexpr byte CMD_CURVE        = byte{0x04};
    constexpr byte CMD_ABORT        = byte{0x05};
    constexpr byte CMD_STATUS       = byte{0x06};

    constexpr byte RSP_PING         = byte{0x81};
    constexpr byte RSP_ACK          = byte{0x82};
    constexpr byte RSP_STATUS       = byte{0x83};
    constexpr byte RSP_OK           = byte{0x84};
    constexpr byte RSP_FACTOR       = byte{0x85};
    constexpr byte RSP_ERROR        = byte{0x86};
}

#endif // FPGA_IO_HEADER
