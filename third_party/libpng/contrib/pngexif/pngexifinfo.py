#!/usr/bin/env python

"""
Show the PNG EXIF information.

Copyright (C) 2017-2020 Cosmin Truta.

Use, modification and distribution are subject to the MIT License.
Please see the accompanying file LICENSE_MIT.txt
"""

from __future__ import absolute_import, division, print_function

import argparse
import io
import re
import sys
import zlib

from bytepack import unpack_uint32be, unpack_uint8
from exifinfo import print_raw_exif_info

_PNG_SIGNATURE = b"\x89PNG\x0d\x0a\x1a\x0a"
_PNG_CHUNK_SIZE_MAX = 0x7fffffff
_READ_DATA_SIZE_MAX = 0x3ffff


def print_error(msg):
    """Print an error message to stderr."""
    sys.stderr.write("%s: error: %s\n" % (sys.argv[0], msg))


def print_debug(msg):
    """Print a debug message to stderr."""
    sys.stderr.write("%s: debug: %s\n" % (sys.argv[0], msg))


def _check_png(condition, chunk_sig=None):
    """Check a PNG-specific assertion."""
    if condition:
        return
    if chunk_sig is None:
        raise RuntimeError("bad PNG data")
    raise RuntimeError("bad PNG data in '%s'" % chunk_sig)


def _check_png_crc(data, checksum, chunk_sig):
    """Check a CRC32 value inside a PNG stream."""
    if unpack_uint32be(data) == (checksum & 0xffffffff):
        return
    raise RuntimeError("bad PNG checksum in '%s'" % chunk_sig)


def _extract_png_exif(data, **kwargs):
    """Extract the EXIF header and data from a PNG chunk."""
    debug = kwargs.get("debug", False)
    if unpack_uint8(data, 0) == 0:
        if debug:
            print_debug("found compressed EXIF, compression method 0")
        if (unpack_uint8(data, 1) & 0x0f) == 0x08:
            data = zlib.decompress(data[1:])
        elif unpack_uint8(data, 1) == 0 \
                and (unpack_uint8(data, 5) & 0x0f) == 0x08:
            if debug:
                print_debug("found uncompressed-length EXIF field")
            data_len = unpack_uint32be(data, 1)
            data = zlib.decompress(data[5:])
            if data_len != len(data):
                raise RuntimeError(
                    "incorrect uncompressed-length field in PNG EXIF")
        else:
            raise RuntimeError("invalid compression method in PNG EXIF")
    if data.startswith(b"MM\x00\x2a") or data.startswith(b"II\x2a\x00"):
        return data
    raise RuntimeError("invalid TIFF/EXIF header in PNG EXIF")


def print_png_exif_info(instream, **kwargs):
    """Print the EXIF information found in the given PNG datastream."""
    debug = kwargs.get("debug", False)
    has_exif = False
    while True:
        chunk_hdr = instream.read(8)
        _check_png(len(chunk_hdr) == 8)
        chunk_len = unpack_uint32be(chunk_hdr, offset=0)
        chunk_sig = chunk_hdr[4:8].decode("latin_1", errors="ignore")
        _check_png(re.search(r"^[A-Za-z]{4}$", chunk_sig), chunk_sig=chunk_sig)
        _check_png(chunk_len < _PNG_CHUNK_SIZE_MAX, chunk_sig=chunk_sig)
        if debug:
            print_debug("processing chunk: %s" % chunk_sig)
        if chunk_len <= _READ_DATA_SIZE_MAX:
            # The chunk size does not exceed an arbitrary, reasonable limit.
            chunk_data = instream.read(chunk_len)
            chunk_crc = instream.read(4)
            _check_png(len(chunk_data) == chunk_len and len(chunk_crc) == 4,
                       chunk_sig=chunk_sig)
            checksum = zlib.crc32(chunk_hdr[4:8])
            checksum = zlib.crc32(chunk_data, checksum)
            _check_png_crc(chunk_crc, checksum, chunk_sig=chunk_sig)
        else:
            # The chunk is too big. Skip it.
            instream.seek(chunk_len + 4, io.SEEK_CUR)
            continue
        if chunk_sig == "IEND":
            _check_png(chunk_len == 0, chunk_sig=chunk_sig)
            break
        if chunk_sig.lower() in ["exif", "zxif"] and chunk_len > 8:
            has_exif = True
            exif_data = _extract_png_exif(chunk_data, **kwargs)
            print_raw_exif_info(exif_data, **kwargs)
    if not has_exif:
        raise RuntimeError("no EXIF data in PNG stream")


def print_exif_info(file, **kwargs):
    """Print the EXIF information found in the given file."""
    with open(file, "rb") as stream:
        header = stream.read(4)
        if header == _PNG_SIGNATURE[0:4]:
            if stream.read(4) != _PNG_SIGNATURE[4:8]:
                raise RuntimeError("corrupted PNG file")
            print_png_exif_info(instream=stream, **kwargs)
        elif header == b"II\x2a\x00" or header == b"MM\x00\x2a":
            data = header + stream.read(_READ_DATA_SIZE_MAX)
            print_raw_exif_info(data, **kwargs)
        else:
            raise RuntimeError("not a PNG file")


def main():
    """The main function."""
    parser = argparse.ArgumentParser(
        prog="pngexifinfo",
        usage="%(prog)s [options] [--] files...",
        description="Show the PNG EXIF information.")
    parser.add_argument("files",
                        metavar="file",
                        nargs="*",
                        help="a PNG file or a raw EXIF blob")
    parser.add_argument("-x",
                        "--hex",
                        dest="hex",
                        action="store_true",
                        help="show EXIF tags in base 16")
    parser.add_argument("-v",
                        "--verbose",
                        dest="verbose",
                        action="store_true",
                        help="run in verbose mode")
    parser.add_argument("--debug",
                        dest="debug",
                        action="store_true",
                        help="run in debug mode")
    args = parser.parse_args()
    if not args.files:
        parser.error("missing file operand")
    result = 0
    for file in args.files:
        try:
            print_exif_info(file,
                            hex=args.hex,
                            debug=args.debug,
                            verbose=args.verbose)
        except (IOError, OSError) as err:
            print_error(str(err))
            result = 66  # os.EX_NOINPUT
        except RuntimeError as err:
            print_error("%s: %s" % (file, str(err)))
            result = 69  # os.EX_UNAVAILABLE
    parser.exit(result)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        sys.stderr.write("INTERRUPTED\n")
        sys.exit(130)  # SIGINT
