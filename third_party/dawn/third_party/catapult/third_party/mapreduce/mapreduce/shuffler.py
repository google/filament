#!/usr/bin/env python
# Copyright 2011 Google Inc. All Rights Reserved.
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


"""Mapreduce shuffler implementation."""

from __future__ import with_statement




__all__ = [
    "ShufflePipeline",
    ]

# Using opensource naming conventions, pylint: disable=g-bad-name

import gc
import heapq
import logging
import pickle
import time

import pipeline
from pipeline import common as pipeline_common
from google.appengine.ext import db
from mapreduce import context
from mapreduce import errors
from mapreduce import input_readers
from mapreduce import kv_pb
from mapreduce import mapper_pipeline
from mapreduce import model
from mapreduce import operation
from mapreduce import output_writers
from mapreduce import pipeline_base
from mapreduce import records
from mapreduce import util

# pylint: disable=g-import-not-at-top
# TODO(user): Cleanup imports if/when cloudstorage becomes part of runtime.
try:
  # Check if the full cloudstorage package exists. The stub part is in runtime.
  import cloudstorage
  if hasattr(cloudstorage, "_STUB"):
    cloudstorage = None
except ImportError:
  pass  # CloudStorage library not available


# pylint: disable=g-bad-name
# pylint: disable=protected-access


class _OutputFile(db.Model):
  """Entity to store output filenames of pipelines.

  These entities are always children of key returned by get_root_key().
  """

  @classmethod
  def kind(cls):
    """Returns entity kind."""
    return "_AE_MR_OutputFile"

  @classmethod
  def get_root_key(cls, job_id):
    """Get root key to store output files.

    Args:
      job_id: pipeline's job id.

    Returns:
      root key for a given job id to store output file entities.
    """
    return db.Key.from_path(cls.kind(), job_id)


def _compare_keys(key_record1, key_record2):
  """Compare two (key, records) protos by key."""
  return cmp(key_record1[0], key_record2[0])


class _BatchGCSRecordsReader(
    input_readers._GoogleCloudStorageRecordInputReader):
  """GCS Records reader that reads in big batches."""

  BATCH_SIZE = 1024 *1024 * 3

  def __iter__(self):
    # pylint: disable=redefined-outer-name
    records = []
    size = 0
    try:
      while True:
        record = super(_BatchGCSRecordsReader, self).next()
        records.append(record)
        size += len(record)
        if size > self.BATCH_SIZE:
          yield records
          size = 0
          records = []
          gc.collect()
    except StopIteration:
      pass
    if records:
      yield records
      records = []
      gc.collect()


# pylint: disable=redefined-outer-name
def _sort_records_map(records):
  """Map function sorting records.

  Converts records to KeyValue protos, sorts them by key and writes them
  into new GCS file. Creates _OutputFile entity to record resulting
  file name.

  Args:
    records: list of records which are serialized KeyValue protos.
  """
  ctx = context.get()
  l = len(records)
  key_records = [None] * l

  logging.debug("Parsing")
  for i in range(l):
    proto = kv_pb.KeyValue()
    proto.ParseFromString(records[i])
    key_records[i] = (proto.key(), records[i])

  logging.debug("Sorting")
  key_records.sort(cmp=_compare_keys)

  logging.debug("Writing")
  mapper_spec = ctx.mapreduce_spec.mapper
  params = input_readers._get_params(mapper_spec)
  bucket_name = params.get("bucket_name")
  filename = (ctx.mapreduce_spec.name + "/" + ctx.mapreduce_id + "/output-" +
              ctx.shard_id + "-" + str(int(time.time())))
  full_filename = "/%s/%s" % (bucket_name, filename)
  filehandle = cloudstorage.open(full_filename, mode="w")
  with output_writers.GCSRecordsPool(filehandle, ctx=ctx) as pool:
    for key_record in key_records:
      pool.append(key_record[1])

  logging.debug("Finalizing")
  filehandle.close()

  entity = _OutputFile(key_name=full_filename,
                       parent=_OutputFile.get_root_key(ctx.mapreduce_id))
  entity.put()


