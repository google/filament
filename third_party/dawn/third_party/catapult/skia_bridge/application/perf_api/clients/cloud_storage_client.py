# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from google.cloud import storage
from google.api_core import exceptions
from google.api_core.retry import Retry

RETRIABLE_ERRORS = [
    exceptions.TooManyRequests,  # 429
    exceptions.BadGateway,  # 502
    exceptions.ServiceUnavailable,  # 503
]

def is_retryable(exc):
  return isinstance(exc, RETRIABLE_ERRORS)

class CloudStorageClient:
  _client = storage_client = storage.Client()

  def UploadDataToBucket(
      self,
      file_name: str,
      file_content: str,
      bucket_name: str,
      content_type='text/plain'):

    retry_policy = Retry(predicate=is_retryable)
    bucket = self._client.bucket(bucket_name)
    blob = bucket.blob(file_name)

    blob.upload_from_string(
        file_content,
        content_type=content_type,
        retry=retry_policy)
