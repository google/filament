#!/usr/bin/env python
# Copyright 2010 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Output writers for MapReduce."""

from __future__ import with_statement



__all__ = [
    "GoogleCloudStorageConsistentOutputWriter",
    "GoogleCloudStorageConsistentRecordOutputWriter",
    "GoogleCloudStorageKeyValueOutputWriter",
    "GoogleCloudStorageOutputWriter",
    "GoogleCloudStorageRecordOutputWriter",
    "COUNTER_IO_WRITE_BYTES",
    "COUNTER_IO_WRITE_MSEC",
    "OutputWriter",
    "GCSRecordsPool"
    ]

# pylint: disable=g-bad-name
# pylint: disable=protected-access

import cStringIO
import gc
import logging
import pickle
import random
import string
import time

from mapreduce import context
from mapreduce import errors
from mapreduce import json_util
from mapreduce import kv_pb
from mapreduce import model
from mapreduce import operation
from mapreduce import records
from mapreduce import shard_life_cycle

# pylint: disable=g-import-not-at-top
# TODO(user): Cleanup imports if/when cloudstorage becomes part of runtime.
try:
  # Check if the full cloudstorage package exists. The stub part is in runtime.
  cloudstorage = None
  import cloudstorage
  if hasattr(cloudstorage, "_STUB"):
    cloudstorage = None
  # "if" is needed because apphosting/ext/datastore_admin:main_test fails.
  if cloudstorage:
    from cloudstorage import cloudstorage_api
    from cloudstorage import errors as cloud_errors
except ImportError:
  pass  # CloudStorage library not available

# Attempt to load cloudstorage from the bundle (availble in some tests)
if cloudstorage is None:
  try:
    import cloudstorage
    from cloudstorage import cloudstorage_api
  except ImportError:
    pass  # CloudStorage library really not available


# Counter name for number of bytes written.
COUNTER_IO_WRITE_BYTES = "io-write-bytes"

# Counter name for time spent writing data in msec
COUNTER_IO_WRITE_MSEC = "io-write-msec"


class OutputWriter(json_util.JsonMixin):
  """Abstract base class for output writers.

  Output writers process all mapper handler output, which is not
  the operation.

  OutputWriter's lifecycle is the following:
    0) validate called to validate mapper specification.
    1) init_job is called to initialize any job-level state.
    2) create() is called, which should create a new instance of output
       writer for a given shard
    3) from_json()/to_json() are used to persist writer's state across
       multiple slices.
    4) write() method is called to write data.
    5) finalize() is called when shard processing is done.
    6) finalize_job() is called when job is completed.
    7) get_filenames() is called to get output file names.
  """

  @classmethod
  def validate(cls, mapper_spec):
    """Validates mapper specification.

    Output writer parameters are expected to be passed as "output_writer"
    subdictionary of mapper_spec.params. To be compatible with previous
    API output writer is advised to check mapper_spec.params and issue
    a warning if "output_writer" subdicationary is not present.
    _get_params helper method can be used to simplify implementation.

    Args:
      mapper_spec: an instance of model.MapperSpec to validate.
    """
    raise NotImplementedError("validate() not implemented in %s" % cls)

  @classmethod
  def init_job(cls, mapreduce_state):
    """Initialize job-level writer state.

    This method is only to support the deprecated feature which is shared
    output files by many shards. New output writers should not do anything
    in this method.

    Args:
      mapreduce_state: an instance of model.MapreduceState describing current
      job. MapreduceState.writer_state can be modified during initialization
      to save the information about the files shared by many shards.
    """
    pass

  @classmethod
  def finalize_job(cls, mapreduce_state):
    """Finalize job-level writer state.

    This method is only to support the deprecated feature which is shared
    output files by many shards. New output writers should not do anything
    in this method.

    This method should only be called when mapreduce_state.result_status shows
    success. After finalizing the outputs, it should save the info for shard
    shared files into mapreduce_state.writer_state so that other operations
    can find the outputs.

    Args:
      mapreduce_state: an instance of model.MapreduceState describing current
      job. MapreduceState.writer_state can be modified during finalization.
    """
    pass

  @classmethod
  def from_json(cls, state):
    """Creates an instance of the OutputWriter for the given json state.

    Args:
      state: The OutputWriter state as a dict-like object.

    Returns:
      An instance of the OutputWriter configured using the values of json.
    """
    raise NotImplementedError("from_json() not implemented in %s" % cls)

  def to_json(self):
    """Returns writer state to serialize in json.

    Returns:
      A json-izable version of the OutputWriter state.
    """
    raise NotImplementedError("to_json() not implemented in %s" %
                              self.__class__)

  @classmethod
  def create(cls, mr_spec, shard_number, shard_attempt, _writer_state=None):
    """Create new writer for a shard.

    Args:
      mr_spec: an instance of model.MapreduceSpec describing current job.
      shard_number: int shard number.
      shard_attempt: int shard attempt.
      _writer_state: deprecated. This is for old writers that share file
        across shards. For new writers, each shard must have its own
        dedicated outputs. Output state should be contained in
        the output writer instance. The serialized output writer
        instance will be saved by mapreduce across slices.
    """
    raise NotImplementedError("create() not implemented in %s" % cls)

  def write(self, data):
    """Write data.

    Args:
      data: actual data yielded from handler. Type is writer-specific.
    """
    raise NotImplementedError("write() not implemented in %s" %
                              self.__class__)

  def finalize(self, ctx, shard_state):
    """Finalize writer shard-level state.

    This should only be called when shard_state.result_status shows success.
    After finalizing the outputs, it should save per-shard output file info
    into shard_state.writer_state so that other operations can find the
    outputs.

    Args:
      ctx: an instance of context.Context.
      shard_state: shard state. ShardState.writer_state can be modified.
    """
    raise NotImplementedError("finalize() not implemented in %s" %
                              self.__class__)

  @classmethod
  def get_filenames(cls, mapreduce_state):
    """Obtain output filenames from mapreduce state.

    This method should only be called when a MR is finished. Implementors of
    this method should not assume any other methods of this class have been
    called. In the case of no input data, no other method except validate
    would have been called.

    Args:
      mapreduce_state: an instance of model.MapreduceState

    Returns:
      List of filenames this mapreduce successfully wrote to. The list can be
    empty if no output file was successfully written.
    """
    raise NotImplementedError("get_filenames() not implemented in %s" % cls)

  # pylint: disable=unused-argument
  def _supports_shard_retry(self, tstate):
    """Whether this output writer instance supports shard retry.

    Args:
      tstate: model.TransientShardState for current shard.

    Returns:
      boolean. Whether this output writer instance supports shard retry.
    """
    return False

  def _supports_slice_recovery(self, mapper_spec):
    """Whether this output writer supports slice recovery.

    Args:
      mapper_spec: instance of model.MapperSpec.

    Returns:
      boolean. Whether this output writer instance supports slice recovery.
    """
    return False

  # pylint: disable=unused-argument
  def _recover(self, mr_spec, shard_number, shard_attempt):
    """Create a new output writer instance from the old one.

    This method is called when _supports_slice_recovery returns True,
    and when there is a chance the old output writer instance is out of sync
    with its storage medium due to a retry of a slice. _recover should
    create a new instance based on the old one. When finalize is called
    on the new instance, it could combine valid outputs from all instances
    to generate the final output. How the new instance maintains references
    to previous outputs is up to implementation.

    Any exception during recovery is subject to normal slice/shard retry.
    So recovery logic must be idempotent.

    Args:
      mr_spec: an instance of model.MapreduceSpec describing current job.
      shard_number: int shard number.
      shard_attempt: int shard attempt.

    Returns:
      a new instance of output writer.
    """
    raise NotImplementedError()


