# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import datetime
import logging
import re
import six.moves.urllib.parse as urlparse

from dashboard.pinpoint.models import errors
from dashboard.pinpoint.models.change import commit_cache
from dashboard.services import gerrit_service
import six


class GerritPatch(
    collections.namedtuple('GerritPatch', ('server', 'change', 'revision'))):
  """A patch in Gerrit.

  change is a change ID of the format '<project>~<branch>~<Change-Id>' and
  revision is a commit ID. Both are described in the Gerrit API documentation.
  https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#ids

  They must be in a canonical format so we can look up builds precisely.
  """

  def __str__(self):
    return self.revision[:7].strip()

  @property
  def id_string(self):
    return '%s/%s/%s' % (self.server, str(
        self.change).strip(), str(self.revision).strip())

  def BuildParameters(self):
    patch_info = gerrit_service.GetChange(
        self.server, self.change, fields=('ALL_REVISIONS',))
    revision_info = patch_info['revisions'][self.revision]
    return {
        'patch_gerrit_url': self.server,
        'patch_issue': patch_info['_number'],
        'patch_project': patch_info['project'],
        'patch_ref': revision_info['fetch']['http']['ref'],
        'patch_repository_url': revision_info['fetch']['http']['url'],
        'patch_set': revision_info['_number'],
        'patch_storage': 'gerrit',
    }

  @property
  def hostname(self):
    h = self.server.split('://')[-1]
    return h.split('/')[0]

  def BuildsetTags(self):
    patch_info = gerrit_service.GetChange(
        self.server, self.change, fields=('ALL_REVISIONS',))
    revision_info = patch_info['revisions'][self.revision]
    return 'buildset:patch/gerrit/%s/%s/%s' % (
        self.hostname, patch_info['_number'], revision_info['_number'])

  def AsDict(self):
    revision = str(self.revision).strip()
    d = {
        'server': self.server,
        'change': self.change,
        'revision': revision,
    }

    try:
      d.update(commit_cache.Get(self.id_string))
      d['created'] = d['created'].isoformat()
    except KeyError:
      try:
        patch_info = gerrit_service.GetChange(
            self.server,
            str(self.change).strip(),
            fields=('ALL_REVISIONS', 'DETAILED_ACCOUNTS', 'COMMIT_FOOTERS'))
        revision_info = patch_info['revisions'][revision]
        url = '%s/c/%s/+/%d/%d' % (
            self.server,
            patch_info['project'],
            patch_info['_number'],
            revision_info['_number'],
        )
        author = revision_info['uploader']['email']
        created = datetime.datetime.strptime(
            revision_info['created'],
            '%Y-%m-%d %H:%M:%S.%f000',
        )
        subject = patch_info['subject']
        current_revision = patch_info['current_revision']
        message = patch_info['revisions'][current_revision][
            'commit_with_footers']

        d.update({
            'url': url,
            'author': author,
            'created': created.isoformat(),
            'subject': subject,
            'message': message,
        })
        commit_cache.Put(self.id_string, url, author, created, subject, message)
      except gerrit_service.NotFoundError:
        logging.warning('Failed to retrieve change %s', revision)

    return d

  @classmethod
  def FromData(cls, data):
    """Creates a new GerritPatch from the given request data.

    Args:
      data: A patch URL string, for example:
        https://chromium-review.googlesource.com/c/chromium/tools/build/+/679595
        Or a dict containing {server, change, revision [optional]}.
        change is a {change-id} as described in the Gerrit API documentation.
        revision is a commit ID hash or numeric patch number.
        If revision is omitted, it is the change's current revision.

    Returns:
      A GerritPatch.

    Raises:
      KeyError: The patch doesn't exist or doesn't have the given revision.
      ValueError: The URL has an unrecognized format.
    """
    if isinstance(data, six.string_types):
      return cls.FromUrl(data)
    return cls.FromDict(data)

  @classmethod
  def FromUrl(cls, url):
    """Creates a new GerritPatch from the given URL.

    Args:
      url: A patch URL string, for example:
        https://chromium-review.googlesource.com/c/chromium/tools/build/+/679595

    Returns:
      A GerritPatch.

    Raises:
      KeyError: The patch doesn't have the given revision.
      ValueError: The URL has an unrecognized format.
    """
    url_parts = urlparse.urlparse(url)
    server = urlparse.urlunsplit(
        (url_parts.scheme, url_parts.netloc, '', '', ''))

    change_rev_match = re.match(r'^https.*\/\+\/(\d+)(?:\/(\d+))?\/?$', url)
    change_match = re.match(r'^\/(\d+)\/?$', url_parts.path)
    redirector_match = re.match(r'^/c/(\d+)(?:\/(\d+))?\/?$', url_parts.path)
    if change_rev_match:
      change = change_rev_match.group(1)
      revision = change_rev_match.group(2)
    elif change_match:  # support URLs returned by the 'git cl issue' command
      change = change_match.group(1)
      revision = None
    elif redirector_match:  # Supprt non-fully-resolved URLs
      change = redirector_match.group(1)
      revision = redirector_match.group(2)
    else:
      raise errors.BuildGerritURLInvalid(url)

    return cls.FromDict({
        'server': server,
        'change': int(change),
        'revision': int(revision) if revision else None,
    })

  @classmethod
  def FromDict(cls, data):
    """Creates a new GerritPatch from the given dict.

    Args:
      data: A dict containing {server, change, revision [optional]}.
        change is a {change-id} as described in the Gerrit API documentation.
        revision is a commit ID hash or numeric patch number.
        If revision is omitted, it is the change's current revision.

    Returns:
      A GerritPatch.

    Raises:
      KeyError: The patch doesn't have the given revision.
    """
    server = data['server']
    change = str(data['change']).strip()
    revision = data.get('revision')

    # Look up the patch and convert everything to a canonical format.
    try:
      patch_info = gerrit_service.GetChange(
          server, change, fields=('ALL_REVISIONS',))
    except gerrit_service.NotFoundError as e:
      raise KeyError(str(e)) from e
    change = patch_info['id']

    # Revision can be a revision ID or numeric patch number.
    if not revision:
      revision = patch_info['current_revision']
    for revision_id, revision_info in patch_info['revisions'].items():
      if revision in [revision_id, revision_info.get('_number')]:
        revision = revision_id
        break
    else:
      raise KeyError('Patch revision not found: %s/%s revision %s' %
                     (server, change, revision))

    return cls(server, change, revision)
