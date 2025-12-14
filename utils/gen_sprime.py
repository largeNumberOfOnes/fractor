from typing import Tuple

import subprocess as sb
import sys

def get_num(arr: bytes, offset: int) -> Tuple[int, int]: # (val, offset)
    len_size = 4
    byte_order = 'big'
    size = int.from_bytes(
        result.stdout[offset:offset + len_size],
        byteorder=byte_order
    )
    val = int.from_bytes(
        result.stdout[offset + len_size:offset + len_size + size],
        byteorder=byte_order
    )
    return val, len_size + size

if __name__ == '__main__':
    count = 10
    result = sb.run(
        f'./build/gen {count} -v -c 1'.split(),
        capture_output=True
    )

    sprime, sprime_offset = get_num(result, 0)
    prime1, prime1_offset = get_num(result, sprime_offset)
    prime2, prime2_offset = get_num(result, sprime_offset + prime1_offset)

    print(f'{prime1} * {prime2} = {sprime}')