# Flush size for files api write requests. Approximately one block of data.
_FILE_POOL_FLUSH_SIZE = 128*1024

# Maximum size of files api request. Slightly less than 1M.
_FILE_POOL_MAX_SIZE = 1000*1024


def _get_params(mapper_spec, allowed_keys=None, allow_old=True):
  """Obtain output writer parameters.

  Utility function for output writer implementation. Fetches parameters
  from mapreduce specification giving appropriate usage warnings.

  Args:
    mapper_spec: The MapperSpec for the job
    allowed_keys: set of all allowed keys in parameters as strings. If it is not
      None, then parameters are expected to be in a separate "output_writer"
      subdictionary of mapper_spec parameters.
    allow_old: Allow parameters to exist outside of the output_writer
      subdictionary for compatability.

  Returns:
    mapper parameters as dict

  Raises:
    BadWriterParamsError: if parameters are invalid/missing or not allowed.
  """
  if "output_writer" not in mapper_spec.params:
    message = (
        "Output writer's parameters should be specified in "
        "output_writer subdictionary.")
    if not allow_old or allowed_keys:
      raise errors.BadWriterParamsError(message)
    params = mapper_spec.params
    params = dict((str(n), v) for n, v in params.iteritems())
  else:
    if not isinstance(mapper_spec.params.get("output_writer"), dict):
      raise errors.BadWriterParamsError(
          "Output writer parameters should be a dictionary")
    params = mapper_spec.params.get("output_writer")
    params = dict((str(n), v) for n, v in params.iteritems())
    if allowed_keys:
      params_diff = set(params.keys()) - allowed_keys
      if params_diff:
        raise errors.BadWriterParamsError(
            "Invalid output_writer parameters: %s" % ",".join(params_diff))
  return params


