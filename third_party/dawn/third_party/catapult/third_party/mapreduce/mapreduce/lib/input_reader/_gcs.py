#!/usr/bin/env python
"""GCS related input readers."""

__all__ = [
    "GCSInputReader",
    "GCSRecordInputReader",
    "PathFilter",
    ]

# pylint: disable=g-bad-name
# pylint: disable=protected-access

import logging
import pickle
import time

from mapreduce import errors
from mapreduce import records
from mapreduce.api import map_job


# pylint: disable=g-import-not-at-top
# TODO(user): Cleanup imports if/when cloudstorage becomes part of runtime.
try:
  # Check if the full cloudstorage package exists. The stub part is in runtime.
  import cloudstorage
  if hasattr(cloudstorage, "_STUB"):
    cloudstorage = None
except ImportError:
  pass  # CloudStorage library not available


class PathFilter(object):
  """Path filter for GCSInputReader."""

  def accept(self, slice_ctx, path):
    """Accepts a path.

    Only accepted path will be opened for read.

    Args:
      slice_ctx: the instance of SliceContext for current slice.
      path: a GCS filename of form '/bucket/filename'

    Returns:
      True if this file should be read. False otherwise.
    """
    raise NotImplementedError()


class GCSInputReader(map_job.InputReader):
  """Input reader from Google Cloud Storage using the cloudstorage library.

  Required configuration in the mapper_spec.input_reader dictionary.
    BUCKET_NAME_PARAM: name of the bucket to use. No "/" prefix or suffix.
    OBJECT_NAMES_PARAM: a list of object names or prefixes. All objects must be
      in the BUCKET_NAME_PARAM bucket. If the name ends with a * it will be
      treated as prefix and all objects with matching names will be read.
      Entries should not start with a slash unless that is part of the object's
      name. An example list could be:
      ["my-1st-input-file", "directory/my-2nd-file", "some/other/dir/input-*"]
      To retrieve all files "*" will match every object in the bucket. If a file
      is listed twice or is covered by multiple prefixes it will be read twice,
      there is no de-duplication.

  Optional configuration in the mapper_sec.input_reader dictionary.
    BUFFER_SIZE_PARAM: the size of the read buffer for each file handle.
    PATH_FILTER_PARAM: an instance of PathFilter. PathFilter is a predicate
      on which filenames to read.
    DELIMITER_PARAM: str. The delimiter that signifies directory.
      If you have too many files to shard on the granularity of individual
      files, you can specify this to enable shallow splitting. In this mode,
      the reader only goes one level deep during "*" expansion and stops when
      the delimiter is encountered.
  """

  # Counters.
  COUNTER_FILE_READ = "file-read"
  COUNTER_FILE_MISSING = "file-missing"

  # Supported parameters
  BUCKET_NAME_PARAM = "bucket_name"
  OBJECT_NAMES_PARAM = "objects"
  BUFFER_SIZE_PARAM = "buffer_size"
  DELIMITER_PARAM = "delimiter"
  PATH_FILTER_PARAM = "path_filter"

  # Internal parameters
  _ACCOUNT_ID_PARAM = "account_id"

  # Other internal configuration constants
  _JSON_PICKLE = "pickle"
  _STRING_MAX_FILES_LISTED = 10  # Max files shown in the str representation

  # Input reader can also take in start and end filenames and do
  # listbucket. This saves space but has two cons.
  # 1. Files to read are less well defined: files can be added or removed over
  #    the lifetime of the MR job.
  # 2. A shard has to process files from a contiguous namespace.
  #    May introduce staggering shard.
  def __init__(self, filenames, index=0, buffer_size=None, _account_id=None,
               delimiter=None, path_filter=None):
    """Initialize a GoogleCloudStorageInputReader instance.

    Args:
      filenames: A list of Google Cloud Storage filenames of the form
        '/bucket/objectname'.
      index: Index of the next filename to read.
      buffer_size: The size of the read buffer, None to use default.
      _account_id: Internal use only. See cloudstorage documentation.
      delimiter: Delimiter used as path separator. See class doc.
      path_filter: An instance of PathFilter.
    """
    super(GCSInputReader, self).__init__()
    self._filenames = filenames
    self._index = index
    self._buffer_size = buffer_size
    self._account_id = _account_id
    self._delimiter = delimiter
    self._bucket = None
    self._bucket_iter = None
    self._path_filter = path_filter
    self._slice_ctx = None

  def _next_file(self):
    """Find next filename.

    self._filenames may need to be expanded via listbucket.

    Returns:
      None if no more file is left. Filename otherwise.
    """
    while True:
      if self._bucket_iter:
        try:
          return self._bucket_iter.next().filename
        except StopIteration:
          self._bucket_iter = None
          self._bucket = None
      if self._index >= len(self._filenames):
        return
      filename = self._filenames[self._index]
      self._index += 1
      if self._delimiter is None or not filename.endswith(self._delimiter):
        return filename
      self._bucket = cloudstorage.listbucket(filename,
                                             delimiter=self._delimiter)
      self._bucket_iter = iter(self._bucket)

  @classmethod
  def validate(cls, job_config):
    """Validate mapper specification.

    Args:
      job_config: map_job.JobConfig.

    Raises:
      BadReaderParamsError: if the specification is invalid for any reason such
        as missing the bucket name or providing an invalid bucket name.
    """
    reader_params = job_config.input_reader_params

    # Bucket Name is required
    if cls.BUCKET_NAME_PARAM not in reader_params:
      raise errors.BadReaderParamsError(
          "%s is required for Google Cloud Storage" %
          cls.BUCKET_NAME_PARAM)
    try:
      cloudstorage.validate_bucket_name(
          reader_params[cls.BUCKET_NAME_PARAM])
    except ValueError, error:
      raise errors.BadReaderParamsError("Bad bucket name, %s" % (error))

    # Object Name(s) are required
    if cls.OBJECT_NAMES_PARAM not in reader_params:
      raise errors.BadReaderParamsError(
          "%s is required for Google Cloud Storage" %
          cls.OBJECT_NAMES_PARAM)
    filenames = reader_params[cls.OBJECT_NAMES_PARAM]
    if not isinstance(filenames, list):
      raise errors.BadReaderParamsError(
          "Object name list is not a list but a %s" %
          filenames.__class__.__name__)
    for filename in filenames:
      if not isinstance(filename, basestring):
        raise errors.BadReaderParamsError(
            "Object name is not a string but a %s" %
            filename.__class__.__name__)

    # Delimiter.
    if cls.DELIMITER_PARAM in reader_params:
      delimiter = reader_params[cls.DELIMITER_PARAM]
      if not isinstance(delimiter, basestring):
        raise errors.BadReaderParamsError(
            "%s is not a string but a %s" %
            (cls.DELIMITER_PARAM, type(delimiter)))

    # Buffer size.
    if cls.BUFFER_SIZE_PARAM in reader_params:
      buffer_size = reader_params[cls.BUFFER_SIZE_PARAM]
      if not isinstance(buffer_size, int):
        raise errors.BadReaderParamsError(
            "%s is not an int but a %s" %
            (cls.BUFFER_SIZE_PARAM, type(buffer_size)))

    # Path filter.
    if cls.PATH_FILTER_PARAM in reader_params:
      path_filter = reader_params[cls.PATH_FILTER_PARAM]
      if not isinstance(path_filter, PathFilter):
        raise errors.BadReaderParamsError(
            "%s is not an instance of PathFilter but %s." %
            (cls.PATH_FILTER_PARAM, type(path_filter)))

  @classmethod
  def split_input(cls, job_config):
    """Returns a list of input readers.

    An equal number of input files are assigned to each shard (+/- 1). If there
    are fewer files than shards, fewer than the requested number of shards will
    be used. Input files are currently never split (although for some formats
    could be and may be split in a future implementation).

    Args:
      job_config: map_job.JobConfig

    Returns:
      A list of InputReaders. None when no input data can be found.
    """
    reader_params = job_config.input_reader_params
    bucket = reader_params[cls.BUCKET_NAME_PARAM]
    filenames = reader_params[cls.OBJECT_NAMES_PARAM]
    delimiter = reader_params.get(cls.DELIMITER_PARAM)
    account_id = reader_params.get(cls._ACCOUNT_ID_PARAM)
    buffer_size = reader_params.get(cls.BUFFER_SIZE_PARAM)
    path_filter = reader_params.get(cls.PATH_FILTER_PARAM)

    # Gather the complete list of files (expanding wildcards)
    all_filenames = []
    for filename in filenames:
      if filename.endswith("*"):
        all_filenames.extend(
            [file_stat.filename for file_stat in cloudstorage.listbucket(
                "/" + bucket + "/" + filename[:-1], delimiter=delimiter,
                _account_id=account_id)])
      else:
        all_filenames.append("/%s/%s" % (bucket, filename))

    # Split into shards
    readers = []
    for shard in range(0, job_config.shard_count):
      shard_filenames = all_filenames[shard::job_config.shard_count]
      if shard_filenames:
        readers.append(cls(
            shard_filenames, buffer_size=buffer_size, _account_id=account_id,
            delimiter=delimiter, path_filter=path_filter))
    return readers

  @classmethod
  def from_json(cls, state):
    obj = pickle.loads(state[cls._JSON_PICKLE])
    if obj._bucket:
      obj._bucket_iter = iter(obj._bucket)
    return obj

  def to_json(self):
    before_iter = self._bucket_iter
    before_slice_ctx = self._slice_ctx
    self._bucket_iter = None
    self._slice_ctx = None
    try:
      return {self._JSON_PICKLE: pickle.dumps(self)}
    finally:
      self._bucket_itr = before_iter
      self._slice_ctx = before_slice_ctx

  def next(self):
    """Returns a handler to the next file.

    Non existent files will be logged and skipped. The file might have been
    removed after input splitting.

    Returns:
      The next input from this input reader in the form of a cloudstorage
      ReadBuffer that supports a File-like interface (read, readline, seek,
      tell, and close). An error may be raised if the file can not be opened.

    Raises:
      StopIteration: The list of files has been exhausted.
    """
    options = {}
    if self._buffer_size:
      options["read_buffer_size"] = self._buffer_size
    if self._account_id:
      options["_account_id"] = self._account_id
    while True:
      filename = self._next_file()
      if filename is None:
        raise StopIteration()
      if (self._path_filter and
          not self._path_filter.accept(self._slice_ctx, filename)):
        continue
      try:
        start_time = time.time()
        handle = cloudstorage.open(filename, **options)
        self._slice_ctx.incr(self.COUNTER_IO_READ_MSEC,
                             int(time.time() - start_time) * 1000)
        self._slice_ctx.incr(self.COUNTER_FILE_READ)
        return handle
      except cloudstorage.NotFoundError:
        logging.warning("File %s may have been removed. Skipping file.",
                        filename)
        self._slice_ctx.incr(self.COUNTER_FILE_MISSING)

  def __str__(self):
    # Only show a limited number of files individually for readability
    num_files = len(self._filenames)
    if num_files > self._STRING_MAX_FILES_LISTED:
      names = "%s...%s + %d not shown" % (
          ",".join(self._filenames[0:self._STRING_MAX_FILES_LISTED-1]),
          self._filenames[-1],
          num_files - self._STRING_MAX_FILES_LISTED)
    else:
      names = ",".join(self._filenames)

    if self._index > num_files:
      status = "EOF"
    else:
      status = "Next %s (%d of %d)" % (
          self._filenames[self._index],
          self._index + 1,  # +1 for human 1-indexing
          num_files)
    return "CloudStorage [%s, %s]" % (status, names)

  @classmethod
  def params_to_json(cls, params):
    """Inherit docs."""
    params_cp = dict(params)
    if cls.PATH_FILTER_PARAM in params_cp:
      path_filter = params_cp[cls.PATH_FILTER_PARAM]
      params_cp[cls.PATH_FILTER_PARAM] = pickle.dumps(path_filter)
    return params_cp

  @classmethod
  def params_from_json(cls, json_params):
    if cls.PATH_FILTER_PARAM in json_params:
      path_filter = pickle.loads(json_params[cls.PATH_FILTER_PARAM])
      json_params[cls.PATH_FILTER_PARAM] = path_filter
    return json_params


