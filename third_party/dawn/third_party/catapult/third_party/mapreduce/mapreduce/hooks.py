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

"""API allowing control over some mapreduce implementation details."""



__all__ = ["Hooks"]


class Hooks(object):
  """Allows subclasses to control some aspects of mapreduce execution.

  control.start_map accepts an optional "hooks" argument that can be passed a
  subclass of this class.
  """

  def __init__(self, mapreduce_spec):
    """Initializes a Hooks class.

    Args:
      mapreduce_spec: The mapreduce.model.MapreduceSpec for the current
        mapreduce.
    """
    self.mapreduce_spec = mapreduce_spec

  def enqueue_worker_task(self, task, queue_name):
    """Enqueues a worker task that is used to run the mapper.

    Args:
      task: A taskqueue.Task that must be queued in order for the mapreduce
        mappers to be run. The task is named.
      queue_name: The queue where the task should be run e.g. "default".

    Raises:
      NotImplementedError: to indicate that the default worker queueing strategy
        should be used.
    """
    raise NotImplementedError()

  def enqueue_kickoff_task(self, task, queue_name):
    """Enqueues a task that is used to start the mapreduce.

    This hook will be called within a transaction scope.
    Hook should add task transactionally.

    Args:
      task: A taskqueue.Task that must be queued to run KickOffJobHandler.
      queue_name: The queue where the task should be run e.g. "default".

    Raises:
      NotImplementedError: to indicate that the default mapreduce start strategy
        should be used.
    """
    raise NotImplementedError()

  def enqueue_done_task(self, task, queue_name):
    """Enqueues a task that is triggered when the mapreduce completes.

    This hook will be called within a transaction scope.
    Hook should add task transactionally.

    Args:
      task: A taskqueue.Task that must be queued in order for the client to be
        notified when the mapreduce is complete.
      queue_name: The queue where the task should be run e.g. "default".

    Raises:
      NotImplementedError: to indicate that the default mapreduce notification
        strategy should be used.
    """
    raise NotImplementedError()

  def enqueue_controller_task(self, task, queue_name):
    """Enqueues a task that is used to monitor the mapreduce process.

    Args:
      task: A taskqueue.Task that must be queued in order for updates to the
        mapreduce process to be properly tracked. The task is named.
      queue_name: The queue where the task should be run e.g. "default".

    Raises:
      NotImplementedError: to indicate that the default mapreduce tracking
        strategy should be used.
    """
    raise NotImplementedError()
