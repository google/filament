#!/usr/bin/env python
"""Sample Input Reader for map job."""
import random
import string
import time

from mapreduce import context
from mapreduce import errors
from mapreduce import operation
from mapreduce.api import map_job

# pylint: disable=invalid-name

# Counter name for number of bytes read.
COUNTER_IO_READ_BYTES = "io-read-bytes"

# Counter name for milliseconds spent reading data.
COUNTER_IO_READ_MSEC = "io-read-msec"


class SampleInputReader(map_job.InputReader):
  """A sample InputReader that generates random strings as output.

  Primary usage is to as an example InputReader that can be use for test
  purposes.
  """

  # Total number of entries this reader should generate.
  COUNT = "count"
  # Length of the generated strings.
  STRING_LENGTH = "string_length"
  # The default string length if one is not specified.
  _DEFAULT_STRING_LENGTH = 10

  def __init__(self, count, string_length):
    """Initialize input reader.

    Args:
      count: number of entries this shard should generate.
      string_length: the length of generated random strings.
    """
    self._count = count
    self._string_length = string_length

  def __iter__(self):
    ctx = context.get()

    while self._count:
      self._count -= 1
      start_time = time.time()
      content = "".join(random.choice(string.ascii_lowercase)
                        for _ in range(self._string_length))
      if ctx:
        operation.counters.Increment(
            COUNTER_IO_READ_MSEC, int((time.time() - start_time) * 1000))(ctx)
        operation.counters.Increment(COUNTER_IO_READ_BYTES, len(content))(ctx)
      yield content

  @classmethod
  def from_json(cls, state):
    """Inherit docs."""
    return cls(state[cls.COUNT], state[cls.STRING_LENGTH])

  def to_json(self):
    """Inherit docs."""
    return {self.COUNT: self._count, self.STRING_LENGTH: self._string_length}

  @classmethod
  def split_input(cls, job_config):
    """Inherit docs."""
    params = job_config.input_reader_params
    count = params[cls.COUNT]
    string_length = params.get(cls.STRING_LENGTH, cls._DEFAULT_STRING_LENGTH)

    shard_count = job_config.shard_count
    count_per_shard = count // shard_count

    mr_input_readers = [
        cls(count_per_shard, string_length) for _ in range(shard_count)]

    left = count - count_per_shard*shard_count
    if left > 0:
      mr_input_readers.append(cls(left, string_length))

    return mr_input_readers

  @classmethod
  def validate(cls, job_config):
    """Inherit docs."""
    super(SampleInputReader, cls).validate(job_config)

    params = job_config.input_reader_params
    # Validate count.
    if cls.COUNT not in params:
      raise errors.BadReaderParamsError("Must specify %s" % cls.COUNT)
    if not isinstance(params[cls.COUNT], int):
      raise errors.BadReaderParamsError("%s should be an int but is %s" %
                                        (cls.COUNT, type(params[cls.COUNT])))
    if params[cls.COUNT] <= 0:
      raise errors.BadReaderParamsError("%s should be a positive int")
    # Validate string length.
    if cls.STRING_LENGTH in params and not (
        isinstance(params[cls.STRING_LENGTH], int) and
        params[cls.STRING_LENGTH] > 0):
      raise errors.BadReaderParamsError("%s should be a positive int "
                                        "but is %s" %
                                        (cls.STRING_LENGTH,
                                         params[cls.STRING_LENGTH]))


