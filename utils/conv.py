import sys

print(
    int.from_bytes(sys.stdin.buffer.read(), byteorder='big')
)

