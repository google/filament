# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import json
import logging
from six.moves.urllib import parse as urlparse

from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models import isolate
from dashboard.pinpoint.models.quest import execution
from dashboard.pinpoint.models.quest import quest
from dashboard.services import buildbucket_service
from dashboard.services import gerrit_service
from dashboard.common import utils

BUCKET = 'master.tryserver.chromium.perf'

_PP_TO_PERF_BUILDER_MAP = {
    'Linux Builder Perf':
        'linux-builder-perf',
    'Android Compile Perf':
        'android-builder-perf',
    'Android arm64 Compile Perf':
        'android_arm64-builder-perf',
    'Android arm64 High End Compile Perf':
        'android_arm64_high_end-builder-perf',
    'Mac Builder Perf':
        'mac-builder-perf',
    'Mac arm Builder Perf':
        'mac-arm-builder-perf',
    'Mac arm Builder Perf PGO':
        'mac-arm-builder-perf-pgo',
    'Win x64 Builder Perf':
        'win64-builder-perf',
    'Chromeos Amd64 Generic Lacros Builder Perf':
        'chromeos-amd64-generic-lacros-builder-perf'
}


class FindIsolate(quest.Quest):

  def __init__(self,
               builder,
               target,
               bucket,
               fallback_target=None,
               comparison_mode='performance'):
    self._builder_name = builder
    self._target = target
    self._bucket = bucket
    self._fallback_target = fallback_target
    self._comparison_mode = comparison_mode

    self._previous_builds = {}
    self._build_tags = collections.OrderedDict()
    self._started_executions = {}
  def __eq__(self, other):
    return (isinstance(other, type(self)) and self._bucket == other._bucket
            and self._builder_name == other._builder_name
            and self._fallback_target == other._fallback_target
            and self._started_executions == other._started_executions)
  def __hash__(self):
    return hash(self.__str__())

  def __str__(self):
    return 'Build'

  @property
  def build_tags(self):
    if hasattr(self, '_build_tags'):
      return self._build_tags
    return collections.OrderedDict()

  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    # TODO(dberris): Remove this when we migrate all the instances.
    if not hasattr(self, '_build_tags'):
      self._build_tags = collections.OrderedDict()

  def Start(self, change):
    if not hasattr(self, '_started_executions'):
      self._started_executions = {}
    if change not in self._started_executions:
      self._started_executions[change] = []
    index = len(self._started_executions[change])
    build_execution = _FindIsolateExecution(
        self,
        self._builder_name,
        self._target,
        self._bucket,
        change,
        index,
        self._previous_builds,
        self.build_tags,
        fallback_target=self._fallback_target,
        comparison_mode=self._comparison_mode)

    self._started_executions[change].append(build_execution)
    return build_execution
  def PropagateJob(self, job):
    self._build_tags = BuildTagsFromJob(job)

  @classmethod
  def FromDict(cls, arguments):
    for arg in ('builder', 'target', 'bucket', 'comparison_mode'):
      if arg not in arguments:
        raise TypeError('Missing "{0}" argument'.format(arg))

    return cls(arguments['builder'], arguments['target'], arguments['bucket'],
               arguments.get('fallback_target'), arguments['comparison_mode'])
  def CompleteExecutions(self, change, index, result_arguments):
    for e in self._started_executions[change][:index+1]:
      e._Complete(result_arguments=result_arguments)
