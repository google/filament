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

"""Google App Engine Pipeline API for complex, asynchronous workflows."""

__all__ = [
    # Public API.
    'Error', 'PipelineSetupError', 'PipelineExistsError',
    'PipelineRuntimeError', 'SlotNotFilledError', 'SlotNotDeclaredError',
    'UnexpectedPipelineError', 'PipelineStatusError', 'Slot', 'Pipeline',
    'PipelineFuture', 'After', 'InOrder', 'Retry', 'Abort', 'get_status_tree',
    'get_pipeline_names', 'get_root_list', 'create_handlers_map',
    'set_enforce_auth',
]

import datetime
import hashlib
import itertools
import logging
import os
import posixpath
import pprint
import re
import sys
import threading
import time
import urllib
import uuid

from google.appengine.api import mail
from google.appengine.api import app_identity
from google.appengine.api import users
from google.appengine.api import taskqueue
from google.appengine.ext import blobstore
from google.appengine.ext import db
from google.appengine.ext import webapp

# pylint: disable=g-import-not-at-top
# TODO(user): Cleanup imports if/when cloudstorage becomes part of runtime.
try:
  # Check if the full cloudstorage package exists. The stub part is in runtime.
  import cloudstorage
  if hasattr(cloudstorage, "_STUB"):
    cloudstorage = None
except ImportError:
  pass  # CloudStorage library not available

try:
  import json
except ImportError:
  import simplejson as json

# Relative imports
import models
import status_ui
import util as mr_util

# pylint: disable=g-bad-name
# pylint: disable=protected-access

# For convenience
_BarrierIndex = models._BarrierIndex
_BarrierRecord = models._BarrierRecord
_PipelineRecord = models._PipelineRecord
_SlotRecord = models._SlotRecord
_StatusRecord = models._StatusRecord


# Overall TODOs:
# - Add a human readable name for start()

# Potential TODOs:
# - Add support for ANY N barriers.
# - Allow Pipelines to declare they are "short" and optimize the evaluate()
#   function to run as many of them in quick succession.
# - Add support in all Pipelines for hold/release where up-stream
#   barriers will fire but do nothing because the Pipeline is not ready.

################################################################################


class Error(Exception):
  """Base class for exceptions in this module."""


class PipelineSetupError(Error):
  """Base class for exceptions that happen before Pipeline execution."""


class PipelineExistsError(PipelineSetupError):
  """A new Pipeline with an assigned idempotence_key cannot be overwritten."""


class PipelineRuntimeError(Error):
  """Base class for exceptions that happen during Pipeline execution."""


class SlotNotFilledError(PipelineRuntimeError):
  """A slot that should have been filled already was not yet filled."""


class SlotNotDeclaredError(PipelineRuntimeError):
  """A slot that was filled or passed along was not previously declared."""


class UnexpectedPipelineError(PipelineRuntimeError):
  """An assertion failed, potentially leaving the pipeline unable to proceed."""


class PipelineUserError(Error):
  """Exceptions raised indirectly by developers to cause certain behaviors."""


class Retry(PipelineUserError):
  """The currently running pipeline should be retried at a later time."""


class Abort(PipelineUserError):
  """The currently running pipeline should be aborted up to the root."""


class PipelineStatusError(Error):
  """Exceptions raised when trying to collect pipeline status."""


class _CallbackTaskError(Error):
  """A callback task was unable to execute properly for some reason."""


################################################################################

_MAX_BARRIERS_TO_NOTIFY = 10

_MAX_ABORTS_TO_BEGIN = 10

_TEST_MODE = False

_TEST_ROOT_PIPELINE_KEY = None

_DEFAULT_BACKOFF_SECONDS = 15

_DEFAULT_BACKOFF_FACTOR = 2

_DEFAULT_MAX_ATTEMPTS = 3

_RETRY_WIGGLE_TIMEDELTA = datetime.timedelta(seconds=20)

_DEBUG = False

_MAX_JSON_SIZE = 900000

_ENFORCE_AUTH = True

_MAX_CALLBACK_TASK_RETRIES = 5

################################################################################


class Slot(object):
  """An output that is filled by a Pipeline as it executes."""

  def __init__(self, name=None, slot_key=None, strict=False):
    """Initializer.

    Args:
      name: The name of this slot.
      slot_key: The db.Key for this slot's _SlotRecord if it's already been
        allocated by an up-stream pipeline.
      strict: If this Slot was created as an output of a strictly defined
        pipeline.
    """
    if name is None:
      raise UnexpectedPipelineError('Slot with key "%s" missing a name.' %
                                    slot_key)
    if slot_key is None:
      slot_key = db.Key.from_path(_SlotRecord.kind(), uuid.uuid4().hex)
      self._exists = _TEST_MODE
    else:
      self._exists = True
    self._touched = False
    self._strict = strict
    self.name = name
    self.key = slot_key
    self.filled = False
    self._filler_pipeline_key = None
    self._fill_datetime = None
    self._value = None

  @property
  def value(self):
    """Returns the current value of this slot.

    Returns:
      The value of the slot (a serializable Python type).

    Raises:
      SlotNotFilledError if the value hasn't been filled yet.
    """
    if not self.filled:
      raise SlotNotFilledError('Slot with name "%s", key "%s" not yet filled.'
                               % (self.name, self.key))
    return self._value

  @property
  def filler(self):
    """Returns the pipeline ID that filled this slot's value.

    Returns:
      A string that is the pipeline ID.

    Raises:
      SlotNotFilledError if the value hasn't been filled yet.
    """
    if not self.filled:
      raise SlotNotFilledError('Slot with name "%s", key "%s" not yet filled.'
                               % (self.name, self.key))
    return self._filler_pipeline_key.name()

  @property
  def fill_datetime(self):
    """Returns when the slot was filled.

    Returns:
      A datetime.datetime.

    Raises:
      SlotNotFilledError if the value hasn't been filled yet.
    """
    if not self.filled:
      raise SlotNotFilledError('Slot with name "%s", key "%s" not yet filled.'
                               % (self.name, self.key))
    return self._fill_datetime

  def _set_value(self, slot_record):
    """Sets the value of this slot based on its corresponding _SlotRecord.

    Does nothing if the slot has not yet been filled.

    Args:
      slot_record: The _SlotRecord containing this Slot's value.
    """
    if slot_record.status == _SlotRecord.FILLED:
      self.filled = True
      self._filler_pipeline_key = _SlotRecord.filler.get_value_for_datastore(
          slot_record)
      self._fill_datetime = slot_record.fill_time
      self._value = slot_record.value

  def _set_value_test(self, filler_pipeline_key, value):
    """Sets the value of this slot for use in testing.

    Args:
      filler_pipeline_key: The db.Key of the _PipelineRecord that filled
        this slot.
      value: The serializable value set for this slot.
    """
    self.filled = True
    self._filler_pipeline_key = filler_pipeline_key
    self._fill_datetime = datetime.datetime.utcnow()
    # Convert to JSON and back again, to simulate the behavior of production.
    self._value = json.loads(json.dumps(
        value, cls=mr_util.JsonEncoder), cls=mr_util.JsonDecoder)

  def __repr__(self):
    """Returns a string representation of this slot."""
    if self.filled:
      return repr(self._value)
    else:
      return 'Slot(name="%s", slot_key="%s")' % (self.name, self.key)


class PipelineFuture(object):
  """A future for accessing the outputs of a Pipeline."""

  # NOTE: Do not, ever, add a names() method to this class. Callers cannot do
  # introspection on their context of being called. Even though the runtime
  # environment of the Pipeline can allow for that to happen, such behavior
  # would prevent synchronous simulation and verification, whic is an
  # unacceptable tradeoff.

  def __init__(self, output_names, force_strict=False):
    """Initializer.

    Args:
      output_names: The list of require output names that will be strictly
        enforced by this class.
      force_strict: If True, force this future to be in strict mode.
    """
    self._after_all_pipelines = set()
    self._output_dict = {
      'default': Slot(name='default'),
    }

    self._strict = len(output_names) > 0 or force_strict
    if self._strict:
      for name in output_names:
        if name in self._output_dict:
          raise UnexpectedPipelineError('Output name reserved: "%s"' % name)
        self._output_dict[name] = Slot(name=name, strict=True)

  def _inherit_outputs(self,
                       pipeline_name,
                       already_defined,
                       resolve_outputs=False):
    """Inherits outputs from a calling Pipeline.

    Args:
      pipeline_name: The Pipeline class name (used for debugging).
      already_defined: Maps output name to stringified db.Key (of _SlotRecords)
        of any exiting output slots to be inherited by this future.
      resolve_outputs: When True, this method will dereference all output slots
        before returning back to the caller, making those output slots' values
        available.

    Raises:
      UnexpectedPipelineError when resolve_outputs is True and any of the output
      slots could not be retrived from the Datastore.
    """
    for name, slot_key in already_defined.iteritems():
      if not isinstance(slot_key, db.Key):
        slot_key = db.Key(slot_key)

      slot = self._output_dict.get(name)
      if slot is None:
        if self._strict:
          raise UnexpectedPipelineError(
              'Inherited output named "%s" must be filled but '
              'not declared for pipeline class "%s"' % (name, pipeline_name))
        else:
          self._output_dict[name] = Slot(name=name, slot_key=slot_key)
      else:
        slot.key = slot_key
        slot._exists = True

    if resolve_outputs:
      slot_key_dict = dict((s.key, s) for s in self._output_dict.itervalues())
      all_slots = db.get(slot_key_dict.keys())
      for slot, slot_record in zip(slot_key_dict.itervalues(), all_slots):
        if slot_record is None:
          raise UnexpectedPipelineError(
              'Inherited output named "%s" for pipeline class "%s" is '
              'missing its Slot in the datastore: "%s"' %
              (slot.name, pipeline_name, slot.key))
        slot = slot_key_dict[slot_record.key()]
        slot._set_value(slot_record)

  def __getattr__(self, name):
    """Provides an output Slot instance with the given name if allowed."""
    if name not in self._output_dict:
      if self._strict:
        raise SlotNotDeclaredError('Undeclared output with name "%s"' % name)
      self._output_dict[name] = Slot(name=name)
    slot = self._output_dict[name]
    return slot


class _PipelineMeta(type):
  """Meta-class for recording all Pipelines that have been defined."""

  # List of all Pipeline classes that have been seen.
  _all_classes = []

  def __new__(meta, name, bases, cls_dict):
    """Initializes the class path of a Pipeline and saves it."""
    cls = type.__new__(meta, name, bases, cls_dict)
    meta._all_classes.append(cls)
    return cls


class ClassProperty(object):
  """Descriptor that lets us have read-only class properties."""

  def __init__(self, method):
    self.method = method

  def __get__(self, cls, obj):
    return self.method(obj)


