#!/usr/bin/env python

"""
Byte packing and unpacking utilities.

Copyright (C) 2017-2020 Cosmin Truta.

Use, modification and distribution are subject to the MIT License.
Please see the accompanying file LICENSE_MIT.txt
"""

from __future__ import absolute_import, division, print_function

import struct


def unpack_uint32be(buffer, offset=0):
    """Unpack an unsigned int from its 32-bit big-endian representation."""
    return struct.unpack(">I", buffer[offset:offset + 4])[0]


def unpack_uint32le(buffer, offset=0):
    """Unpack an unsigned int from its 32-bit little-endian representation."""
    return struct.unpack("<I", buffer[offset:offset + 4])[0]


def unpack_uint16be(buffer, offset=0):
    """Unpack an unsigned int from its 16-bit big-endian representation."""
    return struct.unpack(">H", buffer[offset:offset + 2])[0]


def unpack_uint16le(buffer, offset=0):
    """Unpack an unsigned int from its 16-bit little-endian representation."""
    return struct.unpack("<H", buffer[offset:offset + 2])[0]


def unpack_uint8(buffer, offset=0):
    """Unpack an unsigned int from its 8-bit representation."""
    return struct.unpack("B", buffer[offset:offset + 1])[0]


if __name__ == "__main__":
    # For testing only.
    assert unpack_uint32be(b"ABCDEF", 1) == 0x42434445
    assert unpack_uint32le(b"ABCDEF", 1) == 0x45444342
    assert unpack_uint16be(b"ABCDEF", 1) == 0x4243
    assert unpack_uint16le(b"ABCDEF", 1) == 0x4342
    assert unpack_uint8(b"ABCDEF", 1) == 0x42
