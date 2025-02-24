# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Encapsulates a simplistic interface to the buildbucket service."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import json
import re
import uuid

from dashboard.services import request

# If we need to use the dev environment of buildbucket, the host will be:
#    https://cr-buildbucket-dev.appspot.com
# and the url for the build is:
#    https://luci-milo-dev.appspot.com/b/<build_id>
API_BASE_URL2 = 'https://cr-buildbucket.appspot.com/prpc/buildbucket.v2.Builds/'

# Default Buildbucket bucket name.
_BUCKET_NAME = 'master.tryserver.chromium.perf'

# Gitiles commit buildset pattern. Example:
# ('commit/gitiles/chromium.googlesource.com/infra/luci/luci-go/+/'
#  'b7a757f457487cd5cfe2dae83f65c5bc10e288b7')
_GITILES_COMMIT_RE = re.compile(
    r'^commit/gitiles/([^/]+)/(.+?)/\+/([a-f0-9]{40})$')
# Gerrit CL buildset pattern. Example:
# patch/gerrit/chromium-review.googlesource.com/677784/5
_GERRIT_CL_RE = re.compile(r'^patch/gerrit/([^/]+)/(\d+)/(\d+)$')

_BUCKET_RE = re.compile(r'luci\.([^.]+)\.([^.]+)')


def Put(bucket, tags, parameters):
  logging.info("bbv2 tags: \n%s\n", tags)
  bucket_parts = _BUCKET_RE.split(bucket)
  if len(bucket_parts) != 4:
    raise ValueError('Could not parse bucket value: %s' % bucket)
  project = bucket_parts[1]
  bucket = bucket_parts[2]
  request_tags, gerrit_changes, gitiles_commit = [], [], None
  for tag_kvp in [tag.split(':') for tag in tags]:
    if tag_kvp[0] == 'buildset':
      match = _GERRIT_CL_RE.match(tag_kvp[1])
      if match:
        gerrit_changes.append({
            "host": match.group(1),
            "change": match.group(2),
            "patchset": match.group(3),
            "project": parameters.get('properties',
                                      {}).get('patch_project', '')
        })
      match = _GITILES_COMMIT_RE.match(tag_kvp[1])
      if match:
        gitiles_commit = {
            "host": match.group(1),
            "project": match.group(2),
            "id": match.group(3),
            "ref": "refs/heads/main"
        }
    else:
      request_tags.append({'key': tag_kvp[0], 'value': tag_kvp[1]})
  body = {
      'requestId': str(uuid.uuid4()),
      'builder': {
          'project': project,
          'bucket': bucket,
          'builder': parameters['builder_name'],
      },
      # Make sure 'tags' gets formatted like StringPair expects:
      # [{'key': key, 'value'; value}]
      'tags': request_tags,
      'properties': parameters.get('properties', {}),
  }
  if gerrit_changes:
    body['gerritChanges'] = gerrit_changes
  if gitiles_commit:
    body['gitilesCommit'] = gitiles_commit
  logging.info("bbv2 Put body: \n%s\n", json.dumps(body))

  return request.RequestJson(
      API_BASE_URL2 + 'ScheduleBuild', method='POST', body=body)


# TODO: Rename to Get().
def GetJobStatus(job_id):
  """Gets the details of a job via buildbucket's API."""
  body = {'id': job_id}
  logging.info("bbv2 GetJobStatus body: \n%s\n", json.dumps(body))
  return request.RequestJson(
      API_BASE_URL2 + 'GetBuild', method='POST', body=body)

def GetBuilds(project: str, bucket: str, builder: str):
  builder = {'project': project, 'bucket': bucket, 'builder': builder}
  predicate = {'builder': builder, 'status': 'SUCCESS'}
  body = {'predicate': predicate}
  return request.RequestJson(
    API_BASE_URL2 + 'SearchBuilds', method='POST', body=body)



# TODO(robertocn): Implement CancelJobByID