class Pipeline(object):
  """A Pipeline function-object that performs operations and has a life cycle.

  Class properties (to be overridden by sub-classes):
    async: When True, this Pipeline will execute asynchronously and fill the
      default output slot itself using the complete() method.
    output_names: List of named outputs (in addition to the default slot) that
      this Pipeline must output to (no more, no less).
    public_callbacks: If the callback URLs generated for this class should be
      accessible by all external requests regardless of login or task queue.
    admin_callbacks: If the callback URLs generated for this class should be
      accessible by the task queue ane externally by users logged in as admins.
    class_path: String identifier for this Pipeline, which is derived from
      its path in the global system modules dictionary.

  Modifiable instance properties:
    backoff_seconds: How many seconds to use as the constant factor in
      exponential backoff; may be changed by the user.
    backoff_factor: Base factor to use for exponential backoff. The formula
      followed is (backoff_seconds * backoff_factor^current_attempt).
    max_attempts: Maximum number of retry attempts to make before failing
      completely and aborting the entire pipeline up to the root.
    target: The application version to use for processing this Pipeline. This
      can be set to the name of a backend to direct Pipelines to run there.

  Instance properties:
    pipeline_id: The ID of this pipeline.
    root_pipeline_id: The ID of the root of this pipeline.
    queue_name: The queue this pipeline runs on or None if unknown.
    current_attempt: The current attempt being tried for this pipeline.
  """

  __metaclass__ = _PipelineMeta

  # To be set by sub-classes
  async = False
  output_names = []
  public_callbacks = False
  admin_callbacks = False

  # Internal only.
  _class_path = None  # Set for each class
  _send_mail = mail.send_mail_to_admins  # For testing

  # callback_xg_transaction: Determines whether callbacks are processed within
  # a single entity-group transaction (False), a cross-entity-group
  # transaction (True), or no transaction (None, default). It is generally
  # unsafe for a callback to modify pipeline state outside of a transaction, in
  # particular any pre-initialized state from the pipeline record, such as the
  # outputs. If a transaction is used, the callback method must operate within
  # the datastore's transaction time limits.
  # TODO(user): Make non-internal once other API calls are considered for
  # transaction support.
  _callback_xg_transaction = None

  def __init__(self, *args, **kwargs):
    """Initializer.

    Args:
      *args: The positional arguments for this function-object.
      **kwargs: The keyword arguments for this function-object.
    """
    self.args = args
    self.kwargs = kwargs
    self.outputs = None
    self.backoff_seconds = _DEFAULT_BACKOFF_SECONDS
    self.backoff_factor = _DEFAULT_BACKOFF_FACTOR
    self.max_attempts = _DEFAULT_MAX_ATTEMPTS
    self.target = None
    self.task_retry = False
    self._current_attempt = 0
    self._root_pipeline_key = None
    self._pipeline_key = None
    self._context = None
    self._result_status = None
    self._set_class_path()
    # Introspectively set the target so pipelines stick to the version it
    # started.
    self.target = mr_util._get_task_target()

    if _TEST_MODE:
      self._context = _PipelineContext('', 'default', '')
      self._root_pipeline_key = _TEST_ROOT_PIPELINE_KEY
      self._pipeline_key = db.Key.from_path(
          _PipelineRecord.kind(), uuid.uuid4().hex)
      self.outputs = PipelineFuture(self.output_names)
      self._context.evaluate_test(self)

  @property
  def pipeline_id(self):
    """Returns the ID of this Pipeline as a string or None if unknown."""
    if self._pipeline_key is None:
      return None
    return self._pipeline_key.name()

  @property
  def root_pipeline_id(self):
    """Returns root pipeline ID as a websafe string or None if unknown."""
    if self._root_pipeline_key is None:
      return None
    return self._root_pipeline_key.name()

  @property
  def is_root(self):
    """Returns True if this pipeline is a root pipeline, False otherwise."""
    return self._root_pipeline_key == self._pipeline_key

  @property
  def queue_name(self):
    """Returns the queue name this Pipeline runs on or None if unknown."""
    if self._context:
      return self._context.queue_name
    return None

  @property
  def base_path(self):
    """Returns the base path for Pipeline URL handlers or None if unknown."""
    if self._context:
      return self._context.base_path
    return None

  @property
  def has_finalized(self):
    """Returns True if this pipeline has completed and finalized."""
    return self._result_status == _PipelineRecord.DONE

  @property
  def was_aborted(self):
    """Returns True if this pipeline was aborted."""
    return self._result_status == _PipelineRecord.ABORTED

  @property
  def current_attempt(self):
    """Returns the current attempt at running this pipeline, starting at 1."""
    return self._current_attempt + 1

  @property
  def test_mode(self):
    """Returns True if the pipeline is running in test mode."""
    return _TEST_MODE

  @ClassProperty
  def class_path(cls):
    """Returns the unique string identifier for this Pipeline class.

    Refers to how to find the Pipeline in the global modules dictionary.
    """
    cls._set_class_path()
    return cls._class_path

  @classmethod
  def from_id(cls, pipeline_id, resolve_outputs=True, _pipeline_record=None):
    """Returns an instance corresponding to an existing Pipeline.

    The returned object will have the same properties a Pipeline does while
    it's running synchronously (e.g., like what it's first allocated), allowing
    callers to inspect caller arguments, outputs, fill slots, complete the
    pipeline, abort, retry, etc.

    Args:
      pipeline_id: The ID of this pipeline (a string).
      resolve_outputs: When True, dereference the outputs of this Pipeline
        so their values can be accessed by the caller.
      _pipeline_record: Internal-only. The _PipelineRecord instance to use
        to instantiate this instance instead of fetching it from
        the datastore.

    Returns:
      Pipeline sub-class instances or None if it could not be found.
    """
    pipeline_record = _pipeline_record

    # Support pipeline IDs and idempotence_keys that are not unicode.
    if not isinstance(pipeline_id, unicode):
      try:
        pipeline_id = pipeline_id.encode('utf-8')
      except UnicodeDecodeError:
        pipeline_id = hashlib.sha1(pipeline_id).hexdigest()

    pipeline_key = db.Key.from_path(_PipelineRecord.kind(), pipeline_id)

    if pipeline_record is None:
      pipeline_record = db.get(pipeline_key)
    if pipeline_record is None:
      return None

    try:
      pipeline_func_class = mr_util.for_name(pipeline_record.class_path)
    except ImportError, e:
      logging.warning('Tried to find Pipeline %s#%s, but class could '
                      'not be found. Using default Pipeline class instead.',
                      pipeline_record.class_path, pipeline_id)
      pipeline_func_class = cls

    params = pipeline_record.params
    arg_list, kwarg_dict = _dereference_args(
        pipeline_record.class_path, params['args'], params['kwargs'])
    outputs = PipelineFuture(pipeline_func_class.output_names)
    outputs._inherit_outputs(
        pipeline_record.class_path,
        params['output_slots'],
        resolve_outputs=resolve_outputs)

    stage = pipeline_func_class(*arg_list, **kwarg_dict)
    stage.backoff_seconds = params['backoff_seconds']
    stage.backoff_factor = params['backoff_factor']
    stage.max_attempts = params['max_attempts']
    stage.task_retry = params['task_retry']
    stage.target = params.get('target')  # May not be defined for old Pipelines
    stage._current_attempt = pipeline_record.current_attempt
    stage._set_values_internal(
        _PipelineContext('', params['queue_name'], params['base_path']),
        pipeline_key,
        _PipelineRecord.root_pipeline.get_value_for_datastore(pipeline_record),
        outputs,
        pipeline_record.status)
    return stage

  # Methods that can be invoked on a Pipeline instance by anyone with a
  # valid object (e.g., directly instantiated, retrieve via from_id).
  def start(self,
            idempotence_key='',
            queue_name='default',
            base_path='/_ah/pipeline',
            return_task=False,
            countdown=None,
            eta=None):
    """Starts a new instance of this pipeline.

    Args:
      idempotence_key: The ID to use for this Pipeline and throughout its
        asynchronous workflow to ensure the operations are idempotent. If
        empty a starting key will be automatically assigned.
      queue_name: What queue this Pipeline's workflow should execute on.
      base_path: The relative URL path to where the Pipeline API is
        mounted for access by the taskqueue API or external requests.
      return_task: When True, a task to start this pipeline will be returned
        instead of submitted, allowing the caller to start off this pipeline
        as part of a separate transaction (potentially leaving this newly
        allocated pipeline's datastore entities in place if that separate
        transaction fails for any reason).
      countdown: Time in seconds into the future that this Task should execute.
        Defaults to zero.
      eta: A datetime.datetime specifying the absolute time at which the task
        should be executed. Must not be specified if 'countdown' is specified.
        This may be timezone-aware or timezone-naive. If None, defaults to now.
        For pull tasks, no worker will be able to lease this task before the
        time indicated by eta.

    Returns:
      A taskqueue.Task instance if return_task was True. This task will *not*
      have a name, thus to ensure reliable execution of your pipeline you
      should add() this task as part of a separate Datastore transaction.

    Raises:
      PipelineExistsError if the pipeline with the given idempotence key exists.
      PipelineSetupError if the pipeline could not start for any other reason.
    """
    if not idempotence_key:
      idempotence_key = uuid.uuid4().hex
    elif not isinstance(idempotence_key, unicode):
      try:
        idempotence_key.encode('utf-8')
      except UnicodeDecodeError:
        idempotence_key = hashlib.sha1(idempotence_key).hexdigest()

    pipeline_key = db.Key.from_path(_PipelineRecord.kind(), idempotence_key)
    context = _PipelineContext('', queue_name, base_path)
    future = PipelineFuture(self.output_names, force_strict=True)
    try:
      self._set_values_internal(
          context, pipeline_key, pipeline_key, future, _PipelineRecord.WAITING)
      return context.start(
          self, return_task=return_task, countdown=countdown, eta=eta)
    except Error:
      # Pass through exceptions that originate in this module.
      raise
    except Exception, e:
      # Re-type any exceptions that were raised in dependent methods.
      raise PipelineSetupError('Error starting %s#%s: %s' % (
          self, idempotence_key, str(e)))

  def start_test(self, idempotence_key=None, base_path='', **kwargs):
    """Starts this pipeline in test fashion.

    Args:
      idempotence_key: Dummy idempotence_key to use for this root pipeline.
      base_path: Dummy base URL path to use for this root pipeline.
      kwargs: Ignored keyword arguments usually passed to start().
    """
    if not idempotence_key:
      idempotence_key = uuid.uuid4().hex
    pipeline_key = db.Key.from_path(_PipelineRecord.kind(), idempotence_key)
    context = _PipelineContext('', 'default', base_path)
    future = PipelineFuture(self.output_names, force_strict=True)
    self._set_values_internal(
        context, pipeline_key, pipeline_key, future, _PipelineRecord.WAITING)
    context.start_test(self)

  # Pipeline control methods.
  def retry(self, retry_message=''):
    """Forces a currently running asynchronous pipeline to retry.

    Note this may not be called by synchronous or generator pipelines. Those
    must instead raise the 'Retry' exception during execution.

    Args:
      retry_message: Optional message explaining why the retry happened.

    Returns:
      True if the Pipeline should be retried, False if it cannot be cancelled
      mid-flight for some reason.
    """
    if not self.async:
      raise UnexpectedPipelineError(
          'May only call retry() method for asynchronous pipelines.')
    if self.try_cancel():
      self._context.transition_retry(self._pipeline_key, retry_message)
      return True
    else:
      return False

  def abort(self, abort_message=''):
    """Mark the entire pipeline up to the root as aborted.

    Note this should only be called from *outside* the context of a running
    pipeline. Synchronous and generator pipelines should raise the 'Abort'
    exception to cause this behavior during execution.

    Args:
      abort_message: Optional message explaining why the abort happened.

    Returns:
      True if the abort signal was sent successfully; False if the pipeline
      could not be aborted for any reason.
    """
    # TODO: Use thread-local variable to enforce that this is not called
    # while a pipeline is executing in the current thread.
    if (self.async and self._root_pipeline_key == self._pipeline_key and
        not self.try_cancel()):
      # Handle the special case where the root pipeline is async and thus
      # cannot be aborted outright.
      return False
    else:
      return self._context.begin_abort(
          self._root_pipeline_key, abort_message=abort_message)

  # Methods used by the Pipeline as it runs.
  def fill(self, name_or_slot, value):
    """Fills an output slot required by this Pipeline.

    Args:
      name_or_slot: The name of the slot (a string) or Slot record to fill.
      value: The serializable value to assign to this slot.

    Raises:
      UnexpectedPipelineError if the Slot no longer exists. SlotNotDeclaredError
      if trying to output to a slot that was not declared ahead of time.
    """
    if isinstance(name_or_slot, basestring):
      slot = getattr(self.outputs, name_or_slot)
    elif isinstance(name_or_slot, Slot):
      slot = name_or_slot
    else:
      raise UnexpectedPipelineError(
          'Could not fill invalid output name: %r' % name_or_slot)

    if not slot._exists:
      raise SlotNotDeclaredError(
          'Cannot fill output with name "%s" that was just '
          'declared within the Pipeline context.' % slot.name)

    self._context.fill_slot(self._pipeline_key, slot, value)

  def set_status(self, message=None, console_url=None, status_links=None):
    """Sets the current status of this pipeline.

    This method is purposefully non-transactional. Updates are written to the
    datastore immediately and overwrite all existing statuses.

    Args:
      message: (optional) Overall status message.
      console_url: (optional) Relative URL to use for the "console" of this
        pipeline that displays current progress. When None, no console will
        be displayed.
      status_links: (optional) Dictionary of readable link names to relative
        URLs that should be associated with this pipeline as it runs. These links
        provide convenient access to other dashboards, consoles, etc associated
        with the pipeline.

    Raises:
      PipelineRuntimeError if the status could not be set for any reason.
    """
    if _TEST_MODE:
      logging.info(
          'New status for %s#%s: message=%r, console_url=%r, status_links=%r',
          self, self.pipeline_id, message, console_url, status_links)
      return

    status_key = db.Key.from_path(_StatusRecord.kind(), self.pipeline_id)
    root_pipeline_key = db.Key.from_path(
        _PipelineRecord.kind(), self.root_pipeline_id)
    status_record = _StatusRecord(
        key=status_key, root_pipeline=root_pipeline_key)

    try:
      if message:
        status_record.message = message
      if console_url:
        status_record.console_url = console_url
      if status_links:
        # Alphabeticalize the list.
        status_record.link_names = sorted(
            db.Text(s) for s in status_links.iterkeys())
        status_record.link_urls = [
            db.Text(status_links[name]) for name in status_record.link_names]

      status_record.status_time = datetime.datetime.utcnow()

      status_record.put()
    except Exception, e:
      raise PipelineRuntimeError('Could not set status for %s#%s: %s' %
          (self, self.pipeline_id, str(e)))

  def complete(self, default_output=None):
    """Marks this asynchronous Pipeline as complete.

    Args:
      default_output: What value the 'default' output slot should be assigned.

    Raises:
      UnexpectedPipelineError if the slot no longer exists or this method was
      called for a pipeline that is not async.
    """
    # TODO: Enforce that all outputs expected by this async pipeline were
    # filled before this complete() function was called. May required all
    # async functions to declare their outputs upfront.
    if not self.async:
      raise UnexpectedPipelineError(
          'May only call complete() method for asynchronous pipelines.')
    self._context.fill_slot(
        self._pipeline_key, self.outputs.default, default_output)

  def get_callback_url(self, **kwargs):
    """Returns a relative URL for invoking this Pipeline's callback method.

    Args:
      kwargs: Dictionary mapping keyword argument names to single values that
        should be passed to the callback when it is invoked.

    Raises:
      UnexpectedPipelineError if this is invoked on pipeline that is not async.
    """
    # TODO: Support positional parameters.
    if not self.async:
      raise UnexpectedPipelineError(
          'May only call get_callback_url() method for asynchronous pipelines.')
    kwargs['pipeline_id'] = self._pipeline_key.name()
    params = urllib.urlencode(sorted(kwargs.items()))
    return '%s/callback?%s' % (self.base_path, params)

  def get_callback_task(self, *args, **kwargs):
    """Returns a task for calling back this Pipeline.

    Args:
      params: Keyword argument containing a dictionary of key/value pairs
        that will be passed to the callback when it is executed.
      args, kwargs: Passed to the taskqueue.Task constructor. Use these
        arguments to set the task name (for idempotence), etc.

    Returns:
      A taskqueue.Task instance that must be enqueued by the caller.
    """
    if not self.async:
      raise UnexpectedPipelineError(
          'May only call get_callback_task() method for asynchronous pipelines.')

    params = kwargs.get('params', {})
    kwargs['params'] = params
    params['pipeline_id'] = self._pipeline_key.name()
    kwargs['url'] = self.base_path + '/callback'
    kwargs['method'] = 'POST'
    return taskqueue.Task(*args, **kwargs)

  def send_result_email(self, sender=None):
    """Sends an email to admins indicating this Pipeline has completed.

    For developer convenience. Automatically called from finalized for root
    Pipelines that do not override the default action.

    Args:
      sender: (optional) Override the sender's email address.
    """
    status = 'successful'
    if self.was_aborted:
      status = 'aborted'

    app_id = os.environ['APPLICATION_ID']
    shard_index = app_id.find('~')
    if shard_index != -1:
      app_id = app_id[shard_index+1:]

    param_dict = {
        'status': status,
        'app_id': app_id,
        'class_path': self._class_path,
        'pipeline_id': self.root_pipeline_id,
        'base_path': '%s.appspot.com%s' % (app_id, self.base_path),
    }
    subject = (
        'Pipeline %(status)s: App "%(app_id)s", %(class_path)s'
        '#%(pipeline_id)s' % param_dict)
    body = """View the pipeline results here:

http://%(base_path)s/status?root=%(pipeline_id)s

Thanks,

The Pipeline API
""" % param_dict

    html = """<html><body>
<p>View the pipeline results here:</p>

<p><a href="http://%(base_path)s/status?root=%(pipeline_id)s"
>http://%(base_path)s/status?root=%(pipeline_id)s</a></p>

<p>
Thanks,
<br>
The Pipeline API
</p>
</body></html>
""" % param_dict

    if sender is None:
      sender = '%s@%s.appspotmail.com' % (app_id, app_id)
    try:
      self._send_mail(sender, subject, body, html=html)
    except (mail.InvalidSenderError, mail.InvalidEmailError):
      logging.warning('Could not send result email for '
                      'root pipeline ID "%s" from sender "%s"',
                      self.root_pipeline_id, sender)

  def cleanup(self):
    """Clean up this Pipeline and all Datastore records used for coordination.

    Only works when called on a root pipeline. Child pipelines will ignore
    calls to this method.

    After this method is called, Pipeline.from_id() and related status
    methods will return inconsistent or missing results. This method is
    fire-and-forget and asynchronous.
    """
    if self._root_pipeline_key is None:
      raise UnexpectedPipelineError(
          'Could not cleanup Pipeline with unknown root pipeline ID.')
    if not self.is_root:
      return
    task = taskqueue.Task(
        params=dict(root_pipeline_key=self._root_pipeline_key),
        url=self.base_path + '/cleanup',
        headers={'X-Ae-Pipeline-Key': self._root_pipeline_key})
    taskqueue.Queue(self.queue_name).add(task)

  def with_params(self, **kwargs):
    """Modify various execution parameters of a Pipeline before it runs.

    This method has no effect in test mode.

    Args:
      kwargs: Attributes to modify on this Pipeline instance before it has
        been executed.

    Returns:
      This Pipeline instance, for easy chaining.
    """
    if _TEST_MODE:
      logging.info(
          'Setting runtime parameters for %s#%s: %r',
          self, self.pipeline_id, kwargs)
      return self

    if self.pipeline_id is not None:
      raise UnexpectedPipelineError(
          'May only call with_params() on a Pipeline that has not yet '
          'been scheduled for execution.')

    ALLOWED = ('backoff_seconds', 'backoff_factor', 'max_attempts', 'target')
    for name, value in kwargs.iteritems():
      if name not in ALLOWED:
        raise TypeError('Unexpected keyword: %s=%r' % (name, value))
      setattr(self, name, value)
    return self

  # Methods implemented by developers for lifecycle management. These
  # must be idempotent under all circumstances.
  def run(self, *args, **kwargs):
    """Runs this Pipeline."""
    raise NotImplementedError('Must implement "run" in Pipeline sub-class.')

  def run_test(self, *args, **kwargs):
    """Runs this Pipeline in test mode."""
    raise NotImplementedError(
        'Must implement "run_test" in Pipeline sub-class.')

  def finalized(self):
    """Finalizes this Pipeline after execution if it's a generator.

    Default action as the root pipeline is to email the admins with the status.
    Implementors be sure to call 'was_aborted' to find out if the finalization
    that you're handling is for a success or error case.
    """
    if self.pipeline_id == self.root_pipeline_id:
      self.send_result_email()

  def finalized_test(self, *args, **kwargs):
    """Finalized this Pipeline in test mode."""
    raise NotImplementedError(
        'Must implement "finalized_test" in Pipeline sub-class.')

  def callback(self, **kwargs):
    """This Pipeline received an asynchronous callback request."""
    raise NotImplementedError(
        'Must implement "callback" in Pipeline sub-class.')

  def try_cancel(self):
    """This pipeline has been cancelled.

    Called when a pipeline is interrupted part-way through due to some kind
    of failure (an abort of the whole pipeline to the root or a forced retry on
    this child pipeline).

    Returns:
      True to indicate that cancellation was successful and this pipeline may
      go in the retry or aborted state; False to indicate that this pipeline
      cannot be canceled right now and must remain as-is.
    """
    return False

  # Internal methods.
  @classmethod
  def _set_class_path(cls, module_dict=sys.modules):
    """Sets the absolute path to this class as a string.

    Used by the Pipeline API to reconstruct the Pipeline sub-class object
    at execution time instead of passing around a serialized function.

    Args:
      module_dict: Used for testing.
    """
    # Do not traverse the class hierarchy fetching the class path attribute.
    found = cls.__dict__.get('_class_path')
    if found is not None:
      return

    # Do not set the _class_path for the base-class, otherwise all children's
    # lookups for _class_path will fall through and return 'Pipeline' above.
    # This situation can happen if users call the generic Pipeline.from_id
    # to get the result of a Pipeline without knowing its specific class.
    if cls is Pipeline:
      return

    class_path = '%s.%s' % (cls.__module__, cls.__name__)
    # When a WSGI handler is invoked as an entry point, any Pipeline class
    # defined in the same file as the handler will get __module__ set to
    # __main__. Thus we need to find out its real fully qualified path.
    if cls.__module__ == '__main__':
      for name, module in module_dict.items():
        if name == '__main__':
          continue
        found = getattr(module, cls.__name__, None)
        if found is cls:
          class_path = '%s.%s' % (name, cls.__name__)
          break
    cls._class_path = class_path

  def _set_values_internal(self,
                           context,
                           pipeline_key,
                           root_pipeline_key,
                           outputs,
                           result_status):
    """Sets the user-visible values provided as an API by this class.

    Args:
      context: The _PipelineContext used for this Pipeline.
      pipeline_key: The db.Key of this pipeline.
      root_pipeline_key: The db.Key of the root pipeline.
      outputs: The PipelineFuture for this pipeline.
      result_status: The result status of this pipeline.
    """
    self._context = context
    self._pipeline_key = pipeline_key
    self._root_pipeline_key = root_pipeline_key
    self._result_status = result_status
    self.outputs = outputs

  def _callback_internal(self, kwargs):
    """Used to execute callbacks on asynchronous pipelines."""
    logging.debug('Callback %s(*%s, **%s)#%s with params: %r',
                  self._class_path, _short_repr(self.args),
                  _short_repr(self.kwargs), self._pipeline_key.name(), kwargs)
    return self.callback(**kwargs)

  def _run_internal(self,
                    context,
                    pipeline_key,
                    root_pipeline_key,
                    caller_output):
    """Used by the Pipeline evaluator to execute this Pipeline."""
    self._set_values_internal(
        context, pipeline_key, root_pipeline_key, caller_output,
        _PipelineRecord.RUN)
    logging.debug('Running %s(*%s, **%s)#%s',
                  self._class_path, _short_repr(self.args),
                  _short_repr(self.kwargs), self._pipeline_key.name())
    return self.run(*self.args, **self.kwargs)

  def _finalized_internal(self,
                          context,
                          pipeline_key,
                          root_pipeline_key,
                          caller_output,
                          aborted):
    """Used by the Pipeline evaluator to finalize this Pipeline."""
    result_status = _PipelineRecord.RUN
    if aborted:
      result_status = _PipelineRecord.ABORTED

    self._set_values_internal(
        context, pipeline_key, root_pipeline_key, caller_output, result_status)
    logging.debug('Finalizing %s(*%r, **%r)#%s',
                  self._class_path, _short_repr(self.args),
                  _short_repr(self.kwargs), self._pipeline_key.name())
    try:
      self.finalized()
    except NotImplementedError:
      pass

  def __repr__(self):
    """Returns a string representation of this Pipeline."""
    return '%s(*%s, **%s)' % (
        self._class_path, _short_repr(self.args), _short_repr(self.kwargs))