class _SortChunksPipeline(pipeline_base.PipelineBase):
  """A pipeline to sort multiple key-value files.

  Args:
    job_name: root job name.
    bucket_name: The name of the Google Cloud Storage bucket.
    filenames: list of a list of filenames (hashed/bucketed) to sort,
      as produced by _HashingGCSOutputWriter.

  Returns:
    The list of lists of sorted filenames. Each list corresponds to each
    list of input files. Each filenames contains a chunk of sorted data.
  """

  def run(self, job_name, bucket_name, filenames):
    sort_mappers = []
    for i in range(len(filenames)):
      filenames_only = util.strip_prefix_from_items("/%s/" % bucket_name,
                                                    filenames[i])
      sort_mapper = yield mapper_pipeline.MapperPipeline(
          "%s-shuffle-sort-%s" % (job_name, str(i)),
          __name__ + "._sort_records_map",
          __name__ + "._BatchGCSRecordsReader",
          None,
          {
              "input_reader": {
                  "bucket_name": bucket_name,
                  "objects": filenames_only,
              },
          },
          shards=1)
      sort_mappers.append(sort_mapper)
    with pipeline.After(*sort_mappers):
      job_ids = yield pipeline_common.Append(*[mapper.job_id for mapper in
                                               sort_mappers])
      result = yield _CollectOutputFiles(job_ids)
      with pipeline.After(result):
        yield _CleanupOutputFiles(job_ids)
      yield pipeline_common.Return(result)


class _CollectOutputFiles(pipeline_base.PipelineBase):
  """Collect output file names from _OutputFile entities for given jobs.

  Args:
    job_ids: list of job ids to load filenames.

  Returns:
    list of lists of filenames produced by specified job ids.
  """

  def run(self, job_ids):
    result = []
    for job_id in job_ids:
      entities = _OutputFile.all().ancestor(_OutputFile.get_root_key(job_id))
      result.append([entity.key().name() for entity in entities])
    return result


class _CleanupOutputFiles(pipeline_base.PipelineBase):
  """Cleanup _OutputFile entities for given job ids.

  Args:
    job_ids: list of job ids.
  """

  def run(self, job_ids):
    for job_id in job_ids:
      db.delete(_OutputFile.all().ancestor(_OutputFile.get_root_key(job_id)))


class _MergingReader(input_readers.InputReader):
  """Reader which merge-reads multiple sorted KeyValue files.

  Reads list of lists of filenames. Each filename list constitutes one shard
  and is merged together.

  Yields (key, values) tuple. If none of the max_values_count and
  max_values_size parameters are not specified, then there will be a single key.
  Otherwise multiple (key, values) pairs for the same key will be created,
  according to restrictions.
  """

  expand_parameters = True

  FILES_PARAM = "files"
  MAX_VALUES_COUNT_PARAM = "max_values_count"
  MAX_VALUES_SIZE_PARAM = "max_values_size"

  # Use a smaller buffer than the default.
  GCS_BUFFER_SIZE = 256 * 1024  # 256K.

  def __init__(self,
               offsets,
               max_values_count,
               max_values_size):
    """Constructor.

    Args:
      offsets: offsets for each input file to start from as list of ints.
      max_values_count: maximum number of values to yield for a single value at
        a time. Ignored if -1.
      max_values_size: maximum total size of yielded values.  Ignored if -1
    """
    self._offsets = offsets
    self._max_values_count = max_values_count
    self._max_values_size = max_values_size

  def __iter__(self):
    """Iterate over records in input files.

    self._offsets is always correctly updated so that stopping iterations
    doesn't skip records and doesn't read the same record twice.

    Raises:
      Exception: when Files list and offsets do not match.

    Yields:
      The result.
    """
    ctx = context.get()
    mapper_spec = ctx.mapreduce_spec.mapper
    shard_number = ctx._shard_state.shard_number
    filenames = mapper_spec.params[self.FILES_PARAM][shard_number]

    if len(filenames) != len(self._offsets):
      raise Exception("Files list and offsets do not match.")

    # Heap with (Key, Value, Index, reader) pairs.
    readers = []

    # Initialize heap
    for (i, filename) in enumerate(filenames):
      offset = self._offsets[i]
      # TODO(user): Shrinking the buffer size is a workaround until
      # a tiered/segmented merge is implemented.
      reader = records.RecordsReader(
          cloudstorage.open(filename, read_buffer_size=self.GCS_BUFFER_SIZE))
      reader.seek(offset)
      readers.append((None, None, i, reader))

    # Read records from heap and merge values with the same key.

    # current_result is yielded and consumed buy _merge_map.
    # current_result = (key, value, is_partial)
    current_result = None
    current_count = 0
    current_size = 0
    while readers:
      (key, value, index, reader) = readers[0]

      if key is not None:
        current_count += 1
        current_size += len(value)

        should_yield = False
        if current_result:
          if key != current_result[0]:
            # New key encountered
            should_yield = True
          elif (self._max_values_count != -1 and
                current_count >= self._max_values_count):
            # Maximum number of values encountered.
            current_result[2] = True
            should_yield = True
          elif (self._max_values_size != -1 and
                current_size >= self._max_values_size):
            # Maximum size of values encountered
            current_result[2] = True
            should_yield = True

        if should_yield:
          # New key encountered or maximum count hit. Yield current key.
          yield current_result
        if not current_result or should_yield:
          current_result = [key, [], False]
          current_count = 0
          current_size = 0
        current_result[1].append(value)

      # Read next key/value from reader.
      try:
        self._offsets[index] = reader.tell()
        start_time = time.time()
        binary_record = reader.read()
        # update counters
        if context.get():
          operation.counters.Increment(
              input_readers.COUNTER_IO_READ_BYTES,
              len(binary_record))(context.get())
          operation.counters.Increment(
              input_readers.COUNTER_IO_READ_MSEC,
              int((time.time() - start_time) * 1000))(context.get())
        proto = kv_pb.KeyValue()
        proto.ParseFromString(binary_record)
        # Put read data back into heap.
        heapq.heapreplace(readers,
                          (proto.key(), proto.value(), index, reader))
      except EOFError:
        heapq.heappop(readers)

    # Yield leftovers.
    if current_result:
      yield current_result

  @classmethod
  def from_json(cls, json):
    """Restore reader from json state."""
    return cls(json["offsets"],
               json["max_values_count"],
               json["max_values_size"])

  def to_json(self):
    """Serialize reader state to json."""
    return {"offsets": self._offsets,
            "max_values_count": self._max_values_count,
            "max_values_size": self._max_values_size}

  @classmethod
  def split_input(cls, mapper_spec):
    """Split input into multiple shards."""
    filelists = mapper_spec.params[cls.FILES_PARAM]
    max_values_count = mapper_spec.params.get(cls.MAX_VALUES_COUNT_PARAM, -1)
    max_values_size = mapper_spec.params.get(cls.MAX_VALUES_SIZE_PARAM, -1)
    return [cls([0] * len(files), max_values_count, max_values_size)
            for files in filelists]

  @classmethod
  def validate(cls, mapper_spec):
    """Validate reader parameters in mapper_spec."""
    if mapper_spec.input_reader_class() != cls:
      raise errors.BadReaderParamsError("Input reader class mismatch")
    params = mapper_spec.params
    if cls.FILES_PARAM not in params:
      raise errors.BadReaderParamsError("Missing files parameter.")


