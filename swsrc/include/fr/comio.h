#ifndef COMIO_HEADER
#define COMIO_HEADER

#include <share/types.h>

int comio_open(std::string dev, uint32 baud_rate);

void comio_send_packet(int fd, byte cmd, uint16 len, const byte *payload);

// returns payload length
uint16 comio_receive_packet(int fd, byte *cmd, byte *payload);

void comio_close(int fd);

#endif // COMIO_HEADER