# TODO: Change InOrder and After to use a common thread-local list of
# execution modifications to apply to the current evaluating pipeline.

class After(object):
  """Causes all contained Pipelines to run after the given ones complete.

  Must be used in a 'with' block.
  """

  _local = threading.local()

  def __init__(self, *futures):
    """Initializer.

    Args:
      *futures: PipelineFutures that all subsequent pipelines should follow.
        May be empty, in which case this statement does nothing.
    """
    for f in futures:
      if not isinstance(f, PipelineFuture):
        raise TypeError('May only pass PipelineFuture instances to After(). %r',
                        type(f))
    self._futures = set(futures)

  def __enter__(self):
    """When entering a 'with' block."""
    After._thread_init()
    After._local._after_all_futures.extend(self._futures)

  def __exit__(self, type, value, trace):
    """When exiting a 'with' block."""
    for future in self._futures:
      After._local._after_all_futures.remove(future)
    return False

  @classmethod
  def _thread_init(cls):
    """Ensure thread local is initialized."""
    if not hasattr(cls._local, '_after_all_futures'):
      cls._local._after_all_futures = []


class InOrder(object):
  """Causes all contained Pipelines to run in order.

  Must be used in a 'with' block.
  """

  _local = threading.local()

  @classmethod
  def _add_future(cls, future):
    """Adds a future to the list of in-order futures thus far.

    Args:
      future: The future to add to the list.
    """
    if cls._local._activated:
      cls._local._in_order_futures.add(future)

  def __init__(self):
    """Initializer."""

  def __enter__(self):
    """When entering a 'with' block."""
    InOrder._thread_init()
    if InOrder._local._activated:
      raise UnexpectedPipelineError('Already in an InOrder "with" block.')
    InOrder._local._activated = True
    InOrder._local._in_order_futures.clear()

  def __exit__(self, type, value, trace):
    """When exiting a 'with' block."""
    InOrder._local._activated = False
    InOrder._local._in_order_futures.clear()
    return False

  @classmethod
  def _thread_init(cls):
    """Ensure thread local is initialized."""
    if not hasattr(cls._local, '_in_order_futures'):
      cls._local._in_order_futures = set()
      cls._local._activated = False


################################################################################

def _short_repr(obj):
  """Helper function returns a truncated repr() of an object."""
  stringified = pprint.saferepr(obj)
  if len(stringified) > 200:
    return '%s... (%d bytes)' % (stringified[:200], len(stringified))
  return stringified


def _write_json_blob(encoded_value, pipeline_id=None):
  """Writes a JSON encoded value to a Cloud Storage File.
  
  This function will store the blob in a GCS file in the default bucket under
  the appengine_pipeline directory. Optionally using another directory level
  specified by pipeline_id
  Args:
    encoded_value: The encoded JSON string.
    pipeline_id: A pipeline id to segment files in Cloud Storage, if none, 
      the file will be created under appengine_pipeline

  Returns:
    The blobstore.BlobKey for the file that was created.
  """
  
  default_bucket = app_identity.get_default_gcs_bucket_name()
  path_components = ['/', default_bucket, "appengine_pipeline"]
  if pipeline_id:
    path_components.append(pipeline_id)
  path_components.append(uuid.uuid4().hex)
  # Use posixpath to get a / even if we're running on windows somehow
  file_name = posixpath.join(*path_components)
  with cloudstorage.open(file_name, 'w', content_type='application/json') as f:
    for start_index in xrange(0, len(encoded_value), _MAX_JSON_SIZE):
      end_index = start_index + _MAX_JSON_SIZE
      f.write(encoded_value[start_index:end_index])

  key_str = blobstore.create_gs_key("/gs" + file_name)
  logging.debug("Created blob for filename = %s gs_key = %s", file_name, key_str)
  return blobstore.BlobKey(key_str)


def _dereference_args(pipeline_name, args, kwargs):
  """Dereference a Pipeline's arguments that are slots, validating them.

  Each argument value passed in is assumed to be a dictionary with the format:
    {'type': 'value', 'value': 'serializable'}  # A resolved value.
    {'type': 'slot', 'slot_key': 'str() on a db.Key'}  # A pending Slot.

  Args:
    pipeline_name: The name of the pipeline class; used for debugging.
    args: Iterable of positional arguments.
    kwargs: Dictionary of keyword arguments.

  Returns:
    Tuple (args, kwargs) where:
      Args: A list of positional arguments values that are all dereferenced.
      Kwargs: A list of keyword arguments values that are all dereferenced.

  Raises:
    SlotNotFilledError if any of the supplied 'slot_key' records are not
    present in the Datastore or have not yet been filled.
    UnexpectedPipelineError if an unknown parameter type was passed.
  """
  lookup_slots = set()
  for arg in itertools.chain(args, kwargs.itervalues()):
    if arg['type'] == 'slot':
      lookup_slots.add(db.Key(arg['slot_key']))

  slot_dict = {}
  for key, slot_record in zip(lookup_slots, db.get(lookup_slots)):
    if slot_record is None or slot_record.status != _SlotRecord.FILLED:
      raise SlotNotFilledError(
          'Slot "%s" missing its value. From %s(*args=%s, **kwargs=%s)' %
          (key, pipeline_name, _short_repr(args), _short_repr(kwargs)))
    slot_dict[key] = slot_record.value

  arg_list = []
  for current_arg in args:
    if current_arg['type'] == 'slot':
      arg_list.append(slot_dict[db.Key(current_arg['slot_key'])])
    elif current_arg['type'] == 'value':
      arg_list.append(current_arg['value'])
    else:
      raise UnexpectedPipelineError('Unknown parameter type: %r' % current_arg)

  kwarg_dict = {}
  for key, current_arg in kwargs.iteritems():
    if current_arg['type'] == 'slot':
      kwarg_dict[key] = slot_dict[db.Key(current_arg['slot_key'])]
    elif current_arg['type'] == 'value':
      kwarg_dict[key] = current_arg['value']
    else:
      raise UnexpectedPipelineError('Unknown parameter type: %r' % current_arg)

  return (arg_list, kwarg_dict)