class GCSRecordInputReader(GCSInputReader):
  """Read data from a Google Cloud Storage file using LevelDB format.

  See the GCSInputReader for additional configuration options.
  """

  def __getstate__(self):
    result = self.__dict__.copy()
    # record reader may not exist if reader has not been used
    if "_record_reader" in result:
      # RecordsReader has no buffering, it can safely be reconstructed after
      # deserialization
      result.pop("_record_reader")
    return result

  def next(self):
    """Returns the next input from this input reader, a record.

    Returns:
      The next input from this input reader in the form of a record read from
      an LevelDB file.

    Raises:
      StopIteration: The ordered set records has been exhausted.
    """
    while True:
      if not hasattr(self, "_cur_handle") or self._cur_handle is None:
        # If there are no more files, StopIteration is raised here
        self._cur_handle = super(GCSRecordInputReader, self).next()
      if not hasattr(self, "_record_reader") or self._record_reader is None:
        self._record_reader = records.RecordsReader(self._cur_handle)

      try:
        start_time = time.time()
        content = self._record_reader.read()
        self._slice_ctx.incr(self.COUNTER_IO_READ_BYTE, len(content))
        self._slice_ctx.incr(self.COUNTER_IO_READ_MSEC,
                             int(time.time() - start_time) * 1000)
        return content
      except EOFError:
        self._cur_handle = None
        self._record_reader = None

