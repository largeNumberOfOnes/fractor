#include <share/require.h>
#include <fr/comio.h>
#include <termios.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int comio_open(std::string dev, uint32 baud_rate)
{
    int fd = open(dev.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    require(fd >= 0, ("Can't open com port[" + dev + "]").c_str());

    termios tty;
    require(!tcgetattr(fd, &tty), "Error in method tcgetattr");

    cfsetospeed(&tty, static_cast<speed_t>(baud_rate));
    cfsetispeed(&tty, static_cast<speed_t>(baud_rate));

    // 8 data-bits & 1 stop-bit
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);     // reading
    tty.c_cflag &= ~(PARENB | PARODD);   // no parity
    tty.c_cflag &= ~CSTOPB;              // 1 stop-bit
    tty.c_cflag &= ~CRTSCTS;             // no control
    tty.c_iflag = 0;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 1;                // read one byte at least
    tty.c_cc[VTIME] = 100;              // 10s timeout

    require(!tcsetattr(fd, TCSANOW, &tty), "Error in method tcsetattr");
    return fd;
}

void comio_send(int fd, const byte *buffer, uint32 len)
{
    write(fd, buffer, len);
}

void comio_receive(int fd, byte *buffer, uint32 len)
{
    ssize_t size = read(fd, buffer, len);
    require(size == len, "COM timeout");
}

void comio_close(int fd)
{
    close(fd);
}

#define SYNC1 static_cast<byte>(0x55)
#define SYNC2 static_cast<byte>(0xAA)

void comio_send_packet(int fd, byte cmd, uint16 len, const byte *payload)
{
    uint32 buffer_len = 2 + sizeof(cmd) + sizeof(len) + len;
    byte *buffer = new byte[buffer_len];
    require(buffer, "Can't allocate memory for comio buffer");

    buffer[0] = SYNC1;
    buffer[1] = SYNC2;
    buffer[2] = cmd;
    buffer[3] = static_cast<byte>((len >> 8) & 0xFF);
    buffer[4] = static_cast<byte>((len >> 0) & 0xFF);
    std::memcpy(buffer + 5, payload, len);

    comio_send(fd, buffer, buffer_len);
}

uint16 comio_receive_packet(int fd, byte *cmd, byte *payload)
{
    byte sync = static_cast<byte>(0);
    while(true)
    {
        comio_receive(fd, &sync, 1);
        if(sync == SYNC1)
        {
            comio_receive(fd, &sync, 1);
            if(sync == SYNC2)
                break;
        }
    }

    comio_receive(fd, cmd, 1);
    
    byte len_buffer[2];
    comio_receive(fd, len_buffer, 2);
    uint16 len = static_cast<uint16>(len_buffer[1]);
    len += (static_cast<uint16>(len_buffer[0]) << 8);

    comio_receive(fd, payload, len);

    return len;
}
