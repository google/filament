# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import

import json

from google.cloud import tasks_v2


class CloudTasksClient:

  def __init__(self):
    self._client = tasks_v2.CloudTasksClient()

  def EnqueueTask(self,
      project: str,
      queue_name: str,
      queue_location:str,
      task_payload: dict,
      target_uri: str,
      headers: dict = {}):

    parent = self._client.queue_path(
        project, queue_location, queue_name)
    payload = json.dumps(task_payload).encode()

    # Construct the request body.
    task = {
        'http_request': {
            'http_method': tasks_v2.HttpMethod.POST,
            'url': target_uri,
            'headers': headers,
            'body': payload
        }
    }

    response = self._client.create_task(parent=parent, task=task)
    print(response)