class _HashingGCSOutputWriter(output_writers.OutputWriter):
  """An OutputWriter which outputs data into GCS in key-value format.

  The output is tailored towards shuffler needs. It shards key/values using
  key hash modulo number of output files. Each shard will hash keys that will
  be placed in one of shard_count number of files (buckets) specific to that
  shard. The same key will be hashed to the same logical file across all of
  the shards. Then the list of all the same logical files will be assembled
  and a list of those lists will be returned.
  """

  # Supported parameters
  BUCKET_NAME_PARAM = "bucket_name"

  # pylint: disable=super-init-not-called
  def __init__(self, filehandles):
    """Constructor.

    Args:
      filehandles: list of file handles that this writer outputs to.
    """
    self._filehandles = filehandles
    self._pools = [None] * len(filehandles)

  @classmethod
  def validate(cls, mapper_spec):
    """Validates mapper specification.

    Args:
      mapper_spec: an instance of model.MapperSpec to validate.
    Raises:
      BadWriterParamsError: when Output writer class mismatch.
    """
    if mapper_spec.output_writer_class() != cls:
      raise errors.BadWriterParamsError("Output writer class mismatch")
    params = output_writers._get_params(mapper_spec)
    # Bucket Name is required
    if cls.BUCKET_NAME_PARAM not in params:
      raise errors.BadWriterParamsError(
          "%s is required for the _HashingGCSOutputWriter" %
          cls.BUCKET_NAME_PARAM)

  @classmethod
  def from_json(cls, json):
    """Creates an instance of the OutputWriter for the given json state.

    Args:
      json: The OutputWriter state as a dict-like object.

    Returns:
      An instance of the OutputWriter configured using the values of json.
    """
    return cls(pickle.loads(json["filehandles"]))

  def to_json(self):
    """Returns writer state to serialize in json.

    Returns:
      A json-izable version of the OutputWriter state.
    """
    # Use the member variable (since we don't have access to the context) to
    # flush each pool to minimize the size of each filehandle before we
    # serialize it.
    for pool in self._pools:
      if pool is not None:
        pool.flush(True)
    return {"filehandles": pickle.dumps(self._filehandles)}

  @classmethod
  def create(cls, mr_spec, shard_number, shard_attempt, _writer_state=None):
    """Inherit docs."""
    mapper_spec = mr_spec.mapper
    params = output_writers._get_params(mapper_spec)
    bucket_name = params.get(cls.BUCKET_NAME_PARAM)
    shards = mapper_spec.shard_count

    filehandles = []
    filename = (mr_spec.name + "/" + mr_spec.mapreduce_id +
                "/shard-" + str(shard_number) + "-bucket-")
    for i in range(shards):
      full_filename = "/%s/%s%d" % (bucket_name, filename, i)
      filehandles.append(cloudstorage.open(full_filename, mode="w"))
    return cls(filehandles)

  @classmethod
  def get_filenames(cls, mapreduce_state):
    """See parent class."""
    shards = mapreduce_state.mapreduce_spec.mapper.shard_count
    filenames = []
    for _ in range(shards):
      filenames.append([None] * shards)
    shard_states = model.ShardState.find_all_by_mapreduce_state(mapreduce_state)
    for x, shard_state in enumerate(shard_states):
      shard_filenames = shard_state.writer_state["shard_filenames"]
      for y in range(shards):
        filenames[y][x] = shard_filenames[y]
    return filenames

  def finalize(self, ctx, shard_state):
    """See parent class."""
    filenames = []
    for filehandle in self._filehandles:
      filenames.append(filehandle.name)
      filehandle.close()
    shard_state.writer_state = {"shard_filenames": filenames}

  def write(self, data):
    """Write data.

    Args:
      data: actual data yielded from handler. Type is writer-specific.
    """
    ctx = context.get()
    if len(data) != 2:
      logging.error("Got bad tuple of length %d (2-tuple expected): %s",
                    len(data), data)

    try:
      key = str(data[0])
      value = str(data[1])
    except TypeError:
      logging.error("Expecting a tuple, but got %s: %s",
                    data.__class__.__name__, data)

    file_index = key.__hash__() % len(self._filehandles)

    # Work-around: Since we don't have access to the context in the to_json()
    # function, but we need to flush each pool before we serialize the
    # filehandle, we rely on a member variable instead of using context for
    # pool management.
    pool = self._pools[file_index]
    if pool is None:
      filehandle = self._filehandles[file_index]
      pool = output_writers.GCSRecordsPool(filehandle=filehandle, ctx=ctx)
      self._pools[file_index] = pool

    proto = kv_pb.KeyValue()
    proto.set_key(key)
    proto.set_value(value)
    pool.append(proto.Encode())


