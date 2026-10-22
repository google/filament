#!/usr/bin/env python

"""
Show the EXIF information.

Copyright (C) 2017-2020 Cosmin Truta.

Use, modification and distribution are subject to the MIT License.
Please see the accompanying file LICENSE_MIT.txt
"""

from __future__ import absolute_import, division, print_function

import sys

from bytepack import (unpack_uint32be,
                      unpack_uint32le,
                      unpack_uint16be,
                      unpack_uint16le,
                      unpack_uint8)


# Generously allow the TIFF file to occupy up to a quarter-gigabyte.
# TODO: Reduce this limit to 64K and use file seeking for anything larger.
_READ_DATA_SIZE_MAX = 256 * 1024 * 1024

_TIFF_TAG_TYPES = {
    1: "byte",
    2: "ascii",
    3: "short",
    4: "long",
    5: "rational",
    6: "sbyte",
    7: "undefined",
    8: "sshort",
    9: "slong",
    10: "srational",
    11: "float",
    12: "double",
}

# See http://www.digitalpreservation.gov/formats/content/tiff_tags.shtml
_TIFF_TAGS = {
    0x00fe: "Subfile Type",
    0x0100: "Width",
    0x0101: "Height",
    0x0102: "Bits per Sample",
    0x0103: "Compression",
    0x0106: "Photometric",
    0x010d: "Document Name",
    0x010e: "Image Description",
    0x010f: "Make",
    0x0110: "Model",
    0x0111: "Strip Offsets",
    0x0112: "Orientation",
    0x0115: "Samples per Pixel",
    0x0116: "Rows per Strip",
    0x0117: "Strip Byte Counts",
    0x0118: "Min Sample Value",
    0x0119: "Max Sample Value",
    0x011a: "X Resolution",
    0x011b: "Y Resolution",
    0x011c: "Planar Configuration",
    0x011d: "Page Name",
    0x011e: "X Position",
    0x011f: "Y Position",
    0x0128: "Resolution Unit",
    0x0129: "Page Number",
    0x0131: "Software",
    0x0132: "Date Time",
    0x013b: "Artist",
    0x013c: "Host Computer",
    0x013d: "Predictor",
    0x013e: "White Point",
    0x013f: "Primary Chromaticities",
    0x0140: "Color Map",
    0x0141: "Half-Tone Hints",
    0x0142: "Tile Width",
    0x0143: "Tile Length",
    0x0144: "Tile Offsets",
    0x0145: "Tile Byte Counts",
    0x0211: "YCbCr Coefficients",
    0x0212: "YCbCr Subsampling",
    0x0213: "YCbCr Positioning",
    0x0214: "Reference Black White",
    0x022f: "Strip Row Counts",
    0x02bc: "XMP",
    0x8298: "Copyright",
    0x83bb: "IPTC",
    0x8769: "EXIF IFD",
    0x8773: "ICC Profile",
    0x8825: "GPS IFD",
    0xa005: "Interoperability IFD",
    0xc4a5: "Print IM",

    # EXIF IFD tags
    0x829a: "Exposure Time",
    0x829d: "F-Number",
    0x8822: "Exposure Program",
    0x8824: "Spectral Sensitivity",
    0x8827: "ISO Speed Ratings",
    0x8828: "OECF",
    0x9000: "EXIF Version",
    0x9003: "DateTime Original",
    0x9004: "DateTime Digitized",
    0x9101: "Components Configuration",
    0x9102: "Compressed Bits Per Pixel",
    0x9201: "Shutter Speed Value",
    0x9202: "Aperture Value",
    0x9203: "Brightness Value",
    0x9204: "Exposure Bias Value",
    0x9205: "Max Aperture Value",
    0x9206: "Subject Distance",
    0x9207: "Metering Mode",
    0x9208: "Light Source",
    0x9209: "Flash",
    0x920a: "Focal Length",
    0x9214: "Subject Area",
    0x927c: "Maker Note",
    0x9286: "User Comment",
    # ... TODO
    0xa000: "Flashpix Version",
    0xa001: "Color Space",
    0xa002: "Pixel X Dimension",
    0xa003: "Pixel Y Dimension",
    0xa004: "Related Sound File",
    # ... TODO

    # GPS IFD tags
    # ... TODO
}

_TIFF_EXIF_IFD = 0x8769
_GPS_IFD = 0x8825
_INTEROPERABILITY_IFD = 0xa005