class _RecordsPoolBase(context.Pool):
  """Base class for Pool of append operations for records files."""

  # Approximate number of bytes of overhead for storing one record.
  _RECORD_OVERHEAD_BYTES = 10

  def __init__(self,
               flush_size_chars=_FILE_POOL_FLUSH_SIZE,
               ctx=None,
               exclusive=False):
    """Constructor.

    Any classes that subclass this will need to implement the _write() function.

    Args:
      flush_size_chars: buffer flush threshold as int.
      ctx: mapreduce context as context.Context.
      exclusive: a boolean flag indicating if the pool has an exclusive
        access to the file. If it is True, then it's possible to write
        bigger chunks of data.
    """
    self._flush_size = flush_size_chars
    self._buffer = []
    self._size = 0
    self._ctx = ctx
    self._exclusive = exclusive

  def append(self, data):
    """Append data to a file."""
    data_length = len(data)
    if self._size + data_length > self._flush_size:
      self.flush()

    if not self._exclusive and data_length > _FILE_POOL_MAX_SIZE:
      raise errors.Error(
          "Too big input %s (%s)."  % (data_length, _FILE_POOL_MAX_SIZE))
    else:
      self._buffer.append(data)
      self._size += data_length

    if self._size > self._flush_size:
      self.flush()

  def flush(self):
    """Flush pool contents."""
    # Write data to in-memory buffer first.
    buf = cStringIO.StringIO()
    with records.RecordsWriter(buf) as w:
      for record in self._buffer:
        w.write(record)
      w._pad_block()
    str_buf = buf.getvalue()
    buf.close()

    if not self._exclusive and len(str_buf) > _FILE_POOL_MAX_SIZE:
      # Shouldn't really happen because of flush size.
      raise errors.Error(
          "Buffer too big. Can't write more than %s bytes in one request: "
          "risk of writes interleaving. Got: %s" %
          (_FILE_POOL_MAX_SIZE, len(str_buf)))

    # Write data to file.
    start_time = time.time()
    self._write(str_buf)
    if self._ctx:
      operation.counters.Increment(
          COUNTER_IO_WRITE_BYTES, len(str_buf))(self._ctx)
      operation.counters.Increment(
          COUNTER_IO_WRITE_MSEC,
          int((time.time() - start_time) * 1000))(self._ctx)

    # reset buffer
    self._buffer = []
    self._size = 0
    gc.collect()

  def _write(self, str_buf):
    raise NotImplementedError("_write() not implemented in %s" % type(self))

  def __enter__(self):
    return self

  def __exit__(self, atype, value, traceback):
    self.flush()


class GCSRecordsPool(_RecordsPoolBase):
  """Pool of append operations for records using GCS."""

  # GCS writes in 256K blocks.
  _GCS_BLOCK_SIZE = 256 * 1024  # 256K

  def __init__(self,
               filehandle,
               flush_size_chars=_FILE_POOL_FLUSH_SIZE,
               ctx=None,
               exclusive=False):
    """Requires the filehandle of an open GCS file to write to."""
    super(GCSRecordsPool, self).__init__(flush_size_chars, ctx, exclusive)
    self._filehandle = filehandle
    self._buf_size = 0

  def _write(self, str_buf):
    """Uses the filehandle to the file in GCS to write to it."""
    self._filehandle.write(str_buf)
    self._buf_size += len(str_buf)

  def flush(self, force=False):
    """Flush pool contents.

    Args:
      force: Inserts additional padding to achieve the minimum block size
        required for GCS.
    """
    super(GCSRecordsPool, self).flush()
    if force:
      extra_padding = self._buf_size % self._GCS_BLOCK_SIZE
      if extra_padding > 0:
        self._write("\x00" * (self._GCS_BLOCK_SIZE - extra_padding))
    self._filehandle.flush()


class _GoogleCloudStorageBase(shard_life_cycle._ShardLifeCycle,
                              OutputWriter):
  """Base abstract class for all GCS writers.

  Required configuration in the mapper_spec.output_writer dictionary.
    BUCKET_NAME_PARAM: name of the bucket to use (with no extra delimiters or
      suffixes such as directories. Directories/prefixes can be specifed as
      part of the NAMING_FORMAT_PARAM).

  Optional configuration in the mapper_spec.output_writer dictionary:
    ACL_PARAM: acl to apply to new files, else bucket default used.
    NAMING_FORMAT_PARAM: prefix format string for the new files (there is no
      required starting slash, expected formats would look like
      "directory/basename...", any starting slash will be treated as part of
      the file name) that should use the following substitutions:
        $name - the name of the job
        $id - the id assigned to the job
        $num - the shard number
      If there is more than one shard $num must be used. An arbitrary suffix may
      be applied by the writer.
    CONTENT_TYPE_PARAM: mime type to apply on the files. If not provided, Google
      Cloud Storage will apply its default.
    TMP_BUCKET_NAME_PARAM: name of the bucket used for writing tmp files by
      consistent GCS output writers. Defaults to BUCKET_NAME_PARAM if not set.
  """

  BUCKET_NAME_PARAM = "bucket_name"
  TMP_BUCKET_NAME_PARAM = "tmp_bucket_name"
  ACL_PARAM = "acl"
  NAMING_FORMAT_PARAM = "naming_format"
  CONTENT_TYPE_PARAM = "content_type"

  # Internal parameter.
  _ACCOUNT_ID_PARAM = "account_id"
  _TMP_ACCOUNT_ID_PARAM = "tmp_account_id"

  @classmethod
  def _get_gcs_bucket(cls, writer_spec):
    return writer_spec[cls.BUCKET_NAME_PARAM]

  @classmethod
  def _get_account_id(cls, writer_spec):
    return writer_spec.get(cls._ACCOUNT_ID_PARAM, None)

  @classmethod
  def _get_tmp_gcs_bucket(cls, writer_spec):
    """Returns bucket used for writing tmp files."""
    if cls.TMP_BUCKET_NAME_PARAM in writer_spec:
      return writer_spec[cls.TMP_BUCKET_NAME_PARAM]
    return cls._get_gcs_bucket(writer_spec)

  @classmethod
  def _get_tmp_account_id(cls, writer_spec):
    """Returns the account id to use with tmp bucket."""
    # pick tmp id iff tmp bucket is set explicitly
    if cls.TMP_BUCKET_NAME_PARAM in writer_spec:
      return writer_spec.get(cls._TMP_ACCOUNT_ID_PARAM, None)
    return cls._get_account_id(writer_spec)


