#!/usr/bin/env python
#
# Copyright 2010 Google Inc.
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

"""Status page handler for mapreduce framework."""

__author__ = ("aizatsky@google.com (Mike Aizatsky)",
              "bslatkin@google.com (Brett Slatkin)")

import os
import pkgutil
import time
import zipfile

from google.appengine.api import validation
from google.appengine.api import yaml_builder
from google.appengine.api import yaml_errors
from google.appengine.api import yaml_listener
from google.appengine.api import yaml_object
from google.appengine.ext import db
from google.appengine.ext import webapp
from mapreduce import base_handler
from mapreduce import errors
from mapreduce import model


# TODO(user): a list of features we'd like to have in status page:
# - show sparklet of entities/sec on index page
# - shard bar chart should color finished shards differently

# mapreduce.yaml file names
MR_YAML_NAMES = ["mapreduce.yaml", "mapreduce.yml"]


class BadStatusParameterError(Exception):
  """A parameter passed to a status handler was invalid."""


class UserParam(validation.Validated):
  """A user-supplied parameter to a mapreduce job."""

  ATTRIBUTES = {
      "name":  r"[a-zA-Z0-9_\.]+",
      "default": validation.Optional(r".*"),
      "value": validation.Optional(r".*"),
  }


class MapperInfo(validation.Validated):
  """Configuration parameters for the mapper part of the job."""

  ATTRIBUTES = {
    "handler": r".+",
    "input_reader": r".+",
    "output_writer": validation.Optional(r".+"),
    "params": validation.Optional(validation.Repeated(UserParam)),
    "params_validator": validation.Optional(r".+"),
  }


class MapreduceInfo(validation.Validated):
  """Mapreduce description in mapreduce.yaml."""

  ATTRIBUTES = {
      "name": r".+",
      "mapper": MapperInfo,
      "params": validation.Optional(validation.Repeated(UserParam)),
      "params_validator": validation.Optional(r".+"),
  }


class MapReduceYaml(validation.Validated):
  """Root class for mapreduce.yaml.

  File format:

  mapreduce:
  - name: <mapreduce_name>
    mapper:
      - input_reader: google.appengine.ext.mapreduce.DatastoreInputReader
      - handler: path_to_my.MapperFunction
      - params:
        - name: foo
          default: bar
        - name: blah
          default: stuff
      - params_validator: path_to_my.ValidatorFunction

  Where
    mapreduce_name: The name of the mapreduce. Used for UI purposes.
    mapper_handler_spec: Full <module_name>.<function_name/class_name> of
      mapper handler. See MapreduceSpec class documentation for full handler
      specification.
    input_reader: Full <module_name>.<function_name/class_name> of the
      InputReader sub-class to use for the mapper job.
    params: A list of optional parameter names and optional default values
      that may be supplied or overridden by the user running the job.
    params_validator is full <module_name>.<function_name/class_name> of
      a callable to validate the mapper_params after they are input by the
      user running the job.
  """

  ATTRIBUTES = {
      "mapreduce": validation.Optional(validation.Repeated(MapreduceInfo))
  }

  @staticmethod
  def to_dict(mapreduce_yaml):
    """Converts a MapReduceYaml file into a JSON-encodable dictionary.

    For use in user-visible UI and internal methods for interfacing with
    user code (like param validation). as a list

    Args:
      mapreduce_yaml: The Pyton representation of the mapreduce.yaml document.

    Returns:
      A list of configuration dictionaries.
    """
    all_configs = []
    for config in mapreduce_yaml.mapreduce:
      out = {
          "name": config.name,
          "mapper_input_reader": config.mapper.input_reader,
          "mapper_handler": config.mapper.handler,
      }
      if config.mapper.params_validator:
        out["mapper_params_validator"] = config.mapper.params_validator
      if config.mapper.params:
        param_defaults = {}
        for param in config.mapper.params:
          param_defaults[param.name] = param.default or param.value
        out["mapper_params"] = param_defaults
      if config.params:
        param_defaults = {}
        for param in config.params:
          param_defaults[param.name] = param.default or param.value
        out["params"] = param_defaults
      if config.mapper.output_writer:
        out["mapper_output_writer"] = config.mapper.output_writer
      all_configs.append(out)

    return all_configs


