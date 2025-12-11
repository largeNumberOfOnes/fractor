#ifndef COMIO_HEADER
#define COMIO_HEADER

#include <share/types.h>

namespace comio
{
    int open(const std::string &dev, uint32 baud_rate);

    void send_packet(int fd, byte cmd, uint16 len, const byte *payload);

    // returns payload length
    uint16 receive_packet(int fd, byte *cmd, byte *payload);

    void close(int fd);
}

#endif // COMIO_HEADER
