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
import six



try:
  from depot_tools.depot_tools import gclient_eval
except ImportError:
  # This is a work around to fix the discrepency on file tree in tests.
  from depot_tools import gclient_eval

from google.appengine.ext import deferred

from dashboard import pinpoint_request
from dashboard.pinpoint.models.change import commit_cache
from dashboard.pinpoint.models.change import repository as repository_module
from dashboard.services import gitiles_service

from dashboard.common import utils

_REPO_EXCLUSION_KEY = 'pinpoint_repo_exclusion_map'


class NonLinearError(Exception):
  """Raised when trying to find the midpoint of Changes that are not linear."""


class DepsParsingError(Exception):
  """Raised when we find a DEPS but fail to parse it."""


Dep = collections.namedtuple('Dep', ('repository_url', 'git_hash'))
CommitPositionInfo = collections.namedtuple('CommitPositionInfo',
                                            ('branch', 'position'))


def ParseDateWithUTCOffset(date_string):
  # Parsing the utc offset within strptime isn't supported until python 3, so
  # using workaround from https://stackoverflow.com/questions/26165659/
  if '+' in date_string:
    utc_sign = '+'
  elif '-' in date_string:
    utc_sign = '-'
  else:
    utc_sign = None

  if utc_sign:
    date_string, utc_offset = date_string.split(utc_sign)
    date_string = date_string.strip()

  dt = datetime.datetime.strptime(date_string, '%a %b %d %H:%M:%S %Y')

  if utc_sign and len(utc_offset) == 4:
    if utc_sign == '+':
      dt -= datetime.timedelta(
          hours=int(utc_offset[0:2]), minutes=int(utc_offset[2:]))
    elif utc_sign == '-':
      dt += datetime.timedelta(
          hours=int(utc_offset[0:2]), minutes=int(utc_offset[2:]))
  return dt