class _FindIsolateExecution(execution.Execution):

  def __init__(self,
               containing_quest,
               builder_name,
               target,
               bucket,
               change,
               index,
               previous_builds,
               build_tags,
               fallback_target,
               comparison_mode='performance'):
    super().__init__()
    self._quest = containing_quest
    self._builder_name = builder_name
    self._target = target
    self._bucket = bucket
    self._change = change
    self._fallback_target = fallback_target
    self._comparison_mode = comparison_mode
    self._index = index

    # previous_builds is shared among all Executions of the same Quest.
    self._previous_builds = previous_builds

    self._build = None
    self._build_url = None

    # an ordered dict of tags.
    self._build_tags = build_tags

  def _AsDict(self):
    details = []
    details.append({
        'key': 'builder',
        'value': self._builder_name,
    })
    if self._build:
      details.append({
          'key': 'build',
          'value': self._build,
          'url': self._build_url if hasattr(self, '_build_url') else None,
      })
    if self._result_arguments:
      details.append({
          'key':
              'isolate',
          'value':
              self._result_arguments['isolate_hash'],
          'url':
              'https://cas-viewer.appspot.com/{}/blobs/{}/tree'.format(
                  self._result_arguments['isolate_server'],
                  self._result_arguments['isolate_hash']),
      })
    return details

  def _Poll(self):
    logging.debug('_FindIsolateExecution Polling: %s', self._AsDict())

    if self._CheckIsolateCache(
        _PP_TO_PERF_BUILDER_MAP.get(self._builder_name, '')):
      return

    if self._CheckIsolateCache():
      return

    if self._build:
      logging.debug('Checking build status for: %s', self._build)
      self._CheckBuildStatus()
      return

    self._RequestBuild()

  def _CheckIsolateCache(self, builder_name_override=''):
    """Checks the isolate cache to see if a build is already available.

    Returns:
      True iff the isolate was found, meaning the execution is completed.
    """
    try:
      builder_name = builder_name_override if builder_name_override \
        else self._builder_name
      isolate_server, isolate_hash = isolate.Get(builder_name, self._change,
                                                 self._target)
    except KeyError as e:
      logging.debug('NOT found in isolate cache: %s', str(e))
      if self._fallback_target:
        try:
          isolate_server, isolate_hash = isolate.Get(self._builder_name,
                                                     self._change,
                                                     self._fallback_target)
          logging.info('Fallback, %s, is found and will be used to replace %s.',
                       self._fallback_target, self._target)
        except KeyError as e:
          logging.debug('fallback NOT found in isolate cache %s', str(e))
          return False
      else:
        return False

    result_arguments = {
        'isolate_server': isolate_server,
        'isolate_hash': isolate_hash,
    }
    logging.debug('Found in isolate cache: %s', result_arguments)
    if not hasattr(self, '_quest'):
      logging.debug('Execute older complete execution method in find_isolate')
      self._Complete(result_arguments=result_arguments)
    else:
      self._quest.CompleteExecutions(self._change, self._index,
                                     result_arguments)
    return True

  def _IsTryJob(self):
    return self._comparison_mode == 'try'

  def _CheckBuildStatus(self):
    """Checks on the status of a previously requested build.

    Raises:
      BuildError: The build failed, was canceled, or didn't produce an isolate.
    """
    job_status = buildbucket_service.GetJobStatus(self._build)
    logging.debug('buildbucket response V2: %s', job_status)

    build_id = job_status.get('id', '')
    self._build_url = utils.GetBuildbucketUrl(build_id)

    build_status = job_status.get('status', '')
    if build_status in ('SCHEDULED', 'STARTED'):
      return
    if build_status == 'FAILURE':
      if self._IsTryJob():
        raise errors.BuildFailedFatal('BUILD_FAILURE')
      raise errors.BuildFailed('BUILD_FAILURE')
    if build_status == 'INFRA_FAILURE':
      reason = 'TIMEOUT' if 'timeout' in job_status.get('statusDetails',
                                                        {}) else 'INFRA_FAILURE'
      if self._IsTryJob():
        raise errors.BuildFailedFatal(reason)
      raise errors.BuildFailed(reason)
    if build_status == 'CANCELED':
      if self._IsTryJob():
        raise errors.BuildCancelledFatal('CANCELED_EXPLICITLY')
      raise errors.BuildCancelled('CANCELED_EXPLICITLY')

    # The build succeeded, and should now be in the isolate cache.
    # If it is, this will call self._Complete()
    if not self._CheckIsolateCache():
      raise errors.BuildIsolateNotFound()

  @property
  def bucket(self):
    if hasattr(self, '_bucket'):
      return self._bucket
    return BUCKET

  @property
  def build_tags(self):
    if hasattr(self, '_build_tags'):
      return self._build_tags
    return collections.OrderedDict()

  def __setstate__(self, state):
    self.__dict__ = state  # pylint: disable=attribute-defined-outside-init
    # TODO(dberris): Remove this when we migrate all the instances.
    if not hasattr(self, '_build_tags'):
      self._build_tags = collections.OrderedDict()

  def _RequestBuild(self):
    """Requests a build.

    If a previous Execution already requested a build for this Change, returns
    that build instead of requesting a new one.
    """
    logging.debug('_FindIsolateExecution _RequestBuild')

    if self._change in self._previous_builds:
      logging.debug('%s in list of previous_builds', self._change)
      # If another Execution already requested a build, reuse that one.
      self._build = self._previous_builds[self._change]
    else:
      logging.debug('Requesting a build')
      # Request a build!
      buildbucket_info = RequestBuild(self._builder_name, self._change,
                                      self.bucket, self.build_tags)
      self._build = buildbucket_info['id']
      self._previous_builds[self._change] = self._build