def _generate_args(pipeline, future, queue_name, base_path):
  """Generate the params used to describe a Pipeline's depedencies.

  The arguments passed to this method may be normal values, Slot instances
  (for named outputs), or PipelineFuture instances (for referring to the
  default output slot).

  Args:
    pipeline: The Pipeline instance to generate args for.
    future: The PipelineFuture for the Pipeline these arguments correspond to.
    queue_name: The queue to run the pipeline on.
    base_path: Relative URL for pipeline URL handlers.

  Returns:
    Tuple (dependent_slots, output_slot_keys, params_text, params_blob) where:
      dependent_slots: List of db.Key instances of _SlotRecords on which
        this pipeline will need to block before execution (passed to
        create a _BarrierRecord for running the pipeline).
      output_slot_keys: List of db.Key instances of _SlotRecords that will
        be filled by this pipeline during its execution (passed to create
        a _BarrierRecord for finalizing the pipeline).
      params_text: JSON dictionary of pipeline parameters to be serialized and
        saved in a corresponding _PipelineRecord. Will be None if the params are
        too big and must be saved in a blob instead.
      params_blob: JSON dictionary of pipeline parameters to be serialized and
        saved in a Blob file, and then attached to a _PipelineRecord. Will be
        None if the params data size was small enough to fit in the entity.
  """
  params = {
      'args': [],
      'kwargs': {},
      'after_all': [],
      'output_slots': {},
      'class_path': pipeline._class_path,
      'queue_name': queue_name,
      'base_path': base_path,
      'backoff_seconds': pipeline.backoff_seconds,
      'backoff_factor': pipeline.backoff_factor,
      'max_attempts': pipeline.max_attempts,
      'task_retry': pipeline.task_retry,
      'target': pipeline.target,
  }
  dependent_slots = set()

  arg_list = params['args']
  for current_arg in pipeline.args:
    if isinstance(current_arg, PipelineFuture):
      current_arg = current_arg.default
    if isinstance(current_arg, Slot):
      arg_list.append({'type': 'slot', 'slot_key': str(current_arg.key)})
      dependent_slots.add(current_arg.key)
    else:
      arg_list.append({'type': 'value', 'value': current_arg})

  kwarg_dict = params['kwargs']
  for name, current_arg in pipeline.kwargs.iteritems():
    if isinstance(current_arg, PipelineFuture):
      current_arg = current_arg.default
    if isinstance(current_arg, Slot):
      kwarg_dict[name] = {'type': 'slot', 'slot_key': str(current_arg.key)}
      dependent_slots.add(current_arg.key)
    else:
      kwarg_dict[name] = {'type': 'value', 'value': current_arg}

  after_all = params['after_all']
  for other_future in future._after_all_pipelines:
    slot_key = other_future._output_dict['default'].key
    after_all.append(str(slot_key))
    dependent_slots.add(slot_key)

  output_slots = params['output_slots']
  output_slot_keys = set()
  for name, slot in future._output_dict.iteritems():
    output_slot_keys.add(slot.key)
    output_slots[name] = str(slot.key)

  params_encoded = json.dumps(params, cls=mr_util.JsonEncoder)
  params_text = None
  params_blob = None
  if len(params_encoded) > _MAX_JSON_SIZE:
    params_blob = _write_json_blob(params_encoded, pipeline.pipeline_id)
  else:
    params_text = params_encoded

  return dependent_slots, output_slot_keys, params_text, params_blob