class Commit(collections.namedtuple('Commit', ('repository', 'git_hash'))):
  """A git repository pinned to a particular commit."""

  def __init__(self, *args, **kwargs):
    super().__init__()
    print(args, kwargs)  #  hard to bypass pylint here
    self._repository_url = None

  def __new__(cls, *args, **kwargs):
    self = super(Commit, cls).__new__(cls, *args, **kwargs)
    return self

  def __str__(self):
    """Returns an informal short string representation of this Commit."""
    return self.repository + '@' + self.git_hash[:7]

  def SetRepository_url(self, repository_url):
    self._repository_url = repository_url

  @property
  def id_string(self):
    """Returns a string that is unique to this repository and git hash."""
    return self.repository + '@' + str(self.git_hash).strip()

  @property
  def repository_url(self):
    """The HTTPS URL of the repository as passed to `git clone`."""
    cached_url = getattr(self, '_repository_url', None)
    if not cached_url:
      self.SetRepository_url(repository_module.RepositoryUrl(self.repository))
    return self._repository_url

  def Deps(self):
    """Return the DEPS of this Commit.

    Returns Dep namedtuples with repository URLs instead of Commit objects,
    because Commit objects must have their repositories verified in the
    datastore, and we'd like to do that more lazily.

    Returns:
      A frozenset of Dep (repository_url, git_hash) namedtuples.
    """
    # Download and execute DEPS file.
    git_hash = str(self.git_hash).strip()
    try:
      deps_file_contents = gitiles_service.FileContents(
          self.repository_url,
          git_hash,
          'DEPS',
      )
    except gitiles_service.NotFoundError:
      logging.debug("No DEPS in this commit: %s", git_hash)
      return frozenset()  # No DEPS file => no DEPS.

    try:
      deps_data = gclient_eval.Parse(
          deps_file_contents, '{}@{}/DEPS'.format(
              self.repository_url,
              git_hash,
          ))
    except gclient_eval.Error as e:
      # Invalid/unparseable DEPS file.
      logging.error(
          'Failed to parse DEPS file in commit %s to do further bisect: %s',
          git_hash, str(e))
      raise DepsParsingError("Failed to parse DEPS file in commit %s@%s" %
                             (self.repository_url, git_hash)) from e

    # Pull out deps dict, including OS-specific deps.
    deps_dict = deps_data.get('deps', {})
    if not deps_dict:
      logging.debug("No deps key in DEPS data in this commit: %s", git_hash)
      return frozenset()

    # Convert deps strings to repository and git hash.
    logging.debug("DEPS data loaded for commit %s: %s", git_hash, deps_dict)
    commits = []
    for dep_value in deps_dict.values():
      if dep_value.get('dep_type') != 'git':
        # We don't support DEPS that are CIPD packages.
        continue

      dep_string = dep_value.get('url', '')

      dep_string_parts = dep_string.split('@')
      if len(dep_string_parts) < 2:
        continue  # Dep is not pinned to any particular revision.
      if len(dep_string_parts) > 2:
        raise NotImplementedError('Unknown DEP format: ' + dep_string)

      repository_url, dep_git_hash = dep_string_parts
      if repository_url.endswith('.git'):
        repository_url = repository_url[:-4]
      commits.append(Dep(repository_url, dep_git_hash.strip()))

    return frozenset(commits)

  def AsDict(self):
    d = {
        'repository': self.repository,
        'git_hash': str(self.git_hash).strip(),
    }

    d.update(self.GetOrCacheCommitInfo())
    d['created'] = d['created'].isoformat()

    commit_position = _ParseCommitPosition(d['message'])
    if commit_position:
      d['commit_branch'] = commit_position.branch
      d['commit_position'] = commit_position.position

    review_url = _ParseCommitField('Reviewed-on: ', d['message'])
    if review_url:
      d['review_url'] = review_url

    change_id = _ParseCommitField('Change-Id: ', d['message'])
    if change_id:
      d['change_id'] = change_id

    return d

  @classmethod
  def FromDep(cls, dep):
    """Create a Commit from a Dep namedtuple as returned by Deps().

    If the repository url is unknown, it will be added to the local datastore.

    If the repository is on the exclusion list returns None.

    Arguments:
      dep: A Dep namedtuple.

    Returns:
      A Commit or None.
    """
    repository = repository_module.RepositoryName(
        dep.repository_url, add_if_missing=True)
    if repository in utils.GetRepositoryExclusions():
      return None
    commit = cls(repository, str(dep.git_hash).strip())
    commit.SetRepository_url(dep.repository_url)
    return commit

  @classmethod
  def FromData(cls, data):
    """Create a Commit from the given request data.

    Raises:
      KeyError: The repository name is not in the local datastore,
                or the git hash is not valid.
      ValueError: The URL has an unrecognized format.
    """
    if isinstance(data, six.string_types):
      return cls.FromUrl(data)
    return cls.FromDict(data)

  @classmethod
  def FromUrl(cls, url):
    """Create a Commit from a Gitiles URL.

    Raises:
      KeyError: The URL's repository or commit doesn't exist.
      ValueError: The URL has an unrecognized format.
    """
    url_parts = url.split('+')
    if len(url_parts) != 2:
      raise ValueError('Unknown commit URL format: ' + url)

    repository, git_hash = url_parts

    return cls.FromDict({
        'repository': repository[:-1],
        'git_hash': str(git_hash[1:]).strip(),
    })

  @classmethod
  def FromDict(cls, data):
    """Create a Commit from a dict.

    If the repository is a repository URL, it will be translated to its short
    form name.

    Raises:
      KeyError: The repository name is not in the local datastore,
                or the git hash is not valid.
    """
    repository = data['repository']
    if repository == 'angleproject':
      repository = 'angle'

    git_hash = str(data['git_hash']).strip()
    if not git_hash:
      raise KeyError("No git hash given")

    # Translate repository if it's a URL.
    if repository.startswith('https://'):
      repository = repository_module.RepositoryName(repository)
    cache_miss = True
    key = repository + '@' + git_hash

    try:
      repository_url = repository_module.RepositoryUrl(repository)

      if git_hash[0] == '-':
        # If it's already in the hash, then we've resolved this recently, and we
        # don't go resolving the data from the gitiles service.
        # new key for query only
        key = repository + '@' + git_hash[1:]
        result = commit_cache.Get(key)
        if result:
          git_hash = result.get('url').split('/')[-1]
          cache_miss = False
        else:
          git_hash = git_hash[1:]
      if cache_miss:
        try:
          # Try with commit position
          git_hash = pinpoint_request.ResolveToGitHash(git_hash)
        except ValueError:
          pass
        result = gitiles_service.CommitInfo(repository_url, git_hash)
        git_hash = result['commit']
    except gitiles_service.NotFoundError as e:
      six.raise_from(KeyError(str(e)), e)

    commit = cls(repository, git_hash)
    commit.SetRepository_url(repository_url)

    # IF this is a ref like HEAD, cache this for a short time so that we
    # fetch the build which is likely already built.
    if not gitiles_service.IsHash(data['git_hash']):
      if cache_miss:
        commit.CacheCommitInfo(result, key=key, memcache_timeout=60 * 60 * 10)
    return commit

  @classmethod
  def CommitRange(cls, commit_a, commit_b):
    # We need to get the full list of commits in between two git hashes, and
    # only look into the chain formed by following the first parents of each
    # commit. This gives us a linear view of the log even in the presence of
    # merge commits.
    commits = []

    # The commit_range by default is in reverse-chronological (latest commit
    # first) order. This means we should keep following the first parent to get
    # the linear history for a branch that we're exploring.
    expected_parent = commit_b.git_hash
    commit_range = gitiles_service.CommitRange(commit_a.repository_url,
                                               commit_a.git_hash,
                                               commit_b.git_hash)
    for commit in commit_range:
      # Skip commits until we find the parent we're looking for.
      if commit['commit'] == expected_parent:
        commits.append(commit)
        if 'parents' in commit and len(commit['parents']):
          expected_parent = commit['parents'][0]

    return commits

  def GetOrCacheCommitInfo(self):
    try:
      return commit_cache.Get(self.id_string)
    except KeyError:
      commit_info = gitiles_service.CommitInfo(self.repository_url,
                                               self.git_hash)
      return self.CacheCommitInfo(commit_info)

  def CacheCommitInfo(self, commit_info, key=None, memcache_timeout=None):
    cache_key = key or self.id_string
    url = self.repository_url + '/+/' + commit_info['commit']
    author = commit_info['author']['email']

    created = ParseDateWithUTCOffset(commit_info['committer']['time'])

    subject = commit_info['message'].split('\n', 1)[0]
    message = commit_info['message']

    commit_cache.Put(
        cache_key,
        url,
        author,
        created,
        subject,
        message,
        memcache_timeout=memcache_timeout)

    return {
        'url': url,
        'author': author,
        'created': created,
        'subject': subject,
        'message': message,
    }

  @classmethod
  def Midpoint(cls, commit_a, commit_b):
    """Return a Commit halfway between the two given Commits.

    Uses Gitiles to look up the commit range.

    Args:
      commit_a: The first Commit in the range.
      commit_b: The last Commit in the range.

    Returns:
      A new Commit representing the midpoint.
      The commit before the midpoint if the range has an even number of commits.
      commit_a if the Commits are the same or adjacent.

    Raises:
      NonLinearError: The Commits are in different repositories or commit_a does
        not come before commit_b.
    """
    if commit_a == commit_b:
      return commit_a

    if commit_a.repository != commit_b.repository:
      raise NonLinearError('Repositories differ between Commits: %s vs %s' %
                           (commit_a.repository, commit_b.repository))

    commits = cls.CommitRange(commit_a, commit_b)

    # We don't handle NotFoundErrors because we assume that all Commits either
    # came from this method or were already validated elsewhere.
    if len(commits) == 0:
      raise NonLinearError('Commit "%s" does not come before commit "%s".' %
                           (commit_a, commit_b))

    if len(commits) == 1:
      return commit_a

    commits.pop(0)  # Remove commit_b from the range.

    # Batch up the commits into chunks of 100 to avoid exceeding the size limit
    # for the bound data in calls to `deferred.defer(...)`.
    for offset in range(0, len(commits), 100):
      deferred.defer(_CacheCommitDetails, commit_a.repository,
                     commits[offset:offset + 100])

    return cls(commit_a.repository, commits[len(commits) // 2]['commit'])


def _CacheCommitDetails(repository, commits):
  for cur in commits:
    c = Commit(repository, cur['commit'])
    c.CacheCommitInfo(cur)


def _ParseCommitPosition(commit_message):
  """Parses a commit message for the commit position.

  Args:
    commit_message: The commit message as a string.

  Returns:
    An int if there is a commit position, or None otherwise."""
  match = re.search('^Cr-Commit-Position: (.+)@{#([0-9]+)}$', commit_message,
                    re.MULTILINE)
  if match:
    return CommitPositionInfo(match.group(1), int(match.group(2)))
  return None


def _ParseCommitField(field, commit_message):
  for l in commit_message.splitlines():
    match = l.split(field)
    if len(match) == 2:
      return match[1]
  return None


def RepositoryInclusionFilter(commit):
  """Returns False for changes in repositories in the exclusion list."""
  return commit.repository not in utils.GetRepositoryExclusions()
