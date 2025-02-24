#!/usr/bin/env python
# Copyright 2011 Google Inc.
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

"""Map Reduce framework errors."""



__all__ = [
    "BadCombinerOutputError",
    "BadParamsError",
    "BadReaderParamsError",
    "BadWriterParamsError",
    "BadYamlError",
    "Error",
    "FailJobError",
    "MissingYamlError",
    "MultipleDocumentsInMrYaml",
    "NotEnoughArgumentsError",
    "RetrySliceError",
    "ShuffleServiceError",
    "InvalidRecordError",
    ]


class Error(Exception):
  """Base-class for exceptions in this module."""


class BadYamlError(Error):
  """Raised when the mapreduce.yaml file is invalid."""


class MissingYamlError(BadYamlError):
  """Raised when the mapreduce.yaml file could not be found."""


class MultipleDocumentsInMrYaml(BadYamlError):
  """There's more than one document in mapreduce.yaml file."""


class BadParamsError(Error):
  """One of the mapper parameters is invalid."""


class BadReaderParamsError(BadParamsError):
  """The input parameters to a reader were invalid."""


class BadWriterParamsError(BadParamsError):
  """The input parameters to a reader were invalid."""


class FailJobError(Error):
  """The job will be failed if this exception is thrown anywhere."""


class NotEnoughArgumentsError(Error):
  """Required argument is missing."""


class BadCombinerOutputError(Error):
  """Combiner outputs data instead of yielding it."""


class ShuffleServiceError(Error):
  """Error doing shuffle through shuffle service."""


class RetrySliceError(Error):
  """The slice will be retried up to some maximum number of times.

  The job will be failed if the slice can't progress before maximum
  number of retries.
  """


class InvalidRecordError(Error):
  """Raised when invalid record encountered."""