class _PipelineContext(object):
  """Internal API for interacting with Pipeline state."""

  _gettime = datetime.datetime.utcnow

  def __init__(self,
               task_name,
               queue_name,
               base_path):
    """Initializer.

    Args:
      task_name: The name of the currently running task or empty if there
        is no task running.
      queue_name: The queue this pipeline should run on (may not be the
        current queue this request is on).
      base_path: Relative URL for the pipeline's handlers.
    """
    self.task_name = task_name
    self.queue_name = queue_name
    self.base_path = base_path
    self.barrier_handler_path = '%s/output' % base_path
    self.pipeline_handler_path = '%s/run' % base_path
    self.finalized_handler_path = '%s/finalized' % base_path
    self.fanout_handler_path = '%s/fanout' % base_path
    self.abort_handler_path = '%s/abort' % base_path
    self.fanout_abort_handler_path = '%s/fanout_abort' % base_path
    self.session_filled_output_names = set()

  @classmethod
  def from_environ(cls, environ=os.environ):
    """Constructs a _PipelineContext from the task queue environment."""
    base_path, unused = (environ['PATH_INFO'].rsplit('/', 1) + [''])[:2]
    return cls(
        environ['HTTP_X_APPENGINE_TASKNAME'],
        environ['HTTP_X_APPENGINE_QUEUENAME'],
        base_path)

  def fill_slot(self, filler_pipeline_key, slot, value):
    """Fills a slot, enqueueing a task to trigger pending barriers.

    Args:
      filler_pipeline_key: db.Key or stringified key of the _PipelineRecord
        that filled this slot.
      slot: The Slot instance to fill.
      value: The serializable value to assign.

    Raises:
      UnexpectedPipelineError if the _SlotRecord for the 'slot' could not
      be found in the Datastore.
    """
    if not isinstance(filler_pipeline_key, db.Key):
      filler_pipeline_key = db.Key(filler_pipeline_key)

    if _TEST_MODE:
      slot._set_value_test(filler_pipeline_key, value)
    else:
      encoded_value = json.dumps(value,
                                       sort_keys=True,
                                       cls=mr_util.JsonEncoder)
      value_text = None
      value_blob = None
      if len(encoded_value) <= _MAX_JSON_SIZE:
        value_text = db.Text(encoded_value)
      else:
        # The encoded value is too big. Save it as a blob.
        value_blob = _write_json_blob(encoded_value, filler_pipeline_key.name())

      def txn():
        slot_record = db.get(slot.key)
        if slot_record is None:
          raise UnexpectedPipelineError(
              'Tried to fill missing slot "%s" '
              'by pipeline ID "%s" with value: %r'
              % (slot.key, filler_pipeline_key.name(), value))
        # NOTE: Always take the override value here. If down-stream pipelines
        # need a consitent view of all up-stream outputs (meaning, all of the
        # outputs came from the same retry attempt of the upstream pipeline),
        # the down-stream pipeline must also wait for the 'default' output
        # of these up-stream pipelines.
        slot_record.filler = filler_pipeline_key
        slot_record.value_text = value_text
        slot_record.value_blob = value_blob
        slot_record.status = _SlotRecord.FILLED
        slot_record.fill_time = self._gettime()
        slot_record.put()
        task = taskqueue.Task(
            url=self.barrier_handler_path,
            params=dict(
                slot_key=slot.key,
                use_barrier_indexes=True),
            headers={'X-Ae-Slot-Key': slot.key,
                     'X-Ae-Filler-Pipeline-Key': filler_pipeline_key})
        task.add(queue_name=self.queue_name, transactional=True)
      db.run_in_transaction_options(
          db.create_transaction_options(propagation=db.ALLOWED), txn)

    self.session_filled_output_names.add(slot.name)

  def notify_barriers(self,
                      slot_key,
                      cursor,
                      use_barrier_indexes,
                      max_to_notify=_MAX_BARRIERS_TO_NOTIFY):
    """Searches for barriers affected by a slot and triggers completed ones.

    Args:
      slot_key: db.Key or stringified key of the _SlotRecord that was filled.
      cursor: Stringified Datastore cursor where the notification query
        should pick up.
      use_barrier_indexes: When True, use _BarrierIndex records to determine
        which _Barriers to trigger by having this _SlotRecord filled. When
        False, use the old method that queries for _BarrierRecords by
        the blocking_slots parameter.
      max_to_notify: Used for testing.

    Raises:
      PipelineStatusError: If any of the barriers are in a bad state.
    """
    if not isinstance(slot_key, db.Key):
      slot_key = db.Key(slot_key)
    logging.debug('Notifying slot %r', slot_key)

    if use_barrier_indexes:
      # Please see models.py:_BarrierIndex to understand how _BarrierIndex
      # entities relate to _BarrierRecord entities.
      query = (
          _BarrierIndex.all(cursor=cursor, keys_only=True)
          .ancestor(slot_key))
      barrier_index_list = query.fetch(max_to_notify)
      barrier_key_list = [
          _BarrierIndex.to_barrier_key(key) for key in barrier_index_list]

      # If there are task and pipeline kickoff retries it's possible for a
      # _BarrierIndex to exist for a _BarrierRecord that was not successfully
      # written. It's safe to ignore this because the original task that wrote
      # the _BarrierIndex and _BarrierRecord would not have made progress to
      # kick off a real pipeline or child pipeline unless all of the writes for
      # these dependent entities went through. We assume that the instigator
      # retried from scratch and somehwere there exists a good _BarrierIndex and
      # corresponding _BarrierRecord that tries to accomplish the same thing.
      barriers = db.get(barrier_key_list)
      results = []
      for barrier_key, barrier in zip(barrier_key_list, barriers):
        if barrier is None:
          logging.debug('Ignoring that Barrier "%r" is missing, '
                        'relies on Slot "%r"', barrier_key, slot_key)
        else:
          results.append(barrier)
    else:
      # TODO(user): Delete this backwards compatible codepath and
      # make use_barrier_indexes the assumed default in all cases.
      query = (
          _BarrierRecord.all(cursor=cursor)
          .filter('blocking_slots =', slot_key))
      results = query.fetch(max_to_notify)

    # Fetch all blocking _SlotRecords for any potentially triggered barriers.
    blocking_slot_keys = []
    for barrier in results:
      blocking_slot_keys.extend(barrier.blocking_slots)

    blocking_slot_dict = {}
    for slot_record in db.get(blocking_slot_keys):
      if slot_record is None:
        continue
      blocking_slot_dict[slot_record.key()] = slot_record

    task_list = []
    updated_barriers = []
    for barrier in results:
      ready_slots = []
      for blocking_slot_key in barrier.blocking_slots:
        slot_record = blocking_slot_dict.get(blocking_slot_key)
        if slot_record is None:
          raise UnexpectedPipelineError(
              'Barrier "%r" relies on Slot "%r" which is missing.' %
              (barrier.key(), blocking_slot_key))
        if slot_record.status == _SlotRecord.FILLED:
          ready_slots.append(blocking_slot_key)

      # When all of the blocking_slots have been filled, consider the barrier
      # ready to trigger. We'll trigger it regardless of the current
      # _BarrierRecord status, since there could be task queue failures at any
      # point in this flow; this rolls forward the state and de-dupes using
      # the task name tombstones.
      pending_slots = set(barrier.blocking_slots) - set(ready_slots)
      if not pending_slots:
        if barrier.status != _BarrierRecord.FIRED:
          barrier.status = _BarrierRecord.FIRED
          barrier.trigger_time = self._gettime()
          updated_barriers.append(barrier)

        purpose = barrier.key().name()
        if purpose == _BarrierRecord.START:
          path = self.pipeline_handler_path
          countdown = None
        else:
          path = self.finalized_handler_path
          # NOTE: Wait one second before finalization to prevent
          # contention on the _PipelineRecord entity.
          countdown = 1
        pipeline_key = _BarrierRecord.target.get_value_for_datastore(barrier)
        logging.debug('Firing barrier %r', barrier.key())
        task_list.append(taskqueue.Task(
            url=path,
            countdown=countdown,
            name='ae-barrier-fire-%s-%s' % (pipeline_key.name(), purpose),
            params=dict(pipeline_key=pipeline_key, purpose=purpose),
            headers={'X-Ae-Pipeline-Key': pipeline_key}))
      else:
        logging.debug('Not firing barrier %r, Waiting for slots: %r',
                      barrier.key(), pending_slots)

    # Blindly overwrite _BarrierRecords that have an updated status. This is
    # acceptable because by this point all finalization barriers for
    # generator children should have already had their final outputs assigned.
    if updated_barriers:
      db.put(updated_barriers)

    # Task continuation with sequence number to prevent fork-bombs.
    if len(results) == max_to_notify:
      the_match = re.match('(.*)-ae-barrier-notify-([0-9]+)', self.task_name)
      if the_match:
        prefix = the_match.group(1)
        end = int(the_match.group(2)) + 1
      else:
        prefix = self.task_name
        end = 0
      task_list.append(taskqueue.Task(
          name='%s-ae-barrier-notify-%d' % (prefix, end),
          url=self.barrier_handler_path,
          params=dict(
              slot_key=slot_key,
              cursor=query.cursor(),
              use_barrier_indexes=use_barrier_indexes)))

    if task_list:
      try:
        taskqueue.Queue(self.queue_name).add(task_list)
      except (taskqueue.TombstonedTaskError, taskqueue.TaskAlreadyExistsError):
        pass

  def begin_abort(self, root_pipeline_key, abort_message):
    """Kicks off the abort process for a root pipeline and all its children.

    Args:
      root_pipeline_key: db.Key of the root pipeline to abort.
      abort_message: Message explaining why the abort happened, only saved
          into the root pipeline.

    Returns:
      True if the abort signal was sent successfully; False otherwise.
    """
    def txn():
      pipeline_record = db.get(root_pipeline_key)
      if pipeline_record is None:
        logging.warning(
            'Tried to abort root pipeline ID "%s" but it does not exist.',
            root_pipeline_key.name())
        raise db.Rollback()
      if pipeline_record.status == _PipelineRecord.ABORTED:
        logging.warning(
            'Tried to abort root pipeline ID "%s"; already in state: %s',
            root_pipeline_key.name(), pipeline_record.status)
        raise db.Rollback()
      if pipeline_record.abort_requested:
        logging.warning(
            'Tried to abort root pipeline ID "%s"; abort signal already sent.',
            root_pipeline_key.name())
        raise db.Rollback()

      pipeline_record.abort_requested = True
      pipeline_record.abort_message = abort_message
      pipeline_record.put()

      task = taskqueue.Task(
          url=self.fanout_abort_handler_path,
          params=dict(root_pipeline_key=root_pipeline_key))
      task.add(queue_name=self.queue_name, transactional=True)
      return True

    return db.run_in_transaction(txn)

  def continue_abort(self,
                     root_pipeline_key,
                     cursor=None,
                     max_to_notify=_MAX_ABORTS_TO_BEGIN):
    """Sends the abort signal to all children for a root pipeline.

    Args:
      root_pipeline_key: db.Key of the root pipeline to abort.
      cursor: The query cursor for enumerating _PipelineRecords when inserting
        tasks to cause child pipelines to terminate.
      max_to_notify: Used for testing.
    """
    if not isinstance(root_pipeline_key, db.Key):
      root_pipeline_key = db.Key(root_pipeline_key)
    # NOTE: The results of this query may include _PipelineRecord instances
    # that are not actually "reachable", meaning you cannot get to them by
    # starting at the root pipeline and following "fanned_out" onward. This
    # is acceptable because even these defunct _PipelineRecords will properly
    # set their status to ABORTED when the signal comes, regardless of any
    # other status they may have had.
    #
    # The only gotcha here is if a Pipeline's finalize method somehow modifies
    # its inputs (like deleting an input file). In the case there are
    # unreachable child pipelines, it will appear as if two finalize methods
    # have been called instead of just one. The saving grace here is that
    # finalize must be idempotent, so this *should* be harmless.
    query = (
        _PipelineRecord.all(cursor=cursor)
        .filter('root_pipeline =', root_pipeline_key))
    results = query.fetch(max_to_notify)

    task_list = []
    for pipeline_record in results:
      if pipeline_record.status not in (
          _PipelineRecord.RUN, _PipelineRecord.WAITING):
        continue

      pipeline_key = pipeline_record.key()
      task_list.append(taskqueue.Task(
          name='%s-%s-abort' % (self.task_name, pipeline_key.name()),
          url=self.abort_handler_path,
          params=dict(pipeline_key=pipeline_key, purpose=_BarrierRecord.ABORT),
          headers={'X-Ae-Pipeline-Key': pipeline_key}))

    # Task continuation with sequence number to prevent fork-bombs.
    if len(results) == max_to_notify:
      the_match = re.match('(.*)-([0-9]+)', self.task_name)
      if the_match:
        prefix = the_match.group(1)
        end = int(the_match.group(2)) + 1
      else:
        prefix = self.task_name
        end = 0
      task_list.append(taskqueue.Task(
          name='%s-%d' % (prefix, end),
          url=self.fanout_abort_handler_path,
          params=dict(root_pipeline_key=root_pipeline_key,
                      cursor=query.cursor())))

    if task_list:
      try:
        taskqueue.Queue(self.queue_name).add(task_list)
      except (taskqueue.TombstonedTaskError, taskqueue.TaskAlreadyExistsError):
        pass

  def start(self, pipeline, return_task=True, countdown=None, eta=None):
    """Starts a pipeline.

    Args:
      pipeline: Pipeline instance to run.
      return_task: When True, do not submit the task to start the pipeline
        but instead return it for someone else to enqueue.
      countdown: Time in seconds into the future that this Task should execute.
        Defaults to zero.
      eta: A datetime.datetime specifying the absolute time at which the task
        should be executed. Must not be specified if 'countdown' is specified.
        This may be timezone-aware or timezone-naive. If None, defaults to now.
        For pull tasks, no worker will be able to lease this task before the
        time indicated by eta.

    Returns:
      The task to start this pipeline if return_task was True.

    Raises:
      PipelineExistsError if the pipeline with the given ID already exists.
    """
    # Adjust all pipeline output keys for this Pipeline to be children of
    # the _PipelineRecord, that way we can write them all and submit in a
    # single transaction.
    for name, slot in pipeline.outputs._output_dict.iteritems():
      slot.key = db.Key.from_path(
          *slot.key.to_path(), **dict(parent=pipeline._pipeline_key))

    _, output_slots, params_text, params_blob = _generate_args(
        pipeline, pipeline.outputs, self.queue_name, self.base_path)

    @db.transactional(propagation=db.INDEPENDENT)
    def txn():
      pipeline_record = db.get(pipeline._pipeline_key)
      if pipeline_record is not None:
        raise PipelineExistsError(
            'Pipeline with idempotence key "%s" already exists; params=%s' %
            (pipeline._pipeline_key.name(),
             _short_repr(pipeline_record.params)))

      entities_to_put = []
      for name, slot in pipeline.outputs._output_dict.iteritems():
        entities_to_put.append(_SlotRecord(
            key=slot.key,
            root_pipeline=pipeline._pipeline_key))

      entities_to_put.append(_PipelineRecord(
          key=pipeline._pipeline_key,
          root_pipeline=pipeline._pipeline_key,
          is_root_pipeline=True,
          # Bug in DB means we need to use the storage name here,
          # not the local property name.
          params=params_text,
          params_blob=params_blob,
          start_time=self._gettime(),
          class_path=pipeline._class_path,
          max_attempts=pipeline.max_attempts))

      entities_to_put.extend(_PipelineContext._create_barrier_entities(
          pipeline._pipeline_key,
          pipeline._pipeline_key,
          _BarrierRecord.FINALIZE,
          output_slots))

      db.put(entities_to_put)

      task = taskqueue.Task(
          url=self.pipeline_handler_path,
          params=dict(pipeline_key=pipeline._pipeline_key),
          headers={'X-Ae-Pipeline-Key': pipeline._pipeline_key},
          target=pipeline.target,
          countdown=countdown,
          eta=eta)
      if return_task:
        return task
      task.add(queue_name=self.queue_name, transactional=True)

    task = txn()
    # Immediately mark the output slots as existing so they can be filled
    # by asynchronous pipelines or used in test mode.
    for output_slot in pipeline.outputs._output_dict.itervalues():
      output_slot._exists = True
    return task

  def start_test(self, pipeline):
    """Starts a pipeline in the test mode.

    Args:
      pipeline: The Pipeline instance to test.
    """
    global _TEST_MODE, _TEST_ROOT_PIPELINE_KEY
    self.start(pipeline, return_task=True)
    _TEST_MODE = True
    _TEST_ROOT_PIPELINE_KEY = pipeline._pipeline_key
    try:
      self.evaluate_test(pipeline, root=True)
    finally:
      _TEST_MODE = False

  def evaluate_test(self, stage, root=False):
    """Recursively evaluates the given pipeline in test mode.

    Args:
      stage: The Pipeline instance to run at this stage in the flow.
      root: True if the supplied stage is the root of the pipeline.
    """
    args_adjusted = []
    for arg in stage.args:
      if isinstance(arg, PipelineFuture):
        arg = arg.default
      if isinstance(arg, Slot):
        value = arg.value
        arg._touched = True
      else:
        value = arg
      args_adjusted.append(value)

    kwargs_adjusted = {}
    for name, arg in stage.kwargs.iteritems():
      if isinstance(arg, PipelineFuture):
        arg = arg.default
      if isinstance(arg, Slot):
        value = arg.value
        arg._touched = True
      else:
        value = arg
      kwargs_adjusted[name] = value

    stage.args, stage.kwargs = args_adjusted, kwargs_adjusted
    pipeline_generator = mr_util.is_generator_function(stage.run)
    logging.debug('Running %s(*%s, **%s)', stage._class_path,
                  _short_repr(stage.args), _short_repr(stage.kwargs))

    if stage.async:
      stage.run_test(*stage.args, **stage.kwargs)
    elif pipeline_generator:
      all_output_slots = set()
      try:
        pipeline_iter = stage.run_test(*stage.args, **stage.kwargs)
      except NotImplementedError:
        pipeline_iter = stage.run(*stage.args, **stage.kwargs)

      all_substages = set()
      next_value = None
      last_sub_stage = None
      while True:
        try:
          yielded = pipeline_iter.send(next_value)
        except StopIteration:
          break

        if isinstance(yielded, Pipeline):
          if yielded in all_substages:
            raise UnexpectedPipelineError(
                'Already yielded pipeline object %r' % yielded)
          else:
            all_substages.add(yielded)

          last_sub_stage = yielded
          next_value = yielded.outputs
          all_output_slots.update(next_value._output_dict.itervalues())
        else:
          raise UnexpectedPipelineError(
              'Yielded a disallowed value: %r' % yielded)

      if last_sub_stage:
        # Generator's outputs inherited from last running sub-stage.
        # If the generator changes its mind and doesn't yield anything, this
        # may not happen at all. Missing outputs will be caught when they
        # are passed to the stage as inputs, or verified from the outside by
        # the test runner.
        for slot_name, slot in last_sub_stage.outputs._output_dict.iteritems():
          stage.outputs._output_dict[slot_name] = slot
          # Any inherited slots won't be checked for declaration.
          all_output_slots.remove(slot)
      else:
        # Generator yielded no children, so treat it as a sync function.
        stage.outputs.default._set_value_test(stage._pipeline_key, None)

      # Enforce the policy of requiring all undeclared output slots from
      # child pipelines to be consumed by their parent generator.
      for slot in all_output_slots:
        if slot.name == 'default':
          continue
        if slot.filled and not slot._strict and not slot._touched:
          raise SlotNotDeclaredError(
              'Undeclared output "%s"; all dynamic outputs from child '
              'pipelines must be consumed.' % slot.name)
    else:
      try:
        result = stage.run_test(*stage.args, **stage.kwargs)
      except NotImplementedError:
        result = stage.run(*stage.args, **stage.kwargs)
      stage.outputs.default._set_value_test(stage._pipeline_key, result)

    # Enforce strict output usage at the top level.
    if root:
      found_outputs = set()
      for slot in stage.outputs._output_dict.itervalues():
        if slot.filled:
          found_outputs.add(slot.name)
        if slot.name == 'default':
          continue
        if slot.name not in stage.output_names:
          raise SlotNotDeclaredError(
              'Undeclared output from root pipeline "%s"' % slot.name)

      missing_outputs = set(stage.output_names) - found_outputs
      if missing_outputs:
        raise SlotNotFilledError(
            'Outputs %r were never filled.' % missing_outputs)

    logging.debug('Finalizing %s(*%s, **%s)', stage._class_path,
                  _short_repr(stage.args), _short_repr(stage.kwargs))
    ran = False
    try:
      stage.finalized_test()
      ran = True
    except NotImplementedError:
      pass
    if not ran:
      try:
        stage.finalized()
      except NotImplementedError:
        pass

  def evaluate(self, pipeline_key, purpose=None, attempt=0):
    """Evaluates the given Pipeline and enqueues sub-stages for execution.

    Args:
      pipeline_key: The db.Key or stringified key of the _PipelineRecord to run.
      purpose: Why evaluate was called ('start', 'finalize', or 'abort').
      attempt: The attempt number that should be tried.
    """
    After._thread_init()
    InOrder._thread_init()
    InOrder._local._activated = False

    if not isinstance(pipeline_key, db.Key):
      pipeline_key = db.Key(pipeline_key)
    pipeline_record = db.get(pipeline_key)
    if pipeline_record is None:
      logging.error('Pipeline ID "%s" does not exist.', pipeline_key.name())
      return
    if pipeline_record.status not in (
        _PipelineRecord.WAITING, _PipelineRecord.RUN):
      logging.error('Pipeline ID "%s" in bad state for purpose "%s": "%s"',
                    pipeline_key.name(), purpose or _BarrierRecord.START,
                    pipeline_record.status)
      return

    params = pipeline_record.params
    root_pipeline_key = \
        _PipelineRecord.root_pipeline.get_value_for_datastore(pipeline_record)
    default_slot_key = db.Key(params['output_slots']['default'])

    default_slot_record, root_pipeline_record = db.get([
        default_slot_key, root_pipeline_key])
    if default_slot_record is None:
      logging.error('Pipeline ID "%s" default slot "%s" does not exist.',
                    pipeline_key.name(), default_slot_key)
      return
    if root_pipeline_record is None:
      logging.error('Pipeline ID "%s" root pipeline ID "%s" is missing.',
                    pipeline_key.name(), root_pipeline_key.name())
      return

    # Always finalize if we're aborting so pipelines have a chance to cleanup
    # before they terminate. Pipelines must access 'was_aborted' to find
    # out how their finalization should work.
    abort_signal = (
        purpose == _BarrierRecord.ABORT or
        root_pipeline_record.abort_requested == True)
    finalize_signal = (
        (default_slot_record.status == _SlotRecord.FILLED and
         purpose == _BarrierRecord.FINALIZE) or abort_signal)

    try:
      pipeline_func_class = mr_util.for_name(pipeline_record.class_path)
    except ImportError, e:
      # This means something is wrong with the deployed code. Rely on the
      # taskqueue system to do retries.
      retry_message = '%s: %s' % (e.__class__.__name__, str(e))
      logging.exception(
          'Could not locate %s#%s. %s',
          pipeline_record.class_path, pipeline_key.name(), retry_message)
      raise

    try:
      pipeline_func = pipeline_func_class.from_id(
          pipeline_key.name(),
          resolve_outputs=finalize_signal,
          _pipeline_record=pipeline_record)
    except SlotNotFilledError, e:
      logging.exception(
          'Could not resolve arguments for %s#%s. Most likely this means there '
          'is a bug in the Pipeline runtime or some intermediate data has been '
          'deleted from the Datastore. Giving up.',
          pipeline_record.class_path, pipeline_key.name())
      self.transition_aborted(pipeline_key)
      return
    except Exception, e:
      retry_message = '%s: %s' % (e.__class__.__name__, str(e))
      logging.exception(
          'Instantiating %s#%s raised exception. %s',
          pipeline_record.class_path, pipeline_key.name(), retry_message)
      self.transition_retry(pipeline_key, retry_message)
      if pipeline_record.params['task_retry']:
        raise
      else:
        return
    else:
      pipeline_generator = mr_util.is_generator_function(
          pipeline_func_class.run)
      caller_output = pipeline_func.outputs

    if (abort_signal and pipeline_func.async and
        pipeline_record.status == _PipelineRecord.RUN
        and not pipeline_func.try_cancel()):
      logging.warning(
          'Could not cancel and abort mid-flight async pipeline: %r#%s',
          pipeline_func, pipeline_key.name())
      return

    if finalize_signal:
      try:
        pipeline_func._finalized_internal(
              self, pipeline_key, root_pipeline_key,
              caller_output, abort_signal)
      except Exception, e:
        # This means something is wrong with the deployed finalization code.
        # Rely on the taskqueue system to do retries.
        retry_message = '%s: %s' % (e.__class__.__name__, str(e))
        logging.exception('Finalizing %r#%s raised exception. %s',
                          pipeline_func, pipeline_key.name(), retry_message)
        raise
      else:
        if not abort_signal:
          self.transition_complete(pipeline_key)
          return

    if abort_signal:
      logging.debug('Marking as aborted %s#%s', pipeline_func,
                    pipeline_key.name())
      self.transition_aborted(pipeline_key)
      return

    if pipeline_record.current_attempt != attempt:
      logging.error(
          'Received evaluation task for pipeline ID "%s" attempt %d but '
          'current pending attempt is %d', pipeline_key.name(), attempt,
          pipeline_record.current_attempt)
      return

    if pipeline_record.current_attempt >= pipeline_record.max_attempts:
      logging.error(
          'Received evaluation task for pipeline ID "%s" on attempt %d '
          'but that exceeds max attempts %d', pipeline_key.name(), attempt,
          pipeline_record.max_attempts)
      return

    if pipeline_record.next_retry_time is not None:
      retry_time = pipeline_record.next_retry_time - _RETRY_WIGGLE_TIMEDELTA
      if self._gettime() <= retry_time:
        detail_message = (
            'Received evaluation task for pipeline ID "%s" on attempt %d, '
            'which will not be ready until: %s' % (pipeline_key.name(),
            pipeline_record.current_attempt, pipeline_record.next_retry_time))
        logging.warning(detail_message)
        raise UnexpectedPipelineError(detail_message)

    if pipeline_record.status == _PipelineRecord.RUN and pipeline_generator:
      if (default_slot_record.status == _SlotRecord.WAITING and
          not pipeline_record.fanned_out):
        # This properly handles the yield-less generator case when the
        # RUN state transition worked properly but outputting to the default
        # slot failed.
        self.fill_slot(pipeline_key, caller_output.default, None)
      return

    if (pipeline_record.status == _PipelineRecord.WAITING and
        pipeline_func.async):
      self.transition_run(pipeline_key)

    try:
      result = pipeline_func._run_internal(
          self, pipeline_key, root_pipeline_key, caller_output)
    except Exception, e:
      if self.handle_run_exception(pipeline_key, pipeline_func, e):
        raise
      else:
        return

    if pipeline_func.async:
      return

    if not pipeline_generator:
      # Catch any exceptions that are thrown when the pipeline's return
      # value is being serialized. This ensures that serialization errors
      # will cause normal abort/retry behavior.
      try:
        self.fill_slot(pipeline_key, caller_output.default, result)
      except Exception, e:
        retry_message = 'Bad return value. %s: %s' % (
            e.__class__.__name__, str(e))
        logging.exception(
            'Generator %r#%s caused exception while serializing return '
            'value %r. %s', pipeline_func, pipeline_key.name(), result,
            retry_message)
        self.transition_retry(pipeline_key, retry_message)
        if pipeline_func.task_retry:
          raise
        else:
          return

      expected_outputs = set(caller_output._output_dict.iterkeys())
      found_outputs = self.session_filled_output_names
      if expected_outputs != found_outputs:
        exception = SlotNotFilledError(
            'Outputs %r for pipeline ID "%s" were never filled by "%s".' % (
            expected_outputs - found_outputs,
            pipeline_key.name(), pipeline_func._class_path))
        if self.handle_run_exception(pipeline_key, pipeline_func, exception):
          raise exception
      return

    pipeline_iter = result
    next_value = None
    last_sub_stage = None
    sub_stage = None
    sub_stage_dict = {}
    sub_stage_ordering = []

    while True:
      try:
        yielded = pipeline_iter.send(next_value)
      except StopIteration:
        break
      except Exception, e:
        if self.handle_run_exception(pipeline_key, pipeline_func, e):
          raise
        else:
          return

      if isinstance(yielded, Pipeline):
        if yielded in sub_stage_dict:
          raise UnexpectedPipelineError(
              'Already yielded pipeline object %r with pipeline ID %s' %
              (yielded, yielded.pipeline_id))

        last_sub_stage = yielded
        next_value = PipelineFuture(yielded.output_names)
        next_value._after_all_pipelines.update(After._local._after_all_futures)
        next_value._after_all_pipelines.update(InOrder._local._in_order_futures)
        sub_stage_dict[yielded] = next_value
        sub_stage_ordering.append(yielded)
        InOrder._add_future(next_value)

        # To aid local testing, the task_retry flag (which instructs the
        # evaluator to raise all exceptions back up to the task queue) is
        # inherited by all children from the root down.
        yielded.task_retry = pipeline_func.task_retry
      else:
        raise UnexpectedPipelineError(
            'Yielded a disallowed value: %r' % yielded)

    if last_sub_stage:
      # Final yielded stage inherits outputs from calling pipeline that were not
      # already filled during the generator's execution.
      inherited_outputs = params['output_slots']
      for slot_name in self.session_filled_output_names:
        del inherited_outputs[slot_name]
      sub_stage_dict[last_sub_stage]._inherit_outputs(
          pipeline_record.class_path, inherited_outputs)
    else:
      # Here the generator has yielded nothing, and thus acts as a synchronous
      # function. We can skip the rest of the generator steps completely and
      # fill the default output slot to cause finalizing.
      expected_outputs = set(caller_output._output_dict.iterkeys())
      expected_outputs.remove('default')
      found_outputs = self.session_filled_output_names
      if expected_outputs != found_outputs:
        exception = SlotNotFilledError(
            'Outputs %r for pipeline ID "%s" were never filled by "%s".' % (
            expected_outputs - found_outputs,
            pipeline_key.name(), pipeline_func._class_path))
        if self.handle_run_exception(pipeline_key, pipeline_func, exception):
          raise exception
      else:
        self.fill_slot(pipeline_key, caller_output.default, None)
        self.transition_run(pipeline_key)
      return

    # Allocate any SlotRecords that do not yet exist.
    entities_to_put = []
    for future in sub_stage_dict.itervalues():
      for slot in future._output_dict.itervalues():
        if not slot._exists:
          entities_to_put.append(_SlotRecord(
              key=slot.key, root_pipeline=root_pipeline_key))

    # Allocate PipelineRecords and BarrierRecords for generator-run Pipelines.
    pipelines_to_run = set()
    all_children_keys = []
    all_output_slots = set()
    for sub_stage in sub_stage_ordering:
      future = sub_stage_dict[sub_stage]

      # Catch any exceptions that are thrown when the pipeline's parameters
      # are being serialized. This ensures that serialization errors will
      # cause normal retry/abort behavior.
      try:
        dependent_slots, output_slots, params_text, params_blob = \
            _generate_args(sub_stage, future, self.queue_name, self.base_path)
      except Exception, e:
        retry_message = 'Bad child arguments. %s: %s' % (
            e.__class__.__name__, str(e))
        logging.exception(
            'Generator %r#%s caused exception while serializing args for '
            'child pipeline %r. %s', pipeline_func, pipeline_key.name(),
            sub_stage, retry_message)
        self.transition_retry(pipeline_key, retry_message)
        if pipeline_func.task_retry:
          raise
        else:
          return

      child_pipeline_key = db.Key.from_path(
          _PipelineRecord.kind(), uuid.uuid4().hex)
      all_output_slots.update(output_slots)
      all_children_keys.append(child_pipeline_key)

      child_pipeline = _PipelineRecord(
          key=child_pipeline_key,
          root_pipeline=root_pipeline_key,
          # Bug in DB means we need to use the storage name here,
          # not the local property name.
          params=params_text,
          params_blob=params_blob,
          class_path=sub_stage._class_path,
          max_attempts=sub_stage.max_attempts)
      entities_to_put.append(child_pipeline)

      if not dependent_slots:
        # This child pipeline will run immediately.
        pipelines_to_run.add(child_pipeline_key)
        child_pipeline.start_time = self._gettime()
      else:
        entities_to_put.extend(_PipelineContext._create_barrier_entities(
            root_pipeline_key,
            child_pipeline_key,
            _BarrierRecord.START,
            dependent_slots))

      entities_to_put.extend(_PipelineContext._create_barrier_entities(
          root_pipeline_key,
          child_pipeline_key,
          _BarrierRecord.FINALIZE,
          output_slots))

    # This generator pipeline's finalization barrier must include all of the
    # outputs of any child pipelines that it runs. This ensures the finalized
    # calls will not happen until all child pipelines have completed.
    #
    # The transition_run() call below will update the FINALIZE _BarrierRecord
    # for this generator pipeline to include all of these child outputs in
    # its list of blocking_slots. That update is done transactionally to
    # make sure the _BarrierRecord only lists the slots that matter.
    #
    # However, the notify_barriers() method doesn't find _BarrierRecords
    # through the blocking_slots field. It finds them through _BarrierIndexes
    # entities. Thus, before we update the FINALIZE _BarrierRecord in
    # transition_run(), we need to write _BarrierIndexes for all child outputs.
    barrier_entities = _PipelineContext._create_barrier_entities(
        root_pipeline_key,
        pipeline_key,
        _BarrierRecord.FINALIZE,
        all_output_slots)
    # Ignore the first element which is the _BarrierRecord. That entity must
    # have already been created and put in the datastore for the parent
    # pipeline before this code generated child pipelines.
    barrier_indexes = barrier_entities[1:]
    entities_to_put.extend(barrier_indexes)

    db.put(entities_to_put)

    self.transition_run(pipeline_key,
                        blocking_slot_keys=all_output_slots,
                        fanned_out_pipelines=all_children_keys,
                        pipelines_to_run=pipelines_to_run)

  @staticmethod
  def _create_barrier_entities(root_pipeline_key,
                               child_pipeline_key,
                               purpose,
                               blocking_slot_keys):
    """Creates all of the entities required for a _BarrierRecord.

    Args:
      root_pipeline_key: The root pipeline this is part of.
      child_pipeline_key: The pipeline this barrier is for.
      purpose: _BarrierRecord.START or _BarrierRecord.FINALIZE.
      blocking_slot_keys: Set of db.Keys corresponding to _SlotRecords that
        this barrier should wait on before firing.

    Returns:
      List of entities, starting with the _BarrierRecord entity, followed by
      _BarrierIndexes used for firing when _SlotRecords are filled in the same
      order as the blocking_slot_keys list provided. All of these entities
      should be put in the Datastore to ensure the barrier fires properly.
    """
    result = []

    blocking_slot_keys = list(blocking_slot_keys)

    barrier = _BarrierRecord(
        parent=child_pipeline_key,
        key_name=purpose,
        target=child_pipeline_key,
        root_pipeline=root_pipeline_key,
        blocking_slots=blocking_slot_keys)

    result.append(barrier)

    for slot_key in blocking_slot_keys:
      barrier_index_path = []
      barrier_index_path.extend(slot_key.to_path())
      barrier_index_path.extend(child_pipeline_key.to_path())
      barrier_index_path.extend([_BarrierIndex.kind(), purpose])
      barrier_index_key = db.Key.from_path(*barrier_index_path)
      barrier_index = _BarrierIndex(
          key=barrier_index_key,
          root_pipeline=root_pipeline_key)
      result.append(barrier_index)

    return result

  def handle_run_exception(self, pipeline_key, pipeline_func, e):
    """Handles an exception raised by a Pipeline's user code.

    Args:
      pipeline_key: The pipeline that raised the error.
      pipeline_func: The class path name of the Pipeline that was running.
      e: The exception that was raised.

    Returns:
      True if the exception should be re-raised up through the calling stack
      by the caller of this method.
    """
    if isinstance(e, Retry):
      retry_message = str(e)
      logging.warning('User forced retry for pipeline ID "%s" of %r: %s',
                      pipeline_key.name(), pipeline_func, retry_message)
      self.transition_retry(pipeline_key, retry_message)
    elif isinstance(e, Abort):
      abort_message = str(e)
      logging.warning('User forced abort for pipeline ID "%s" of %r: %s',
                      pipeline_key.name(), pipeline_func, abort_message)
      pipeline_func.abort(abort_message)
    else:
      retry_message = '%s: %s' % (e.__class__.__name__, str(e))
      logging.exception('Generator %r#%s raised exception. %s',
                        pipeline_func, pipeline_key.name(), retry_message)
      self.transition_retry(pipeline_key, retry_message)

    return pipeline_func.task_retry

  def transition_run(self,
                     pipeline_key,
                     blocking_slot_keys=None,
                     fanned_out_pipelines=None,
                     pipelines_to_run=None):
    """Marks an asynchronous or generator pipeline as running.

    Does nothing if the pipeline is no longer in a runnable state.

    Args:
      pipeline_key: The db.Key of the _PipelineRecord to update.
      blocking_slot_keys: List of db.Key instances that this pipeline's
        finalization barrier should wait on in addition to the existing one.
        This is used to update the barrier to include all child outputs. When
        None, the barrier will not be updated.
      fanned_out_pipelines: List of db.Key instances of _PipelineRecords that
        were fanned out by this generator pipeline. This is distinct from the
        'pipelines_to_run' list because not all of the pipelines listed here
        will be immediately ready to execute. When None, then this generator
        yielded no children.
      pipelines_to_run: List of db.Key instances of _PipelineRecords that should
        be kicked off (fan-out) transactionally as part of this transition.
        When None, no child pipelines will run. All db.Keys in this list must
        also be present in the fanned_out_pipelines list.

    Raises:
      UnexpectedPipelineError if blocking_slot_keys was not empty and the
      _BarrierRecord has gone missing.
    """
    def txn():
      pipeline_record = db.get(pipeline_key)
      if pipeline_record is None:
        logging.warning('Pipeline ID "%s" cannot be marked as run. '
                        'Does not exist.', pipeline_key.name())
        raise db.Rollback()
      if pipeline_record.status != _PipelineRecord.WAITING:
        logging.warning('Pipeline ID "%s" in bad state to be marked as run: %s',
                        pipeline_key.name(), pipeline_record.status)
        raise db.Rollback()

      pipeline_record.status = _PipelineRecord.RUN

      if fanned_out_pipelines:
        # NOTE: We must model the pipeline relationship in a top-down manner,
        # meaning each pipeline must point forward to the pipelines that it
        # fanned out to. The reason is race conditions. If evaluate()
        # dies early, it may create many unused _PipelineRecord and _SlotRecord
        # instances that never progress. The only way we know which of these
        # are valid is by traversing the graph from the root, where the
        # fanned_out property refers to those pipelines that were run using a
        # transactional task.
        child_pipeline_list = list(fanned_out_pipelines)
        pipeline_record.fanned_out = child_pipeline_list

        if pipelines_to_run:
          child_indexes = [
              child_pipeline_list.index(p) for p in pipelines_to_run]
          child_indexes.sort()
          task = taskqueue.Task(
              url=self.fanout_handler_path,
              params=dict(parent_key=str(pipeline_key),
                          child_indexes=child_indexes))
          task.add(queue_name=self.queue_name, transactional=True)

      pipeline_record.put()

      if blocking_slot_keys:
        # NOTE: Always update a generator pipeline's finalization barrier to
        # include all of the outputs of any pipelines that it runs, to ensure
        # that finalized calls will not happen until all child pipelines have
        # completed. This must happen transactionally with the enqueue of
        # the fan-out kickoff task above to ensure the child output slots and
        # the barrier blocking slots are the same.
        barrier_key = db.Key.from_path(
            _BarrierRecord.kind(), _BarrierRecord.FINALIZE,
            parent=pipeline_key)
        finalize_barrier = db.get(barrier_key)
        if finalize_barrier is None:
          raise UnexpectedPipelineError(
              'Pipeline ID "%s" cannot update finalize barrier. '
              'Does not exist.' % pipeline_key.name())
        else:
          finalize_barrier.blocking_slots = list(
              blocking_slot_keys.union(set(finalize_barrier.blocking_slots)))
          finalize_barrier.put()

    db.run_in_transaction(txn)

  def transition_complete(self, pipeline_key):
    """Marks the given pipeline as complete.

    Does nothing if the pipeline is no longer in a state that can be completed.

    Args:
      pipeline_key: db.Key of the _PipelineRecord that has completed.
    """
    def txn():
      pipeline_record = db.get(pipeline_key)
      if pipeline_record is None:
        logging.warning(
            'Tried to mark pipeline ID "%s" as complete but it does not exist.',
            pipeline_key.name())
        raise db.Rollback()
      if pipeline_record.status not in (
          _PipelineRecord.WAITING, _PipelineRecord.RUN):
        logging.warning(
            'Tried to mark pipeline ID "%s" as complete, found bad state: %s',
            pipeline_key.name(), pipeline_record.status)
        raise db.Rollback()

      pipeline_record.status = _PipelineRecord.DONE
      pipeline_record.finalized_time = self._gettime()
      pipeline_record.put()

    db.run_in_transaction(txn)

  def transition_retry(self, pipeline_key, retry_message):
    """Marks the given pipeline as requiring another retry.

    Does nothing if all attempts have been exceeded.

    Args:
      pipeline_key: db.Key of the _PipelineRecord that needs to be retried.
      retry_message: User-supplied message indicating the reason for the retry.
    """
    def txn():
      pipeline_record = db.get(pipeline_key)
      if pipeline_record is None:
        logging.warning(
            'Tried to retry pipeline ID "%s" but it does not exist.',
            pipeline_key.name())
        raise db.Rollback()
      if pipeline_record.status not in (
          _PipelineRecord.WAITING, _PipelineRecord.RUN):
        logging.warning(
            'Tried to retry pipeline ID "%s", found bad state: %s',
            pipeline_key.name(), pipeline_record.status)
        raise db.Rollback()

      params = pipeline_record.params
      offset_seconds = (
          params['backoff_seconds'] *
          (params['backoff_factor'] ** pipeline_record.current_attempt))
      pipeline_record.next_retry_time = (
          self._gettime() + datetime.timedelta(seconds=offset_seconds))
      pipeline_record.current_attempt += 1
      pipeline_record.retry_message = retry_message
      pipeline_record.status = _PipelineRecord.WAITING

      if pipeline_record.current_attempt >= pipeline_record.max_attempts:
        root_pipeline_key = (
            _PipelineRecord.root_pipeline.get_value_for_datastore(
                pipeline_record))
        logging.warning(
            'Giving up on pipeline ID "%s" after %d attempt(s); causing abort '
            'all the way to the root pipeline ID "%s"', pipeline_key.name(),
            pipeline_record.current_attempt, root_pipeline_key.name())
        # NOTE: We do *not* set the status to aborted here to ensure that
        # this pipeline will be finalized before it has been marked as aborted.
        pipeline_record.abort_message = (
            'Aborting after %d attempts' % pipeline_record.current_attempt)
        task = taskqueue.Task(
            url=self.fanout_abort_handler_path,
            params=dict(root_pipeline_key=root_pipeline_key))
        task.add(queue_name=self.queue_name, transactional=True)
      else:
        task = taskqueue.Task(
            url=self.pipeline_handler_path,
            eta=pipeline_record.next_retry_time,
            params=dict(pipeline_key=pipeline_key,
                        purpose=_BarrierRecord.START,
                        attempt=pipeline_record.current_attempt),
            headers={'X-Ae-Pipeline-Key': pipeline_key})
        task.add(queue_name=self.queue_name, transactional=True)

      pipeline_record.put()

    db.run_in_transaction(txn)

  def transition_aborted(self, pipeline_key):
    """Makes the given pipeline as having aborted.

    Does nothing if the pipeline is in a bad state.

    Args:
      pipeline_key: db.Key of the _PipelineRecord that needs to be retried.
    """
    def txn():
      pipeline_record = db.get(pipeline_key)
      if pipeline_record is None:
        logging.warning(
            'Tried to abort pipeline ID "%s" but it does not exist.',
            pipeline_key.name())
        raise db.Rollback()
      if pipeline_record.status not in (
          _PipelineRecord.WAITING, _PipelineRecord.RUN):
        logging.warning(
            'Tried to abort pipeline ID "%s", found bad state: %s',
            pipeline_key.name(), pipeline_record.status)
        raise db.Rollback()

      pipeline_record.status = _PipelineRecord.ABORTED
      pipeline_record.finalized_time = self._gettime()
      pipeline_record.put()

    db.run_in_transaction(txn)