class _ShardOutputs(pipeline_base.PipelineBase):
  """Shards the ouputs.

  Takes a flat list of filenames, returns a list of lists, each with
  one member each.
  """

  def run(self, filenames):
    result = []
    for name in filenames:
      result.append([name])
    return result


# pylint: disable=unused-argument
def _merge_map(key, values, partial):
  """A map function used in merge phase.

  Stores (key, values) into KeyValues proto and yields its serialization.

  Args:
    key: values key.
    values: values themselves.
    partial: True if more values for this key will follow. False otherwise.

  Yields:
    The proto.
  """
  proto = kv_pb.KeyValues()
  proto.set_key(key)
  proto.value_list().extend(values)
  yield proto.Encode()


class _MergePipeline(pipeline_base.PipelineBase):
  """Pipeline to merge sorted chunks.

  This pipeline merges together individually sorted chunks of each shard.

  Args:
    filenames: list of lists of filenames. Each list will correspond to a single
      shard. Each file in the list should have keys sorted and should contain
      records with KeyValue serialized entity.

  Yields:
    The list of filenames, where each filename is fully merged and will contain
    records with KeyValues serialized entity.
  """

  # Maximum number of values to produce in a single KeyValues proto.
  _MAX_VALUES_COUNT = 100000  # Combiners usually good for 5 orders of magnitude
  # Maximum size of values to produce in a single KeyValues proto.
  _MAX_VALUES_SIZE = 1000000

  def run(self, job_name, bucket_name, filenames):
    yield mapper_pipeline.MapperPipeline(
        job_name + "-shuffle-merge",
        __name__ + "._merge_map",
        __name__ + "._MergingReader",
        output_writer_spec=
        output_writers.__name__ + "._GoogleCloudStorageRecordOutputWriter",
        params={
            _MergingReader.FILES_PARAM: filenames,
            _MergingReader.MAX_VALUES_COUNT_PARAM: self._MAX_VALUES_COUNT,
            _MergingReader.MAX_VALUES_SIZE_PARAM: self._MAX_VALUES_SIZE,
            "output_writer": {
                "bucket_name": bucket_name,
            },
        },
        shards=len(filenames))