# N.B. Sadly, we currently don't have and ability to determine
# application root dir at run time. We need to walk up the directory structure
# to find it.
def find_mapreduce_yaml(status_file=__file__):
  """Traverse directory trees to find mapreduce.yaml file.

  Begins with the location of status.py and then moves on to check the working
  directory.

  Args:
    status_file: location of status.py, overridable for testing purposes.

  Returns:
    the path of mapreduce.yaml file or None if not found.
  """
  checked = set()
  yaml = _find_mapreduce_yaml(os.path.dirname(status_file), checked)
  if not yaml:
    yaml = _find_mapreduce_yaml(os.getcwd(), checked)
  return yaml


def _find_mapreduce_yaml(start, checked):
  """Traverse the directory tree identified by start until a directory already
  in checked is encountered or the path of mapreduce.yaml is found.

  Checked is present both to make loop termination easy to reason about and so
  that the same directories do not get rechecked.

  Args:
    start: the path to start in and work upward from
    checked: the set of already examined directories

  Returns:
    the path of mapreduce.yaml file or None if not found.
  """
  dir = start
  while dir not in checked:
    checked.add(dir)
    for mr_yaml_name in MR_YAML_NAMES:
      yaml_path = os.path.join(dir, mr_yaml_name)
      if os.path.exists(yaml_path):
        return yaml_path
    dir = os.path.dirname(dir)
  return None


def parse_mapreduce_yaml(contents):
  """Parses mapreduce.yaml file contents.

  Args:
    contents: mapreduce.yaml file contents.

  Returns:
    MapReduceYaml object with all the data from original file.

  Raises:
    errors.BadYamlError: when contents is not a valid mapreduce.yaml file.
  """
  try:
    builder = yaml_object.ObjectBuilder(MapReduceYaml)
    handler = yaml_builder.BuilderHandler(builder)
    listener = yaml_listener.EventListener(handler)
    listener.Parse(contents)

    mr_info = handler.GetResults()
  except (ValueError, yaml_errors.EventError), e:
    raise errors.BadYamlError(e)

  if len(mr_info) < 1:
    raise errors.BadYamlError("No configs found in mapreduce.yaml")
  if len(mr_info) > 1:
    raise errors.MultipleDocumentsInMrYaml("Found %d YAML documents" %
                                           len(mr_info))

  jobs = mr_info[0]
  job_names = set(j.name for j in jobs.mapreduce)
  if len(jobs.mapreduce) != len(job_names):
    raise errors.BadYamlError(
        "Overlapping mapreduce names; names must be unique")

  return jobs


def get_mapreduce_yaml(parse=parse_mapreduce_yaml):
  """Locates mapreduce.yaml, loads and parses its info.

  Args:
    parse: Used for testing.

  Returns:
    MapReduceYaml object.

  Raises:
    errors.BadYamlError: when contents is not a valid mapreduce.yaml file or the
    file is missing.
  """
  mr_yaml_path = find_mapreduce_yaml()
  if not mr_yaml_path:
    raise errors.MissingYamlError()
  mr_yaml_file = open(mr_yaml_path)
  try:
    return parse(mr_yaml_file.read())
  finally:
    mr_yaml_file.close()