################################################################################


class _BarrierHandler(webapp.RequestHandler):
  """Request handler for triggering barriers."""

  def post(self):
    if 'HTTP_X_APPENGINE_TASKNAME' not in self.request.environ:
      self.response.set_status(403)
      return

    context = _PipelineContext.from_environ(self.request.environ)
    context.notify_barriers(
        self.request.get('slot_key'),
        self.request.get('cursor'),
        use_barrier_indexes=self.request.get('use_barrier_indexes') == 'True')


class _PipelineHandler(webapp.RequestHandler):
  """Request handler for running pipelines."""

  def post(self):
    if 'HTTP_X_APPENGINE_TASKNAME' not in self.request.environ:
      self.response.set_status(403)
      return

    context = _PipelineContext.from_environ(self.request.environ)
    context.evaluate(self.request.get('pipeline_key'),
                     purpose=self.request.get('purpose'),
                     attempt=int(self.request.get('attempt', '0')))


class _FanoutAbortHandler(webapp.RequestHandler):
  """Request handler for fanning out abort notifications."""

  def post(self):
    if 'HTTP_X_APPENGINE_TASKNAME' not in self.request.environ:
      self.response.set_status(403)
      return

    context = _PipelineContext.from_environ(self.request.environ)
    context.continue_abort(
        self.request.get('root_pipeline_key'),
        self.request.get('cursor'))