class _GoogleCloudStorageOutputWriterBase(_GoogleCloudStorageBase):
  """Base class for GCS writers directly interacting with GCS.

  Base class for both _GoogleCloudStorageOutputWriter and
  GoogleCloudStorageConsistentOutputWriter.

  This class is expected to be subclassed with a writer that applies formatting
  to user-level records.

  Subclasses need to define to_json, from_json, create, finalize and
  _get_write_buffer methods.

  See _GoogleCloudStorageBase for config options.
  """

  # Default settings
  _DEFAULT_NAMING_FORMAT = "$name/$id/output-$num"

  # Internal parameters
  _MR_TMP = "gae_mr_tmp"
  _TMP_FILE_NAMING_FORMAT = (
      _MR_TMP + "/$name/$id/attempt-$attempt/output-$num/seg-$seg")

  @classmethod
  def _generate_filename(cls, writer_spec, name, job_id, num,
                         attempt=None, seg_index=None):
    """Generates a filename for a particular output.

    Args:
      writer_spec: specification dictionary for the output writer.
      name: name of the job.
      job_id: the ID number assigned to the job.
      num: shard number.
      attempt: the shard attempt number.
      seg_index: index of the seg. None means the final output.

    Returns:
      a string containing the filename.

    Raises:
      BadWriterParamsError: if the template contains any errors such as invalid
        syntax or contains unknown substitution placeholders.
    """
    naming_format = cls._TMP_FILE_NAMING_FORMAT
    if seg_index is None:
      naming_format = writer_spec.get(cls.NAMING_FORMAT_PARAM,
                                      cls._DEFAULT_NAMING_FORMAT)

    template = string.Template(naming_format)
    try:
      # Check that template doesn't use undefined mappings and is formatted well
      if seg_index is None:
        return template.substitute(name=name, id=job_id, num=num)
      else:
        return template.substitute(name=name, id=job_id, num=num,
                                   attempt=attempt,
                                   seg=seg_index)
    except ValueError, error:
      raise errors.BadWriterParamsError("Naming template is bad, %s" % (error))
    except KeyError, error:
      raise errors.BadWriterParamsError("Naming template '%s' has extra "
                                        "mappings, %s" % (naming_format, error))

  @classmethod
  def get_params(cls, mapper_spec, allowed_keys=None, allow_old=True):
    params = _get_params(mapper_spec, allowed_keys, allow_old)
    # Use the bucket_name defined in mapper_spec params if one was not defined
    # specifically in the output_writer params.
    if (mapper_spec.params.get(cls.BUCKET_NAME_PARAM) is not None and
        params.get(cls.BUCKET_NAME_PARAM) is None):
      params[cls.BUCKET_NAME_PARAM] = mapper_spec.params[cls.BUCKET_NAME_PARAM]
    return params

  @classmethod
  def validate(cls, mapper_spec):
    """Validate mapper specification.

    Args:
      mapper_spec: an instance of model.MapperSpec.

    Raises:
      BadWriterParamsError: if the specification is invalid for any reason such
        as missing the bucket name or providing an invalid bucket name.
    """
    writer_spec = cls.get_params(mapper_spec, allow_old=False)

    # Bucket Name is required
    if cls.BUCKET_NAME_PARAM not in writer_spec:
      raise errors.BadWriterParamsError(
          "%s is required for Google Cloud Storage" %
          cls.BUCKET_NAME_PARAM)
    try:
      cloudstorage.validate_bucket_name(
          writer_spec[cls.BUCKET_NAME_PARAM])
    except ValueError, error:
      raise errors.BadWriterParamsError("Bad bucket name, %s" % (error))

    # Validate the naming format does not throw any errors using dummy values
    cls._generate_filename(writer_spec, "name", "id", 0)
    cls._generate_filename(writer_spec, "name", "id", 0, 1, 0)

  @classmethod
  def _open_file(cls, writer_spec, filename_suffix, use_tmp_bucket=False):
    """Opens a new gcs file for writing."""
    if use_tmp_bucket:
      bucket = cls._get_tmp_gcs_bucket(writer_spec)
      account_id = cls._get_tmp_account_id(writer_spec)
    else:
      bucket = cls._get_gcs_bucket(writer_spec)
      account_id = cls._get_account_id(writer_spec)

    # GoogleCloudStorage format for filenames, Initial slash is required
    filename = "/%s/%s" % (bucket, filename_suffix)

    content_type = writer_spec.get(cls.CONTENT_TYPE_PARAM, None)

    options = {}
    if cls.ACL_PARAM in writer_spec:
      options["x-goog-acl"] = writer_spec.get(cls.ACL_PARAM)

    return cloudstorage.open(filename, mode="w", content_type=content_type,
                             options=options, _account_id=account_id)

  @classmethod
  def _get_filename(cls, shard_state):
    return shard_state.writer_state["filename"]

  @classmethod
  def get_filenames(cls, mapreduce_state):
    filenames = []
    for shard in model.ShardState.find_all_by_mapreduce_state(mapreduce_state):
      if shard.result_status == model.ShardState.RESULT_SUCCESS:
        filenames.append(cls._get_filename(shard))
    return filenames

  def _get_write_buffer(self):
    """Returns a buffer to be used by the write() method."""
    raise NotImplementedError()

  def write(self, data):
    """Write data to the GoogleCloudStorage file.

    Args:
      data: string containing the data to be written.
    """
    start_time = time.time()
    self._get_write_buffer().write(data)
    ctx = context.get()
    operation.counters.Increment(COUNTER_IO_WRITE_BYTES, len(data))(ctx)
    operation.counters.Increment(
        COUNTER_IO_WRITE_MSEC, int((time.time() - start_time) * 1000))(ctx)

  # pylint: disable=unused-argument
  def _supports_shard_retry(self, tstate):
    return True