class ExifInfo:
    """EXIF reader and information lister."""

    _endian = None
    _buffer = None
    _offset = 0
    _global_ifd_offset = 0
    _exif_ifd_offset = 0
    _gps_ifd_offset = 0
    _interoperability_ifd_offset = 0
    _hex = False

    def __init__(self, buffer, **kwargs):
        """Initialize the EXIF data reader."""
        self._hex = kwargs.get("hex", False)
        self._verbose = kwargs.get("verbose", False)
        if not isinstance(buffer, bytes):
            raise RuntimeError("invalid EXIF data type")
        if buffer.startswith(b"MM\x00\x2a"):
            self._endian = "MM"
        elif buffer.startswith(b"II\x2a\x00"):
            self._endian = "II"
        else:
            raise RuntimeError("invalid EXIF header")
        self._buffer = buffer
        self._offset = 4
        self._global_ifd_offset = self._ui32()

    def endian(self):
        """Return the endianness of the EXIF data."""
        return self._endian

    def _tags_for_ifd(self, ifd_offset):
        """Yield the tags found at the given TIFF IFD offset."""
        if ifd_offset < 8:
            raise RuntimeError("invalid TIFF IFD offset")
        self._offset = ifd_offset
        ifd_size = self._ui16()
        for _ in range(0, ifd_size):
            tag_id = self._ui16()
            tag_type = self._ui16()
            count = self._ui32()
            value_or_offset = self._ui32()
            if self._endian == "MM":
                # FIXME:
                # value_or_offset requires a fixup under big-endian encoding.
                if tag_type == 2:
                    # 2 --> "ascii"
                    value_or_offset >>= 24
                elif tag_type == 3:
                    # 3 --> "short"
                    value_or_offset >>= 16
                else:
                    # ... FIXME
                    pass
            if count == 0:
                raise RuntimeError("unsupported count=0 in tag 0x%x" % tag_id)
            if tag_id == _TIFF_EXIF_IFD:
                if tag_type != 4:
                    raise RuntimeError("incorrect tag type for EXIF IFD")
                self._exif_ifd_offset = value_or_offset
            elif tag_id == _GPS_IFD:
                if tag_type != 4:
                    raise RuntimeError("incorrect tag type for GPS IFD")
                self._gps_ifd_offset = value_or_offset
            elif tag_id == _INTEROPERABILITY_IFD:
                if tag_type != 4:
                    raise RuntimeError("incorrect tag type for Interop IFD")
                self._interoperability_ifd_offset = value_or_offset
            yield (tag_id, tag_type, count, value_or_offset)

    def tags(self):
        """Yield all TIFF/EXIF tags."""
        if self._verbose:
            print("TIFF IFD : 0x%08x" % self._global_ifd_offset)
        for tag in self._tags_for_ifd(self._global_ifd_offset):
            yield tag
        if self._exif_ifd_offset > 0:
            if self._verbose:
                print("EXIF IFD : 0x%08x" % self._exif_ifd_offset)
            for tag in self._tags_for_ifd(self._exif_ifd_offset):
                yield tag
        if self._gps_ifd_offset > 0:
            if self._verbose:
                print("GPS IFD : 0x%08x" % self._gps_ifd_offset)
            for tag in self._tags_for_ifd(self._gps_ifd_offset):
                yield tag
        if self._interoperability_ifd_offset > 0:
            if self._verbose:
                print("Interoperability IFD : 0x%08x" %
                      self._interoperability_ifd_offset)
            for tag in self._tags_for_ifd(self._interoperability_ifd_offset):
                yield tag

    def tagid2str(self, tag_id):
        """Return an informative string representation of a TIFF tag id."""
        idstr = _TIFF_TAGS.get(tag_id, "[Unknown]")
        if self._hex:
            idnum = "0x%04x" % tag_id
        else:
            idnum = "%d" % tag_id
        return "%s (%s)" % (idstr, idnum)

    @staticmethod
    def tagtype2str(tag_type):
        """Return an informative string representation of a TIFF tag type."""
        typestr = _TIFF_TAG_TYPES.get(tag_type, "[unknown]")
        return "%d:%s" % (tag_type, typestr)

    def tag2str(self, tag_id, tag_type, count, value_or_offset):
        """Return an informative string representation of a TIFF tag tuple."""
        return "%s (type=%s) (count=%d) : 0x%08x" \
               % (self.tagid2str(tag_id), self.tagtype2str(tag_type), count,
                  value_or_offset)

    def _ui32(self):
        """Decode a 32-bit unsigned int found at the current offset;
           advance the offset by 4.
        """
        if self._offset + 4 > len(self._buffer):
            raise RuntimeError("out-of-bounds uint32 access in EXIF")
        if self._endian == "MM":
            result = unpack_uint32be(self._buffer, self._offset)
        else:
            result = unpack_uint32le(self._buffer, self._offset)
        self._offset += 4
        return result

    def _ui16(self):
        """Decode a 16-bit unsigned int found at the current offset;
           advance the offset by 2.
        """
        if self._offset + 2 > len(self._buffer):
            raise RuntimeError("out-of-bounds uint16 access in EXIF")
        if self._endian == "MM":
            result = unpack_uint16be(self._buffer, self._offset)
        else:
            result = unpack_uint16le(self._buffer, self._offset)
        self._offset += 2
        return result

    def _ui8(self):
        """Decode an 8-bit unsigned int found at the current offset;
           advance the offset by 1.
        """
        if self._offset + 1 > len(self._buffer):
            raise RuntimeError("out-of-bounds uint8 access in EXIF")
        result = unpack_uint8(self._buffer, self._offset)
        self._offset += 1
        return result


def print_raw_exif_info(buffer, **kwargs):
    """Print the EXIF information found in a raw byte stream."""
    lister = ExifInfo(buffer, **kwargs)
    print("EXIF (endian=%s)" % lister.endian())
    for (tag_id, tag_type, count, value_or_offset) in lister.tags():
        print(lister.tag2str(tag_id=tag_id,
                             tag_type=tag_type,
                             count=count,
                             value_or_offset=value_or_offset))


if __name__ == "__main__":
    # For testing only.
    for arg in sys.argv[1:]:
        with open(arg, "rb") as test_stream:
            test_buffer = test_stream.read(_READ_DATA_SIZE_MAX)
            print_raw_exif_info(test_buffer, hex=True, verbose=True)