class _FanoutHandler(webapp.RequestHandler):
  """Request handler for fanning out pipeline children."""

  def post(self):
    if 'HTTP_X_APPENGINE_TASKNAME' not in self.request.environ:
      self.response.set_status(403)
      return

    context = _PipelineContext.from_environ(self.request.environ)

    # Set of stringified db.Keys of children to run.
    all_pipeline_keys = set()

    # For backwards compatibility with the old style of fan-out requests.
    all_pipeline_keys.update(self.request.get_all('pipeline_key'))

    # Fetch the child pipelines from the parent. This works around the 10KB
    # task payload limit. This get() is consistent-on-read and the fan-out
    # task is enqueued in the transaction that updates the parent, so the
    # fanned_out property is consistent here.
    parent_key = self.request.get('parent_key')
    child_indexes = [int(x) for x in self.request.get_all('child_indexes')]
    if parent_key:
      parent_key = db.Key(parent_key)
      parent = db.get(parent_key)
      for index in child_indexes:
        all_pipeline_keys.add(str(parent.fanned_out[index]))

    all_tasks = []
    all_pipelines = db.get([db.Key(pipeline_key) for pipeline_key in all_pipeline_keys])
    for child_pipeline in all_pipelines:
      if child_pipeline is None:
        continue
      pipeline_key = str(child_pipeline.key())
      all_tasks.append(taskqueue.Task(
          url=context.pipeline_handler_path,
          params=dict(pipeline_key=pipeline_key),
          target=child_pipeline.params['target'],
          headers={'X-Ae-Pipeline-Key': pipeline_key},
          name='ae-pipeline-fan-out-' + child_pipeline.key().name()))

    batch_size = 100  # Limit of taskqueue API bulk add.
    for i in xrange(0, len(all_tasks), batch_size):
      batch = all_tasks[i:i+batch_size]
      try:
        taskqueue.Queue(context.queue_name).add(batch)
      except (taskqueue.TombstonedTaskError, taskqueue.TaskAlreadyExistsError):
        pass


class _CleanupHandler(webapp.RequestHandler):
  """Request handler for cleaning up a Pipeline."""

  def post(self):
    if 'HTTP_X_APPENGINE_TASKNAME' not in self.request.environ:
      self.response.set_status(403)
      return

    root_pipeline_key = db.Key(self.request.get('root_pipeline_key'))
    logging.debug('Cleaning up root_pipeline_key=%r', root_pipeline_key)

    # TODO(user): Accumulate all BlobKeys from _PipelineRecord and
    # _SlotRecord entities and delete them.
    pipeline_keys = (
        _PipelineRecord.all(keys_only=True)
        .filter('root_pipeline =', root_pipeline_key))
    db.delete(pipeline_keys)
    slot_keys = (
        _SlotRecord.all(keys_only=True)
        .filter('root_pipeline =', root_pipeline_key))
    db.delete(slot_keys)
    barrier_keys = (
        _BarrierRecord.all(keys_only=True)
        .filter('root_pipeline =', root_pipeline_key))
    db.delete(barrier_keys)
    status_keys = (
        _StatusRecord.all(keys_only=True)
        .filter('root_pipeline =', root_pipeline_key))
    db.delete(status_keys)
    barrier_index_keys = (
        _BarrierIndex.all(keys_only=True)
        .filter('root_pipeline =', root_pipeline_key))
    db.delete(barrier_index_keys)


class _CallbackHandler(webapp.RequestHandler):
  """Receives asynchronous callback requests from humans or tasks."""

  def post(self):
    self.get()

  def get(self):
    try:
      self.run_callback()
    except _CallbackTaskError, e:
      logging.error(str(e))
      if 'HTTP_X_APPENGINE_TASKRETRYCOUNT' in self.request.environ:
        # Silently give up on tasks that have retried many times. This
        # probably means that the target pipeline has been deleted, so there's
        # no reason to keep trying this task forever.
        retry_count = int(
            self.request.environ.get('HTTP_X_APPENGINE_TASKRETRYCOUNT'))
        if retry_count > _MAX_CALLBACK_TASK_RETRIES:
          logging.error('Giving up on task after %d retries',
                        _MAX_CALLBACK_TASK_RETRIES)
          return

      # NOTE: The undescriptive error code 400 are present to address security
      # risks of giving external users access to cause PipelineRecord lookups
      # and execution.
      self.response.set_status(400)

  def run_callback(self):
    """Runs the callback for the pipeline specified in the request.

    Raises:
      _CallbackTaskError if something was wrong with the request parameters.
    """
    pipeline_id = self.request.get('pipeline_id')
    if not pipeline_id:
      raise _CallbackTaskError('"pipeline_id" parameter missing.')

    pipeline_key = db.Key.from_path(_PipelineRecord.kind(), pipeline_id)
    pipeline_record = db.get(pipeline_key)
    if pipeline_record is None:
      raise _CallbackTaskError(
          'Pipeline ID "%s" for callback does not exist.' % pipeline_id)

    params = pipeline_record.params
    real_class_path = params['class_path']
    try:
      pipeline_func_class = mr_util.for_name(real_class_path)
    except ImportError, e:
      raise _CallbackTaskError(
          'Cannot load class named "%s" for pipeline ID "%s".'
          % (real_class_path, pipeline_id))

    if 'HTTP_X_APPENGINE_TASKNAME' not in self.request.environ:
      if pipeline_func_class.public_callbacks:
        pass
      elif pipeline_func_class.admin_callbacks:
        if not users.is_current_user_admin():
          raise _CallbackTaskError(
              'Unauthorized callback for admin-only pipeline ID "%s"'
              % pipeline_id)
      else:
        raise _CallbackTaskError(
            'External callback for internal-only pipeline ID "%s"'
            % pipeline_id)

    kwargs = {}
    for key in self.request.arguments():
      if key != 'pipeline_id':
        kwargs[str(key)] = self.request.get(key)

    def perform_callback():
      stage = pipeline_func_class.from_id(pipeline_id)
      if stage is None:
        raise _CallbackTaskError(
            'Pipeline ID "%s" deleted during callback' % pipeline_id)
      return stage._callback_internal(kwargs)

    # callback_xg_transaction is a 3-valued setting (None=no trans,
    # False=1-eg-trans, True=xg-trans)
    if pipeline_func_class._callback_xg_transaction is not None:
      transaction_options = db.create_transaction_options(
          xg=pipeline_func_class._callback_xg_transaction)
      callback_result = db.run_in_transaction_options(transaction_options,
                                                      perform_callback)
    else:
      callback_result = perform_callback()

    if callback_result is not None:
      status_code, content_type, content = callback_result
      self.response.set_status(status_code)
      self.response.headers['Content-Type'] = content_type
      self.response.out.write(content)


################################################################################

def _get_timestamp_ms(when):
  """Converts a datetime.datetime to integer milliseconds since the epoch.

  Requires special handling to preserve microseconds.

  Args:
    when: A datetime.datetime instance.

  Returns:
    Integer time since the epoch in milliseconds. If the supplied 'when' is
    None, the return value will be None.
  """
  if when is None:
    return None
  ms_since_epoch = float(time.mktime(when.utctimetuple()) * 1000.0)
  ms_since_epoch += when.microsecond / 1000.0
  return int(ms_since_epoch)