class _GoogleCloudStorageOutputWriter(_GoogleCloudStorageOutputWriterBase):
  """Naive version of GoogleCloudStorageWriter.

  This version is known to create inconsistent outputs if the input changes
  during slice retries. Consider using GoogleCloudStorageConsistentOutputWriter
  instead.

  Optional configuration in the mapper_spec.output_writer dictionary:
    _NO_DUPLICATE: if True, slice recovery logic will be used to ensure
      output files has no duplicates. Every shard should have only one final
      output in user specified location. But it may produce many smaller
      files (named "seg") due to slice recovery. These segs live in a
      tmp directory and should be combined and renamed to the final location.
      In current impl, they are not combined.
  """
  _SEG_PREFIX = "seg_prefix"
  _LAST_SEG_INDEX = "last_seg_index"
  _JSON_GCS_BUFFER = "buffer"
  _JSON_SEG_INDEX = "seg_index"
  _JSON_NO_DUP = "no_dup"
  # This can be used to store valid length with a GCS file.
  _VALID_LENGTH = "x-goog-meta-gae-mr-valid-length"
  _NO_DUPLICATE = "no_duplicate"

  # writer_spec only used by subclasses, pylint: disable=unused-argument
  def __init__(self, streaming_buffer, writer_spec=None):
    """Initialize a GoogleCloudStorageOutputWriter instance.

    Args:
      streaming_buffer: an instance of writable buffer from cloudstorage_api.

      writer_spec: the specification for the writer.
    """
    self._streaming_buffer = streaming_buffer
    self._no_dup = False
    if writer_spec:
      self._no_dup = writer_spec.get(self._NO_DUPLICATE, False)
    if self._no_dup:
      # This is the index of the current seg, starting at 0.
      # This number is incremented sequentially and every index
      # represents a real seg.
      self._seg_index = int(streaming_buffer.name.rsplit("-", 1)[1])
      # The valid length of the current seg by the end of the previous slice.
      # This value is updated by the end of a slice, by which time,
      # all content before this have already been either
      # flushed to GCS or serialized to task payload.
      self._seg_valid_length = 0

  @classmethod
  def validate(cls, mapper_spec):
    """Inherit docs."""
    writer_spec = cls.get_params(mapper_spec, allow_old=False)
    if writer_spec.get(cls._NO_DUPLICATE, False) not in (True, False):
      raise errors.BadWriterParamsError("No duplicate must a boolean.")
    super(_GoogleCloudStorageOutputWriter, cls).validate(mapper_spec)

  def _get_write_buffer(self):
    return self._streaming_buffer

  @classmethod
  def create(cls, mr_spec, shard_number, shard_attempt, _writer_state=None):
    """Inherit docs."""
    writer_spec = cls.get_params(mr_spec.mapper, allow_old=False)
    seg_index = None
    if writer_spec.get(cls._NO_DUPLICATE, False):
      seg_index = 0

    # Determine parameters
    key = cls._generate_filename(writer_spec, mr_spec.name,
                                 mr_spec.mapreduce_id,
                                 shard_number, shard_attempt,
                                 seg_index)
    return cls._create(writer_spec, key)

  @classmethod
  def _create(cls, writer_spec, filename_suffix):
    """Helper method that actually creates the file in cloud storage."""
    writer = cls._open_file(writer_spec, filename_suffix)
    return cls(writer, writer_spec=writer_spec)

  @classmethod
  def from_json(cls, state):
    writer = cls(pickle.loads(state[cls._JSON_GCS_BUFFER]))
    no_dup = state.get(cls._JSON_NO_DUP, False)
    writer._no_dup = no_dup
    if no_dup:
      writer._seg_valid_length = state[cls._VALID_LENGTH]
      writer._seg_index = state[cls._JSON_SEG_INDEX]
    return writer

  def end_slice(self, slice_ctx):
    if not self._streaming_buffer.closed:
      self._streaming_buffer.flush()

  def to_json(self):
    result = {self._JSON_GCS_BUFFER: pickle.dumps(self._streaming_buffer),
              self._JSON_NO_DUP: self._no_dup}
    if self._no_dup:
      result.update({
          # Save the length of what has been written, including what is
          # buffered in memory.
          # This assumes from_json and to_json are only called
          # at the beginning of a slice.
          # TODO(user): This may not be a good assumption.
          self._VALID_LENGTH: self._streaming_buffer.tell(),
          self._JSON_SEG_INDEX: self._seg_index})
    return result

  def finalize(self, ctx, shard_state):
    self._streaming_buffer.close()

    if self._no_dup:
      cloudstorage_api.copy2(
          self._streaming_buffer.name,
          self._streaming_buffer.name,
          metadata={self._VALID_LENGTH: self._streaming_buffer.tell()})

      # The filename user requested.
      mr_spec = ctx.mapreduce_spec
      writer_spec = self.get_params(mr_spec.mapper, allow_old=False)
      filename = self._generate_filename(writer_spec,
                                         mr_spec.name,
                                         mr_spec.mapreduce_id,
                                         shard_state.shard_number)
      seg_filename = self._streaming_buffer.name
      prefix, last_index = seg_filename.rsplit("-", 1)
      # These info is enough for any external process to combine
      # all segs into the final file.
      # TODO(user): Create a special input reader to combine segs.
      shard_state.writer_state = {self._SEG_PREFIX: prefix + "-",
                                  self._LAST_SEG_INDEX: int(last_index),
                                  "filename": filename}
    else:
      shard_state.writer_state = {"filename": self._streaming_buffer.name}

  def _supports_slice_recovery(self, mapper_spec):
    writer_spec = self.get_params(mapper_spec, allow_old=False)
    return writer_spec.get(self._NO_DUPLICATE, False)

  def _recover(self, mr_spec, shard_number, shard_attempt):
    next_seg_index = self._seg_index

    # Save the current seg if it actually has something.
    # Remember self._streaming_buffer is the pickled instance
    # from the previous slice.
    if self._seg_valid_length != 0:
      try:
        gcs_next_offset = self._streaming_buffer._get_offset_from_gcs() + 1
        # If GCS is ahead of us, just force close.
        if gcs_next_offset > self._streaming_buffer.tell():
          self._streaming_buffer._force_close(gcs_next_offset)
        # Otherwise flush in memory contents too.
        else:
          self._streaming_buffer.close()
      except cloudstorage.FileClosedError:
        pass
      cloudstorage_api.copy2(
          self._streaming_buffer.name,
          self._streaming_buffer.name,
          metadata={self._VALID_LENGTH:
                    self._seg_valid_length})
      next_seg_index = self._seg_index + 1

    writer_spec = self.get_params(mr_spec.mapper, allow_old=False)
    # Create name for the new seg.
    key = self._generate_filename(
        writer_spec, mr_spec.name,
        mr_spec.mapreduce_id,
        shard_number,
        shard_attempt,
        next_seg_index)
    new_writer = self._create(writer_spec, key)
    new_writer._seg_index = next_seg_index
    return new_writer

  def _get_filename_for_test(self):
    return self._streaming_buffer.name


