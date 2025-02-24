#!/usr/bin/env python
"""A simple reader for file segs produced by GCS output writer."""

from mapreduce import output_writers

# pylint: disable=protected-access
# pylint: disable=invalid-name

# pylint: disable=g-import-not-at-top
# TODO(user): Cleanup imports if/when cloudstorage becomes part of runtime.
try:
  # Check if the full cloudstorage package exists. The stub part is in runtime.
  import cloudstorage
  if hasattr(cloudstorage, "_STUB"):
    cloudstorage = None
except ImportError:
  pass  # CloudStorage library not available


class _GCSFileSegReader(object):
  """A simple reader for file segs produced by GCS output writer.

  Internal use only.

  This reader conforms to Python stream interface.
  """

  def __init__(self, seg_prefix, last_seg_index):
    """Init.

    Instances are pickle safe.

    Args:
      seg_prefix: filename prefix for all segs. It is expected
        seg_prefix + index = seg filename.
      last_seg_index: the last index of all segs. int.
    """
    self._EOF = False
    self._offset = 0

    # fields related to seg.
    self._seg_prefix = seg_prefix
    self._last_seg_index = last_seg_index
    self._seg_index = -1
    self._seg_valid_length = None
    self._seg = None
    self._next_seg()

  def read(self, n):
    """Read data from file segs.

    Args:
      n: max bytes to read. Must be positive.

    Returns:
      some bytes. May be smaller than n bytes. "" when no more data is left.
    """
    if self._EOF:
      return ""

    while self._seg_index <= self._last_seg_index:
      result = self._read_from_seg(n)
      if result != "":
        return result
      else:
        self._next_seg()

    self._EOF = True
    return ""

  def close(self):
    if self._seg:
      self._seg.close()

  def tell(self):
    """Returns the next offset to read."""
    return self._offset

  def _next_seg(self):
    """Get next seg."""
    if self._seg:
      self._seg.close()
    self._seg_index += 1
    if self._seg_index > self._last_seg_index:
      self._seg = None
      return

    filename = self._seg_prefix + str(self._seg_index)
    stat = cloudstorage.stat(filename)
    writer = output_writers._GoogleCloudStorageOutputWriter
    if writer._VALID_LENGTH not in stat.metadata:
      raise ValueError(
          "Expect %s in metadata for file %s." %
          (writer._VALID_LENGTH, filename))
    self._seg_valid_length = int(stat.metadata[writer._VALID_LENGTH])
    if self._seg_valid_length > stat.st_size:
      raise ValueError(
          "Valid length %s is too big for file %s of length %s" %
          (self._seg_valid_length, filename, stat.st_size))
    self._seg = cloudstorage.open(filename)

  def _read_from_seg(self, n):
    """Read from current seg.

    Args:
      n: max number of bytes to read.

    Returns:
      valid bytes from the current seg. "" if no more is left.
    """
    result = self._seg.read(size=n)
    if result == "":
      return result
    offset = self._seg.tell()
    if offset > self._seg_valid_length:
      extra = offset - self._seg_valid_length
      result = result[:-1*extra]
    self._offset += len(result)
    return result