def _hashing_map(binary_record):
  """A map function used in hash phase.

  Reads KeyValue from binary record.

  Args:
    binary_record: The binary record.

  Yields:
    The (key, value).
  """
  proto = kv_pb.KeyValue()
  proto.ParseFromString(binary_record)
  yield (proto.key(), proto.value())


class _HashPipeline(pipeline_base.PipelineBase):
  """A pipeline to read mapper output and hash by key.

  Args:
    job_name: root mapreduce job name.
    bucket_name: The name of the Google Cloud Storage bucket.
    filenames: filenames of mapper output. Should be of records format
      with serialized KeyValue proto.
    shards: Optional. Number of output shards to generate. Defaults
      to the number of input files.

  Yields:
    The list of filenames. Each file is of records formad with serialized
    KeyValue proto. For each proto its output file is decided based on key
    hash. Thus all equal keys would end up in the same file.
  """

  def run(self, job_name, bucket_name, filenames, shards=None):
    filenames_only = (
        util.strip_prefix_from_items("/%s/" % bucket_name, filenames))
    if shards is None:
      shards = len(filenames)
    yield mapper_pipeline.MapperPipeline(
        job_name + "-shuffle-hash",
        __name__ + "._hashing_map",
        input_readers.__name__ + "._GoogleCloudStorageRecordInputReader",
        output_writer_spec=__name__ + "._HashingGCSOutputWriter",
        params={
            "input_reader": {
                "bucket_name": bucket_name,
                "objects": filenames_only,
            },
            "output_writer": {
                "bucket_name": bucket_name,
            },
        },
        shards=shards)


class ShufflePipeline(pipeline_base.PipelineBase):
  """A pipeline to shuffle multiple key-value files.

  Args:
    job_name: The descriptive name of the overall job.
    mapper_params: parameters to use for mapper phase.
    filenames: list of file names to sort. Files have to be of records format
      defined by Files API and contain serialized kv_pb.KeyValue
      protocol messages. The filenames may or may not contain the
      GCS bucket name in their path.
    shards: Optional. Number of output shards to generate. Defaults
      to the number of input files.

  Returns:
    default: a list of filenames as string. Resulting files contain
      serialized kv_pb.KeyValues protocol messages with
      all values collated to a single key. When there is no output,
      an empty list from shuffle service or a list of empty files from
      in memory shuffler.
  """

  def run(self, job_name, mapper_params, filenames, shards=None):
    bucket_name = mapper_params["bucket_name"]
    hashed_files = yield _HashPipeline(job_name, bucket_name,
                                       filenames, shards=shards)
    sorted_files = yield _SortChunksPipeline(job_name, bucket_name,
                                             hashed_files)
    temp_files = [hashed_files, sorted_files]

    merged_files = yield _MergePipeline(job_name, bucket_name, sorted_files)

    with pipeline.After(merged_files):
      all_temp_files = yield pipeline_common.Extend(*temp_files)
      yield _GCSCleanupPipeline(all_temp_files)

    yield pipeline_common.Return(merged_files)


class _GCSCleanupPipeline(pipeline_base.PipelineBase):
  """A pipeline to do a cleanup for mapreduce jobs that use GCS.

  Args:
    filename_or_list: list of files or file lists to delete.
  """

  # The minimum number of retries for GCS to delete the file.
  _MIN_RETRIES = 5
  # The maximum number of retries for GCS to delete the file.
  _MAX_RETRIES = 10

  def delete_file_or_list(self, filename_or_list):
    if isinstance(filename_or_list, list):
      for filename in filename_or_list:
        self.delete_file_or_list(filename)
    else:
      filename = filename_or_list
      retry_params = cloudstorage.RetryParams(min_retries=self._MIN_RETRIES,
                                              max_retries=self._MAX_RETRIES)
      # pylint: disable=bare-except
      try:
        cloudstorage.delete(filename, retry_params)
      except:
        pass

  def run(self, temp_files):
    self.delete_file_or_list(temp_files)