GoogleCloudStorageOutputWriter = _GoogleCloudStorageOutputWriter


class _ConsistentStatus(object):
  """Object used to pass status to the next slice."""

  def __init__(self):
    self.writer_spec = None
    self.mapreduce_id = None
    self.shard = None
    self.mainfile = None
    self.tmpfile = None
    self.tmpfile_1ago = None


class GoogleCloudStorageConsistentOutputWriter(
    _GoogleCloudStorageOutputWriterBase):
  """Output writer to Google Cloud Storage using the cloudstorage library.

  This version ensures that the output written to GCS is consistent.
  """

  # Implementation details:
  # Each slice writes to a new tmpfile in GCS. When the slice is finished
  # (to_json is called) the file is finalized. When slice N is started
  # (from_json is called) it does the following:
  # - append the contents of N-1's tmpfile to the mainfile
  # - remove N-2's tmpfile
  #
  # When a slice fails the file is never finalized and will be garbage
  # collected. It is possible for the slice to fail just after the file is
  # finalized. We will leave a file behind in this case (we don't clean it up).
  #
  # Slice retries don't cause inconsitent and/or duplicate entries to be written
  # to the mainfile (rewriting tmpfile is an idempotent operation).

  _JSON_STATUS = "status"
  _RAND_BITS = 128
  _REWRITE_BLOCK_SIZE = 1024 * 256
  _REWRITE_MR_TMP = "gae_mr_tmp"
  _TMPFILE_PATTERN = _REWRITE_MR_TMP + "/$id-tmp-$shard-$random"
  _TMPFILE_PREFIX = _REWRITE_MR_TMP + "/$id-tmp-$shard-"

  def __init__(self, status):
    """Initialize a GoogleCloudStorageConsistentOutputWriter instance.

    Args:
      status: an instance of _ConsistentStatus with initialized tmpfile
              and mainfile.
    """

    self.status = status
    self._data_written_to_slice = False

  def _get_write_buffer(self):
    if not self.status.tmpfile:
      raise errors.FailJobError(
          "write buffer called but empty, begin_slice missing?")
    return self.status.tmpfile

  def _get_filename_for_test(self):
    return self.status.mainfile.name

  @classmethod
  def create(cls, mr_spec, shard_number, shard_attempt, _writer_state=None):
    """Inherit docs."""
    writer_spec = cls.get_params(mr_spec.mapper, allow_old=False)

    # Determine parameters
    key = cls._generate_filename(writer_spec, mr_spec.name,
                                 mr_spec.mapreduce_id,
                                 shard_number, shard_attempt)

    status = _ConsistentStatus()
    status.writer_spec = writer_spec
    status.mainfile = cls._open_file(writer_spec, key)
    status.mapreduce_id = mr_spec.mapreduce_id
    status.shard = shard_number

    return cls(status)

  def _remove_tmpfile(self, filename, writer_spec):
    if not filename:
      return
    account_id = self._get_tmp_account_id(writer_spec)
    try:
      cloudstorage_api.delete(filename, _account_id=account_id)
    except cloud_errors.NotFoundError:
      pass

  def _rewrite_tmpfile(self, mainfile, tmpfile, writer_spec):
    """Copies contents of tmpfile (name) to mainfile (buffer)."""
    if mainfile.closed:
      # can happen when finalize fails
      return

    account_id = self._get_tmp_account_id(writer_spec)
    f = cloudstorage_api.open(tmpfile, _account_id=account_id)
    # both reads and writes are buffered - the number here doesn't matter
    data = f.read(self._REWRITE_BLOCK_SIZE)
    while data:
      mainfile.write(data)
      data = f.read(self._REWRITE_BLOCK_SIZE)
    f.close()
    mainfile.flush()

  @classmethod
  def _create_tmpfile(cls, status):
    """Creates a new random-named tmpfile."""

    # We can't put the tmpfile in the same directory as the output. There are
    # rare circumstances when we leave trash behind and we don't want this trash
    # to be loaded into bigquery and/or used for restore.
    #
    # We used mapreduce id, shard number and attempt and 128 random bits to make
    # collisions virtually impossible.
    tmpl = string.Template(cls._TMPFILE_PATTERN)
    filename = tmpl.substitute(
        id=status.mapreduce_id, shard=status.shard,
        random=random.getrandbits(cls._RAND_BITS))

    return cls._open_file(status.writer_spec, filename, use_tmp_bucket=True)

  def begin_slice(self, slice_ctx):
    status = self.status
    writer_spec = status.writer_spec

    # we're slice N so we can safely remove N-2's tmpfile
    if status.tmpfile_1ago:
      self._remove_tmpfile(status.tmpfile_1ago.name, writer_spec)

    # rewrite N-1's tmpfile (idempotent)
    # N-1 file might be needed if this this slice is ever retried so we need
    # to make sure it won't be cleaned up just yet.
    files_to_keep = []
    if status.tmpfile:  # does no exist on slice 0
      self._rewrite_tmpfile(status.mainfile, status.tmpfile.name, writer_spec)
      files_to_keep.append(status.tmpfile.name)

    # clean all the garbage you can find
    self._try_to_clean_garbage(
        writer_spec, exclude_list=files_to_keep)

    # Rotate the files in status.
    status.tmpfile_1ago = status.tmpfile
    status.tmpfile = self._create_tmpfile(status)

    # There's a test for this condition. Not sure if this can happen.
    if status.mainfile.closed:
      status.tmpfile.close()
      self._remove_tmpfile(status.tmpfile.name, writer_spec)

  @classmethod
  def from_json(cls, state):
    return cls(pickle.loads(state[cls._JSON_STATUS]))

  def end_slice(self, slice_ctx):
    self.status.tmpfile.close()

  def to_json(self):
    return {self._JSON_STATUS: pickle.dumps(self.status)}

  def write(self, data):
    super(GoogleCloudStorageConsistentOutputWriter, self).write(data)
    self._data_written_to_slice = True

  def _try_to_clean_garbage(self, writer_spec, exclude_list=()):
    """Tries to remove any files created by this shard that aren't needed.

    Args:
      writer_spec: writer_spec for the MR.
      exclude_list: A list of filenames (strings) that should not be
        removed.
    """
    # Try to remove garbage (if any). Note that listbucket is not strongly
    # consistent so something might survive.
    tmpl = string.Template(self._TMPFILE_PREFIX)
    prefix = tmpl.substitute(
        id=self.status.mapreduce_id, shard=self.status.shard)
    bucket = self._get_tmp_gcs_bucket(writer_spec)
    account_id = self._get_tmp_account_id(writer_spec)
    for f in cloudstorage.listbucket("/%s/%s" % (bucket, prefix),
                                     _account_id=account_id):
      if f.filename not in exclude_list:
        self._remove_tmpfile(f.filename, self.status.writer_spec)

  def finalize(self, ctx, shard_state):
    if self._data_written_to_slice:
      raise errors.FailJobError(
          "finalize() called after data was written")

    if self.status.tmpfile:
      self.status.tmpfile.close()  # it's empty
    self.status.mainfile.close()

    # rewrite happened, close happened, we can remove the tmp files
    if self.status.tmpfile_1ago:
      self._remove_tmpfile(self.status.tmpfile_1ago.name,
                           self.status.writer_spec)
    if self.status.tmpfile:
      self._remove_tmpfile(self.status.tmpfile.name,
                           self.status.writer_spec)

    self._try_to_clean_garbage(self.status.writer_spec)

    shard_state.writer_state = {"filename": self.status.mainfile.name}


