#!/usr/bin/env python
"""Per job config for map jobs."""
from mapreduce import hooks
from mapreduce import input_readers
from mapreduce import output_writers
from mapreduce import parameters
from mapreduce import util
from mapreduce.api.map_job import input_reader
from mapreduce.api.map_job import mapper as mapper_module


# pylint: disable=protected-access
# pylint: disable=invalid-name

_Option = parameters._Option
# Current API Version. The API version used to start a map_job will be
# saved to its job states. The framework can use this information to
# handle adding and removing stuffs better.
_API_VERSION = 1


class JobConfig(parameters._Config):
  """Configurations for a map job.

  Names started with _ are reserved for internal use.

  To create an instance:
  all option names can be used as keys to __init__.
  If an option is required, the key must be provided.
  If an option isn't required and no value is given, the default value
  will be used.
  """
  # Job name in str. UI purpose only.
  job_name = _Option(basestring, required=True)

  # Job ID. Must be unique across the app.
  # This is used to store state in datastore.
  # One will be automatically generated if not given.
  job_id = _Option(basestring, default_factory=util._get_descending_key)

  # Reference to your mapper.
  mapper = _Option(mapper_module.Mapper, required=True)

  # The class of input reader to use.
  input_reader_cls = _Option(input_reader.InputReader, required=True)

  # TODO(user): Create a config object for readers instead of a dict.
  # Parameters for input reader. Varies by input reader class.
  input_reader_params = _Option(dict, default_factory=lambda: {})

  # TODO(user): restrict output writer to new API.
  # The class of output writer to use.
  output_writer_cls = _Option(output_writers.OutputWriter,
                              can_be_none=True)

  # Parameters for output writers. Varies by input reader class.
  output_writer_params = _Option(dict, default_factory=lambda: {})

  # Number of map shards.
  # This number is only a hint to the map_job framework. This will be
  # updated to the actual number of running shards after the job starts.
  shard_count = _Option(int,
                        default_factory=lambda: parameters.config.SHARD_COUNT)

  # Additional parameters user can supply and read from their mapper.
  user_params = _Option(dict, default_factory=lambda: {})

  # The queue where all map tasks should run on.
  queue_name = _Option(
      basestring, default_factory=lambda: parameters.config.QUEUE_NAME)

  # max attempts to run and retry a shard.
  shard_max_attempts = _Option(
      int, default_factory=lambda: parameters.config.SHARD_MAX_ATTEMPTS)

  # The URL to GET after the job finish, regardless of success.
  # The map_job_id will be provided as a query string key.
  done_callback_url = _Option(basestring, can_be_none=True)

  # Force datastore writes.
  _force_writes = _Option(bool, default_factory=lambda: False)

  _base_path = _Option(basestring,
                       default_factory=lambda: parameters.config.BASE_PATH)

  _task_max_attempts = _Option(
      int, default_factory=lambda: parameters.config.TASK_MAX_ATTEMPTS)

  _task_max_data_processing_attempts = _Option(
      int, default_factory=(
          lambda: parameters.config.TASK_MAX_DATA_PROCESSING_ATTEMPTS))

  _hooks_cls = _Option(hooks.Hooks, can_be_none=True)

  _app = _Option(basestring, can_be_none=True)

  _api_version = _Option(int, default_factory=lambda: _API_VERSION)

  # The following methods are to convert Config to supply for older APIs.

  def _get_mapper_params(self):
    """Converts self to model.MapperSpec.params."""
    reader_params = self.input_reader_cls.params_to_json(
        self.input_reader_params)
    # TODO(user): Do the same for writer params.
    return {"input_reader": reader_params,
            "output_writer": self.output_writer_params}

  def _get_mapper_spec(self):
    """Converts self to model.MapperSpec."""
    # pylint: disable=g-import-not-at-top
    from mapreduce import model

    return model.MapperSpec(
        handler_spec=util._obj_to_path(self.mapper),
        input_reader_spec=util._obj_to_path(self.input_reader_cls),
        params=self._get_mapper_params(),
        shard_count=self.shard_count,
        output_writer_spec=util._obj_to_path(self.output_writer_cls))

  def _get_mr_params(self):
    """Converts self to model.MapreduceSpec.params."""
    return {"force_writes": self._force_writes,
            "done_callback": self.done_callback_url,
            "user_params": self.user_params,
            "shard_max_attempts": self.shard_max_attempts,
            "task_max_attempts": self._task_max_attempts,
            "task_max_data_processing_attempts":
                self._task_max_data_processing_attempts,
            "queue_name": self.queue_name,
            "base_path": self._base_path,
            "app_id": self._app,
            "api_version": self._api_version}

  # TODO(user): Ideally we should replace all the *_spec and *_params
  # in model.py with JobConfig. This not only cleans up codebase, but may
  # also be necessary for launching input_reader/output_writer API. We don't
  # want to surface the numerous *_spec and *_params objects in our public API.
  # The cleanup has to be done over several releases to not to break runtime.
  @classmethod
  def _get_default_mr_params(cls):
    """Gets default values for old API."""
    cfg = cls(_lenient=True)
    mr_params = cfg._get_mr_params()
    mr_params["api_version"] = 0
    return mr_params

  @classmethod
  def _to_map_job_config(cls,
                         mr_spec,
                         # TODO(user): Remove this parameter after it can be
                         # read from mr_spec.
                         queue_name):
    """Converts model.MapreduceSpec back to JobConfig.

    This method allows our internal methods to use JobConfig directly.
    This method also allows us to expose JobConfig as an API during execution,
    despite that it is not saved into datastore.

    Args:
      mr_spec: model.MapreduceSpec.
      queue_name: queue name.

    Returns:
      The JobConfig object for this job.
    """
    mapper_spec = mr_spec.mapper
    # 0 means all the old APIs before api_version is introduced.
    api_version = mr_spec.params.get("api_version", 0)
    old_api = api_version == 0

    # Deserialize params from json if input_reader/output_writer are new API.
    input_reader_cls = mapper_spec.input_reader_class()
    input_reader_params = input_readers._get_params(mapper_spec)
    if issubclass(input_reader_cls, input_reader.InputReader):
      input_reader_params = input_reader_cls.params_from_json(
          input_reader_params)

    output_writer_cls = mapper_spec.output_writer_class()
    output_writer_params = output_writers._get_params(mapper_spec)
    # TODO(user): Call json (de)serialization for writer.
    # if (output_writer_cls and
    #     issubclass(output_writer_cls, output_writer.OutputWriter)):
    #   output_writer_params = output_writer_cls.params_from_json(
    #       output_writer_params)

    # We can not always convert MapreduceSpec generated by older API
    # to JobConfig. Thus, mr framework should use/expose the returned JobConfig
    # object with caution when a job is started with an old API.
    # In this case, this method only tries not to blow up and assemble a
    # JobConfig object as accurate as possible.
    return cls(_lenient=old_api,
               job_name=mr_spec.name,
               job_id=mr_spec.mapreduce_id,
               # handler_spec from older API may not have map_job.Mapper type.
               mapper=util.for_name(mapper_spec.handler_spec),
               input_reader_cls=input_reader_cls,
               input_reader_params=input_reader_params,
               output_writer_cls=output_writer_cls,
               output_writer_params=output_writer_params,
               shard_count=mapper_spec.shard_count,
               queue_name=queue_name,
               user_params=mr_spec.params.get("user_params"),
               shard_max_attempts=mr_spec.params.get("shard_max_attempts"),
               done_callback_url=mr_spec.params.get("done_callback"),
               _force_writes=mr_spec.params.get("force_writes"),
               _base_path=mr_spec.params["base_path"],
               _task_max_attempts=mr_spec.params.get("task_max_attempts"),
               _task_max_data_processing_attempts=(
                   mr_spec.params.get("task_max_data_processing_attempts")),
               _hooks_cls=util.for_name(mr_spec.hooks_class_name),
               _app=mr_spec.params.get("app_id"),
               _api_version=api_version)