def RequestBuild(builder_name, change, bucket, build_tags, task=None):
  base_as_dict = change.base_commit.AsDict()
  review_url = base_as_dict.get('review_url')
  if not review_url:
    raise errors.BuildGerritUrlNotFound(str(change.base_commit))

  url_parts = urlparse.urlparse(review_url)
  base_review_url = urlparse.urlunsplit(
      (url_parts.scheme, url_parts.netloc, '', '', ''))

  patch = change_module.GerritPatch.FromUrl(review_url)

  change_info = gerrit_service.GetChange(base_review_url, patch.change)

  commit_url_parts = urlparse.urlparse(base_as_dict['url'])

  # Note: The ordering here for buildbucket v1 api is important.
  # crbug.com/937392
  builder_tags = []
  if change.patch:
    builder_tags.append(change.patch.BuildsetTags())
  builder_tags.append('buildset:commit/gitiles/%s/%s/+/%s' %
                      (commit_url_parts.netloc, change_info['project'],
                       change.base_commit.git_hash))
  builder_tags.extend(['%s:%s' % (k, v) for k, v in build_tags.items()])

  deps_overrides = {dep.repository_url: dep.git_hash for dep in change.deps}
  parameters = {
      'builder_name': builder_name,
      'properties': {
          # We're making Pinpoint use incremental builds to amortise the cost
          # of rebuilding the object files. Clobber builds indicate that a
          # builder will clean out previous build artifacts instead of re-using
          # potentially already-built object files from a previous checkout.
          # Incremental builds will be much faster especially with the help of
          # RBE.
          'clobber': False,
          'deps_revision_overrides': deps_overrides,
          'git_repo': change.base_commit.repository_url,
          'revision': change.base_commit.git_hash,
          'staging': utils.IsStagingEnvironment(),
      },
  }

  if change.patch:
    parameters['properties'].update(change.patch.BuildParameters())

  logging.debug('bucket: %s', bucket)
  logging.debug('builder_tags: %s', builder_tags)
  logging.debug('parameters: %s', parameters)

  pubsub_callback = None
  if build_tags:
    # This means we have access to Pinpoint job details, we should provide this
    # information to the attempts to build.
    pubsub_callback = {
        # TODO(dberris): Consolidate constants in environment vars?
        'topic':
            'projects/chromeperf/topics/pinpoint-swarming-updates',
        'auth_token':
            'UNUSED',
        'user_data':
            json.dumps({
                'job_id': build_tags.get('pinpoint_job_id'),
                'task': {
                    'type':
                        'build',
                    'id':
                        build_tags.get('pinpoint_task_id')
                        if not task else task.id,
                }
            })
    }
    logging.debug('pubsub_callback, which we ignore: %s', pubsub_callback)

  # TODO: Look up Buildbucket bucket from builder_name.
  return buildbucket_service.Put(bucket, builder_tags, parameters)


def BuildTagsFromJob(job):
  return collections.OrderedDict([
      ('pinpoint_job_id', job.job_id),
      ('pinpoint_user', job.user),
      ('pinpoint_url', job.url),
  ])