class _GoogleCloudStorageRecordOutputWriterBase(_GoogleCloudStorageBase):
  """Wraps a GCS writer with a records.RecordsWriter.

  This class wraps a WRITER_CLS (and its instance) and delegates most calls
  to it. write() calls are done using records.RecordsWriter.

  WRITER_CLS has to be set to a subclass of _GoogleCloudStorageOutputWriterBase.

  For list of supported parameters see _GoogleCloudStorageBase.
  """

  WRITER_CLS = None

  def __init__(self, writer):
    self._writer = writer
    self._record_writer = records.RecordsWriter(writer)

  @classmethod
  def validate(cls, mapper_spec):
    return cls.WRITER_CLS.validate(mapper_spec)

  @classmethod
  def init_job(cls, mapreduce_state):
    return cls.WRITER_CLS.init_job(mapreduce_state)

  @classmethod
  def finalize_job(cls, mapreduce_state):
    return cls.WRITER_CLS.finalize_job(mapreduce_state)

  @classmethod
  def from_json(cls, state):
    return cls(cls.WRITER_CLS.from_json(state))

  def to_json(self):
    return self._writer.to_json()

  @classmethod
  def create(cls, mr_spec, shard_number, shard_attempt, _writer_state=None):
    return cls(cls.WRITER_CLS.create(mr_spec, shard_number, shard_attempt,
                                     _writer_state))

  def write(self, data):
    self._record_writer.write(data)

  def finalize(self, ctx, shard_state):
    return self._writer.finalize(ctx, shard_state)

  @classmethod
  def get_filenames(cls, mapreduce_state):
    return cls.WRITER_CLS.get_filenames(mapreduce_state)

  def _supports_shard_retry(self, tstate):
    return self._writer._supports_shard_retry(tstate)

  def _supports_slice_recovery(self, mapper_spec):
    return self._writer._supports_slice_recovery(mapper_spec)

  def _recover(self, mr_spec, shard_number, shard_attempt):
    return self._writer._recover(mr_spec, shard_number, shard_attempt)

  def begin_slice(self, slice_ctx):
    return self._writer.begin_slice(slice_ctx)

  def end_slice(self, slice_ctx):
    # Pad if this is not the end_slice call after finalization.
    if not self._writer._get_write_buffer().closed:
      self._record_writer._pad_block()
    return self._writer.end_slice(slice_ctx)


