# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Utility module for dealing with Git timestamps."""

import datetime


def timestamp_offset_to_datetime(timestamp, offset):
    """Converts a timestamp + offset into a datetime.datetime.

    Useful for dealing with the output of porcelain commands, which provide
    times as timestamp and offset strings.

    Args:
        timestamp: An int UTC timestamp, or a string containing decimal digits.
        offset: A str timezone offset. e.g., '-0800'.

    Returns:
        A tz-aware datetime.datetime for this timestamp.
    """
    timestamp = int(timestamp)
    tz = FixedOffsetTZ.from_offset_string(offset)
    return datetime.datetime.fromtimestamp(timestamp, tz)


def datetime_string(dt):
    """Converts a tz-aware datetime.datetime into a string in git format."""
    return dt.strftime('%Y-%m-%d %H:%M:%S %z')


# Adapted from: https://docs.python.org/2/library/datetime.html#tzinfo-objects
class FixedOffsetTZ(datetime.tzinfo):
    def __init__(self, offset, name):
        datetime.tzinfo.__init__(self)
        self.__offset = offset
        self.__name = name

    def __repr__(self):  # pragma: no cover
        return '{}({!r}, {!r})'.format(
            type(self).__name__, self.__offset, self.__name)

    @classmethod
    def from_offset_string(cls, offset):
        try:
            hours = int(offset[:-2])
            minutes = int(offset[-2:])
        except ValueError:
            return cls(datetime.timedelta(0), 'UTC')

        delta = datetime.timedelta(hours=hours, minutes=minutes)
        return cls(delta, offset)

    def utcoffset(self, dt):
        return self.__offset

    def tzname(self, dt):
        return self.__name

    def dst(self, dt):
        return datetime.timedelta(0)
