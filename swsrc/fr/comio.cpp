#include <share/require.h>
#include <unordered_map>
#include <fr/comio.h>
#include <termios.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

static const std::unordered_map<uint32, speed_t> baud_rate_map
{
    {300,       B300    },
    {600,       B600    },
    {1200,      B1200   },
    {2400,      B2400   },
    {4800,      B4800   },
    {9600,      B9600   },
    {19200,     B19200  },
    {38400,     B38400  },
    {57600,     B57600  },
    {115200,    B115200 },
    {230400,    B230400 },
    {460800,    B460800 },
    {500000,    B500000 },
    {576000,    B576000 },
    {921600,    B921600 },
    {1000000,   B1000000},
    {1152000,   B1152000},
    {1500000,   B1500000},
    {2000000,   B2000000},
    {2500000,   B2500000},
    {3000000,   B3000000},
    {3500000,   B3500000},
    {4000000,   B4000000}
};

int comio::open(const std::string &dev, uint32 baud_rate)
{
    int fd = ::open(dev.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    require(fd >= 0, ("Can't open com port[" + dev + "]").c_str());

    termios tty;
    require(!tcgetattr(fd, &tty), "Error in method tcgetattr");

    auto it = baud_rate_map.find(baud_rate);
    require(it != baud_rate_map.end(), "Unsupported baud rate");
    speed_t speed = it->second;
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

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

static void comio_send(int fd, const byte *buffer, uint32 len)
{
    write(fd, buffer, len);
}

static void comio_receive(int fd, byte *buffer, uint32 len)
{
    ssize_t size = read(fd, buffer, len);
    require(size == len, "COM timeout");
}

void comio::close(int fd)
{
    close(fd);
}

static constexpr byte SYNC1 = byte{0x55};
static constexpr byte SYNC2 = byte{0xAA};

void comio::send_packet(int fd, byte cmd, uint16 len, const byte *payload)
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

uint16 comio::receive_packet(int fd, byte *cmd, byte *payload)
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