class _GoogleCloudStorageRecordOutputWriter(
    _GoogleCloudStorageRecordOutputWriterBase):
  WRITER_CLS = _GoogleCloudStorageOutputWriter


GoogleCloudStorageRecordOutputWriter = _GoogleCloudStorageRecordOutputWriter


class GoogleCloudStorageConsistentRecordOutputWriter(
    _GoogleCloudStorageRecordOutputWriterBase):
  WRITER_CLS = GoogleCloudStorageConsistentOutputWriter


# TODO(user): Write a test for this.
class _GoogleCloudStorageKeyValueOutputWriter(
    _GoogleCloudStorageRecordOutputWriter):
  """Write key/values to Google Cloud Storage files in LevelDB format."""

  def write(self, data):
    if len(data) != 2:
      logging.error("Got bad tuple of length %d (2-tuple expected): %s",
                    len(data), data)

    try:
      key = str(data[0])
      value = str(data[1])
    except TypeError:
      logging.error("Expecting a tuple, but got %s: %s",
                    data.__class__.__name__, data)

    proto = kv_pb.KeyValue()
    proto.set_key(key)
    proto.set_value(value)
    GoogleCloudStorageRecordOutputWriter.write(self, proto.Encode())


GoogleCloudStorageKeyValueOutputWriter = _GoogleCloudStorageKeyValueOutputWriter