def _get_internal_status(pipeline_key=None,
                         pipeline_dict=None,
                         slot_dict=None,
                         barrier_dict=None,
                         status_dict=None):
  """Gets the UI dictionary of a pipeline from a set of status dictionaries.

  Args:
    pipeline_key: The key of the pipeline to lookup.
    pipeline_dict: Dictionary mapping pipeline db.Key to _PipelineRecord.
      Default is an empty dictionary.
    slot_dict: Dictionary mapping slot db.Key to _SlotRecord.
      Default is an empty dictionary.
    barrier_dict: Dictionary mapping barrier db.Key to _BarrierRecord.
      Default is an empty dictionary.
    status_dict: Dictionary mapping status record db.Key to _StatusRecord.
      Default is an empty dictionary.

  Returns:
    Dictionary with the keys:
      classPath: The pipeline function being run.
      args: List of positional argument slot dictionaries.
      kwargs: Dictionary of keyword argument slot dictionaries.
      outputs: Dictionary of output slot dictionaries.
      children: List of child pipeline IDs.
      queueName: Queue on which this pipeline is running.
      afterSlotKeys: List of Slot Ids after which this pipeline runs.
      currentAttempt: Number of the current attempt, starting at 1.
      maxAttempts: Maximum number of attempts before aborting.
      backoffSeconds: Constant factor for backoff before retrying.
      backoffFactor: Exponential factor for backoff before retrying.
      status: Current status of the pipeline.
      startTimeMs: When this pipeline ran or will run due to retries, if present.
      endTimeMs: When this pipeline finalized, if present.
      lastRetryMessage: Why the pipeline failed during the last retry, if there
        was a failure; may be empty.
      abortMessage: For root pipelines, why the pipeline was aborted if it was
        aborted; may be empty.

    Dictionary will contain these keys if explicit status is set:
      statusTimeMs: When the status was set as milliseconds since the epoch.
      statusMessage: Status message, if present.
      statusConsoleUrl: The relative URL for the console of this pipeline.
      statusLinks: Dictionary mapping human-readable names to relative URLs
        for related URLs to this pipeline.

  Raises:
    PipelineStatusError if any input is bad.
  """
  if pipeline_dict is None:
    pipeline_dict = {}
  if slot_dict is None:
    slot_dict = {}
  if barrier_dict is None:
    barrier_dict = {}
  if status_dict is None:
    status_dict = {}

  pipeline_record = pipeline_dict.get(pipeline_key)
  if pipeline_record is None:
    raise PipelineStatusError(
        'Could not find pipeline ID "%s"' % pipeline_key.name())

  params = pipeline_record.params
  root_pipeline_key = \
      _PipelineRecord.root_pipeline.get_value_for_datastore(pipeline_record)
  default_slot_key = db.Key(params['output_slots']['default'])
  start_barrier_key = db.Key.from_path(
      _BarrierRecord.kind(), _BarrierRecord.START, parent=pipeline_key)
  finalize_barrier_key = db.Key.from_path(
      _BarrierRecord.kind(), _BarrierRecord.FINALIZE, parent=pipeline_key)
  status_record_key = db.Key.from_path(
      _StatusRecord.kind(), pipeline_key.name())

  start_barrier = barrier_dict.get(start_barrier_key)
  finalize_barrier = barrier_dict.get(finalize_barrier_key)
  default_slot = slot_dict.get(default_slot_key)
  status_record = status_dict.get(status_record_key)
  if finalize_barrier is None:
    raise PipelineStatusError(
        'Finalization barrier missing for pipeline ID "%s"' %
        pipeline_key.name())
  if default_slot is None:
    raise PipelineStatusError(
        'Default output slot with key=%s missing for pipeline ID "%s"' % (
        default_slot_key, pipeline_key.name()))

  output = {
    'classPath': pipeline_record.class_path,
    'args': list(params['args']),
    'kwargs': params['kwargs'].copy(),
    'outputs': params['output_slots'].copy(),
    'children': [key.name() for key in pipeline_record.fanned_out],
    'queueName': params['queue_name'],
    'afterSlotKeys': [str(key) for key in params['after_all']],
    'currentAttempt': pipeline_record.current_attempt + 1,
    'maxAttempts': pipeline_record.max_attempts,
    'backoffSeconds': pipeline_record.params['backoff_seconds'],
    'backoffFactor': pipeline_record.params['backoff_factor'],
  }

  # TODO(user): Truncate args, kwargs, and outputs to < 1MB each so we
  # can reasonably return the whole tree of pipelines and their outputs.
  # Coerce each value to a string to truncate if necessary. For now if the
  # params are too big it will just cause the whole status page to break.

  # Fix the key names in parameters to match JavaScript style.
  for value_dict in itertools.chain(
      output['args'], output['kwargs'].itervalues()):
    if 'slot_key' in value_dict:
      value_dict['slotKey'] = value_dict.pop('slot_key')

  # Figure out the pipeline's status.
  if pipeline_record.status in (_PipelineRecord.WAITING, _PipelineRecord.RUN):
    if default_slot.status == _SlotRecord.FILLED:
      status = 'finalizing'
    elif (pipeline_record.status == _PipelineRecord.WAITING and
          pipeline_record.next_retry_time is not None):
      status = 'retry'
    elif start_barrier and start_barrier.status == _BarrierRecord.WAITING:
      # start_barrier will be missing for root pipelines
      status = 'waiting'
    else:
      status = 'run'
  elif pipeline_record.status == _PipelineRecord.DONE:
    status = 'done'
  elif pipeline_record.status == _PipelineRecord.ABORTED:
    status = 'aborted'

  output['status'] = status

  if status_record:
    output['statusTimeMs'] = _get_timestamp_ms(status_record.status_time)
    if status_record.message:
      output['statusMessage'] = status_record.message
    if status_record.console_url:
      output['statusConsoleUrl'] = status_record.console_url
    if status_record.link_names:
      output['statusLinks'] = dict(
          zip(status_record.link_names, status_record.link_urls))

  # Populate status-depenedent fields.
  if status in ('run', 'finalizing', 'done', 'retry'):
    if pipeline_record.next_retry_time is not None:
      output['startTimeMs'] = _get_timestamp_ms(pipeline_record.next_retry_time)
    elif start_barrier:
      # start_barrier will be missing for root pipelines
      output['startTimeMs'] = _get_timestamp_ms(start_barrier.trigger_time)
    elif pipeline_record.start_time:
      # Assume this pipeline ran immediately upon spawning with no
      # start barrier or it's the root pipeline.
      output['startTimeMs'] = _get_timestamp_ms(pipeline_record.start_time)

  if status in ('finalizing',):
    output['endTimeMs'] = _get_timestamp_ms(default_slot.fill_time)

  if status in ('done',):
    output['endTimeMs'] = _get_timestamp_ms(pipeline_record.finalized_time)

  if pipeline_record.next_retry_time is not None:
    output['lastRetryMessage'] = pipeline_record.retry_message

  if pipeline_record.abort_message:
    output['abortMessage'] = pipeline_record.abort_message

  return output


def _get_internal_slot(slot_key=None,
                       filler_pipeline_key=None,
                       slot_dict=None):
  """Gets information about a _SlotRecord for display in UI.

  Args:
    slot_key: The db.Key of the slot to fetch.
    filler_pipeline_key: In the case the slot has not yet been filled, assume
      that the given db.Key (for a _PipelineRecord) will be the filler of
      the slot in the future.
    slot_dict: The slot JSON dictionary.

  Returns:
    Dictionary with the keys:
      status: Slot status: 'filled' or 'waiting'
      fillTimeMs: Time in milliseconds since the epoch of when it was filled.
      value: The current value of the slot, which is a slot's JSON dictionary.
      fillerPipelineId: The pipeline ID of what stage has or should fill
        this slot.

  Raises:
    PipelineStatusError if any input is bad.
  """
  if slot_dict is None:
    slot_dict = {}

  slot_record = slot_dict.get(slot_key)
  if slot_record is None:
    raise PipelineStatusError(
        'Could not find data for output slot key "%s".' % slot_key)

  output = {}
  if slot_record.status == _SlotRecord.FILLED:
    output['status'] = 'filled'
    output['fillTimeMs'] = _get_timestamp_ms(slot_record.fill_time)
    output['value'] = slot_record.value
    filler_pipeline_key = (
        _SlotRecord.filler.get_value_for_datastore(slot_record))
  else:
    output['status'] = 'waiting'

  if filler_pipeline_key:
    output['fillerPipelineId'] = filler_pipeline_key.name()

  return output


def get_status_tree(root_pipeline_id):
  """Gets the full status tree of a pipeline.

  Args:
    root_pipeline_id: The pipeline ID to get status for.

  Returns:
    Dictionary with the keys:
      rootPipelineId: The ID of the root pipeline.
      slots: Mapping of slot IDs to result of from _get_internal_slot.
      pipelines: Mapping of pipeline IDs to result of _get_internal_status.

  Raises:
    PipelineStatusError if any input is bad.
  """
  root_pipeline_key = db.Key.from_path(_PipelineRecord.kind(), root_pipeline_id)
  root_pipeline_record = db.get(root_pipeline_key)
  if root_pipeline_record is None:
    raise PipelineStatusError(
        'Could not find pipeline ID "%s"' % root_pipeline_id)

  # If the supplied root_pipeline_id is not actually the root pipeline that's
  # okay. We'll find the real root and override the value they passed in.
  actual_root_key = _PipelineRecord.root_pipeline.get_value_for_datastore(
      root_pipeline_record)
  if actual_root_key != root_pipeline_key:
    root_pipeline_key = actual_root_key
    root_pipeline_id = root_pipeline_key.id_or_name()
    root_pipeline_record = db.get(root_pipeline_key)
    if not root_pipeline_record:
      raise PipelineStatusError(
          'Could not find pipeline ID "%s"' % root_pipeline_id)

  # Run all queries asynchronously.
  queries = {}
  for model in (_PipelineRecord, _SlotRecord, _BarrierRecord, _StatusRecord):
    queries[model] = model.all().filter(
        'root_pipeline =', root_pipeline_key).run(batch_size=1000)

  found_pipeline_dict = dict(
      (stage.key(), stage) for stage in queries[_PipelineRecord])
  found_slot_dict = dict(
      (slot.key(), slot) for slot in queries[_SlotRecord])
  found_barrier_dict = dict(
      (barrier.key(), barrier) for barrier in queries[_BarrierRecord])
  found_status_dict = dict(
      (status.key(), status) for status in queries[_StatusRecord])

  # Breadth-first traversal of _PipelineRecord instances by following
  # _PipelineRecord.fanned_out property values.
  valid_pipeline_keys = set([root_pipeline_key])
  slot_filler_dict = {}  # slot_key to pipeline_key
  expand_stack = [root_pipeline_record]
  while expand_stack:
    old_stack = expand_stack
    expand_stack = []
    for pipeline_record in old_stack:
      for child_pipeline_key in pipeline_record.fanned_out:
        # This will let us prune off those pipelines which were allocated in
        # the Datastore but were never run due to mid-flight task failures.
        child_pipeline_record = found_pipeline_dict.get(child_pipeline_key)
        if child_pipeline_record is None:
          raise PipelineStatusError(
              'Pipeline ID "%s" points to child ID "%s" which does not exist.'
              % (pipeline_record.key().name(), child_pipeline_key.name()))
        expand_stack.append(child_pipeline_record)
        valid_pipeline_keys.add(child_pipeline_key)

        # Figure out the deepest pipeline that's responsible for outputting to
        # a particular _SlotRecord, so we can report which pipeline *should*
        # be the filler.
        child_outputs = child_pipeline_record.params['output_slots']
        for output_slot_key in child_outputs.itervalues():
          slot_filler_dict[db.Key(output_slot_key)] = child_pipeline_key

  output = {
    'rootPipelineId': root_pipeline_id,
    'slots': {},
    'pipelines': {},
  }

  for pipeline_key in found_pipeline_dict.keys():
    if pipeline_key not in valid_pipeline_keys:
      continue
    output['pipelines'][pipeline_key.name()] = _get_internal_status(
        pipeline_key=pipeline_key,
        pipeline_dict=found_pipeline_dict,
        slot_dict=found_slot_dict,
        barrier_dict=found_barrier_dict,
        status_dict=found_status_dict)

  for slot_key, filler_pipeline_key in slot_filler_dict.iteritems():
    output['slots'][str(slot_key)] = _get_internal_slot(
        slot_key=slot_key,
        filler_pipeline_key=filler_pipeline_key,
        slot_dict=found_slot_dict)

  return output


def get_pipeline_names():
  """Returns the class paths of all Pipelines defined in alphabetical order."""
  class_path_set = set()
  for cls in _PipelineMeta._all_classes:
      if cls.class_path is not None:
        class_path_set.add(cls.class_path)
  return sorted(class_path_set)


def get_root_list(class_path=None, cursor=None, count=50):
  """Gets a list root Pipelines.

  Args:
    class_path: Optional. If supplied, only return root Pipelines with the
      given class_path. By default all root pipelines are returned.
    cursor: Optional. When supplied, the cursor returned from the last call to
      get_root_list which indicates where to pick up.
    count: How many pipeline returns to return.

  Returns:
    Dictionary with the keys:
      pipelines: The list of Pipeline records in the same format as
        returned by get_status_tree, but with only the roots listed.
      cursor: Cursor to pass back to this function to resume the query. Will
        only be present if there is another page of results.

  Raises:
    PipelineStatusError if any input is bad.
  """
  query = _PipelineRecord.all(cursor=cursor)
  if class_path:
    query.filter('class_path =', class_path)
  query.filter('is_root_pipeline =', True)
  query.order('-start_time')

  root_list = query.fetch(count)

  fetch_list = []
  for pipeline_record in root_list:
    fetch_list.append(db.Key(pipeline_record.params['output_slots']['default']))
    fetch_list.append(db.Key.from_path(
        _BarrierRecord.kind(), _BarrierRecord.FINALIZE,
        parent=pipeline_record.key()))
    fetch_list.append(db.Key.from_path(
        _StatusRecord.kind(), pipeline_record.key().name()))

  pipeline_dict = dict((stage.key(), stage) for stage in root_list)
  slot_dict = {}
  barrier_dict = {}
  status_dict = {}
  for entity in db.get(fetch_list):
    if isinstance(entity, _BarrierRecord):
      barrier_dict[entity.key()] = entity
    elif isinstance(entity, _SlotRecord):
      slot_dict[entity.key()] = entity
    elif isinstance(entity, _StatusRecord):
      status_dict[entity.key()] = entity

  results = []
  for pipeline_record in root_list:
    try:
      output = _get_internal_status(
          pipeline_record.key(),
          pipeline_dict=pipeline_dict,
          slot_dict=slot_dict,
          barrier_dict=barrier_dict,
          status_dict=status_dict)
      output['pipelineId'] = pipeline_record.key().name()
      results.append(output)
    except PipelineStatusError, e:
      output = {'status': e.message}
      output['classPath'] = ''
      output['pipelineId'] = pipeline_record.key().name()
      results.append(output)

  result_dict = {}
  cursor = query.cursor()
  query.with_cursor(cursor)
  if query.get(keys_only=True):
    result_dict.update(cursor=cursor)
  result_dict.update(pipelines=results)
  return result_dict

################################################################################

def set_enforce_auth(new_status):
  """Sets whether Pipeline API handlers rely on app.yaml for access control.

  Args:
    new_status: If True, then the Pipeline API will enforce its own
      access control on status and static file handlers. If False, then
      it will assume app.yaml is doing the enforcement.
  """
  global _ENFORCE_AUTH
  _ENFORCE_AUTH = new_status


def create_handlers_map(prefix='.*'):
  """Create new handlers map.

  Args:
    prefix: url prefix to use.

  Returns:
    list of (regexp, handler) pairs for WSGIApplication constructor.
  """
  return [
      (prefix + '/output', _BarrierHandler),
      (prefix + '/run', _PipelineHandler),
      (prefix + '/finalized', _PipelineHandler),
      (prefix + '/cleanup', _CleanupHandler),
      (prefix + '/abort', _PipelineHandler),
      (prefix + '/fanout', _FanoutHandler),
      (prefix + '/fanout_abort', _FanoutAbortHandler),
      (prefix + '/callback', _CallbackHandler),
      (prefix + '/rpc/tree', status_ui._TreeStatusHandler),
      (prefix + '/rpc/class_paths', status_ui._ClassPathListHandler),
      (prefix + '/rpc/list', status_ui._RootListHandler),
      (prefix + '(/.+)', status_ui._StatusUiHandler),
      ]