class ResourceHandler(webapp.RequestHandler):
  """Handler for static resources."""

  _RESOURCE_MAP = {
      "status": ("overview.html", "text/html"),
      "detail": ("detail.html", "text/html"),
      "base.css": ("base.css", "text/css"),
      "jquery.js": ("jquery-1.6.1.min.js", "text/javascript"),
      "jquery-json.js": ("jquery.json-2.2.min.js", "text/javascript"),
      "jquery-url.js": ("jquery.url.js", "text/javascript"),
      "status.js": ("status.js", "text/javascript"),
  }

  def get(self, relative):
    if relative not in self._RESOURCE_MAP:
      self.response.set_status(404)
      self.response.out.write("Resource not found.")
      return

    real_path, content_type = self._RESOURCE_MAP[relative]
    path = os.path.join(os.path.dirname(__file__), "static", real_path)

    # It's possible we're inside a zipfile (zipimport).  If so, path
    # will include 'something.zip'.
    if ('.zip' + os.sep) in path:
      (zip_file, zip_path) = os.path.relpath(path).split('.zip' + os.sep, 1)
      content = zipfile.ZipFile(zip_file + '.zip').read(zip_path)
    else:
      try:
        data = pkgutil.get_data(__name__, "static/" + real_path)
      except AttributeError:  # Python < 2.6.
        data = None
      content = data or open(path, 'rb').read()

    self.response.headers["Cache-Control"] = "public; max-age=300"
    self.response.headers["Content-Type"] = content_type
    self.response.out.write(content)


class ListConfigsHandler(base_handler.GetJsonHandler):
  """Lists mapreduce configs as JSON for users to start jobs."""

  def handle(self):
    self.json_response["configs"] = MapReduceYaml.to_dict(get_mapreduce_yaml())


class ListJobsHandler(base_handler.GetJsonHandler):
  """Lists running and completed mapreduce jobs for an overview as JSON."""

  def handle(self):
    cursor = self.request.get("cursor")
    count = int(self.request.get("count", "50"))

    query = model.MapreduceState.all()
    if cursor:
      query.filter("__key__ >=", db.Key(cursor))
    query.order("__key__")

    jobs_list = query.fetch(count + 1)
    if len(jobs_list) == (count + 1):
      self.json_response["cursor"] = str(jobs_list[-1].key())
      jobs_list = jobs_list[:-1]

    all_jobs = []
    for job in jobs_list:
      out = {
          # Data shared between overview and detail pages.
          "name": job.mapreduce_spec.name,
          "mapreduce_id": job.mapreduce_spec.mapreduce_id,
          "active": job.active,
          "start_timestamp_ms":
              int(time.mktime(job.start_time.utctimetuple()) * 1000),
          "updated_timestamp_ms":
              int(time.mktime(job.last_poll_time.utctimetuple()) * 1000),

          # Specific to overview page.
          "chart_url": job.sparkline_url,
          "chart_width": job.chart_width,
          "active_shards": job.active_shards,
          "shards": job.mapreduce_spec.mapper.shard_count,
      }
      if job.result_status:
        out["result_status"] = job.result_status
      all_jobs.append(out)

    self.json_response["jobs"] = all_jobs


class GetJobDetailHandler(base_handler.GetJsonHandler):
  """Retrieves the details of a mapreduce job as JSON."""

  def handle(self):
    mapreduce_id = self.request.get("mapreduce_id")
    if not mapreduce_id:
      raise BadStatusParameterError("'mapreduce_id' was invalid")
    job = model.MapreduceState.get_by_key_name(mapreduce_id)
    if job is None:
      raise KeyError("Could not find job with ID %r" % mapreduce_id)

    self.json_response.update(job.mapreduce_spec.to_json())
    self.json_response.update(job.counters_map.to_json())
    self.json_response.update({
        # Shared with overview page.
        "active": job.active,
        "start_timestamp_ms":
            int(time.mktime(job.start_time.utctimetuple()) * 1000),
        "updated_timestamp_ms":
            int(time.mktime(job.last_poll_time.utctimetuple()) * 1000),

        # Specific to detail page.
        "chart_url": job.chart_url,
        "chart_width": job.chart_width,
    })
    self.json_response["result_status"] = job.result_status

    all_shards = []
    for shard in model.ShardState.find_all_by_mapreduce_state(job):
      out = {
          "active": shard.active,
          "result_status": shard.result_status,
          "shard_number": shard.shard_number,
          "shard_id": shard.shard_id,
          "updated_timestamp_ms":
              int(time.mktime(shard.update_time.utctimetuple()) * 1000),
          "shard_description": shard.shard_description,
          "last_work_item": shard.last_work_item,
      }
      out.update(shard.counters_map.to_json())
      all_shards.append(out)
    all_shards.sort(key=lambda x: x["shard_number"])
    self.json_response["shards"] = all_shards
