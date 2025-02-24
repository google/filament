#!/usr/bin/env vpython3
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Copyright (C) 2008 Evan Martin <martine@danga.com>
"""A git-command for integrating reviews on Gerrit."""

from __future__ import annotations

import base64
import collections
import datetime
import enum
import fnmatch
import functools
import httplib2
import itertools
import json
import logging
import multiprocessing
import optparse
import os
import re
import shutil
import stat
import sys
import tempfile
import textwrap
import time
import typing
import urllib.error
import urllib.parse
import urllib.request
import uuid
import webbrowser
import zlib

from typing import Any
from typing import AnyStr
from typing import Callable
from typing import List
from typing import Mapping
from typing import NoReturn
from typing import Optional
from typing import Sequence
from typing import Tuple

import auth
import clang_format
import gclient_paths
import gclient_utils
import gerrit_util
import git_auth
import git_common
import git_footers
import git_new_branch
import google_java_format
import metrics
import metrics_utils
import metrics_xml_format
import newauth
import owners_client
import owners_finder
import presubmit_canned_checks
import presubmit_support
import rustfmt
import scm
import setup_color
import split_cl
import subcommand
import subprocess2
import swift_format
import watchlists

from third_party import colorama


__version__ = '2.0'

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

# Traces for git push will be stored in a traces directory inside the
# depot_tools checkout.
DEPOT_TOOLS = os.path.dirname(os.path.abspath(__file__))
TRACES_DIR = os.path.join(DEPOT_TOOLS, 'traces')
PRESUBMIT_SUPPORT = os.path.join(DEPOT_TOOLS, 'presubmit_support.py')

# When collecting traces, Git hashes will be reduced to 6 characters to reduce
# the size after compression.
GIT_HASH_RE = re.compile(r'\b([a-f0-9]{6})[a-f0-9]{34}\b', flags=re.I)
# Used to redact the cookies from the gitcookies file.
GITCOOKIES_REDACT_RE = re.compile(r'1/.*')

MAX_ATTEMPTS = 3

# The maximum number of traces we will keep. Multiplied by 3 since we store
# 3 files per trace.
MAX_TRACES = 3 * 10
# Message to be displayed to the user to inform where to find the traces for a
# git-cl upload execution.
TRACES_MESSAGE = (
    '\n'
    'The traces of this git-cl execution have been recorded at:\n'
    '  %(trace_name)s-traces.zip\n'
    'Copies of your gitcookies file and git config have been recorded at:\n'
    '  %(trace_name)s-git-info.zip\n')
# Format of the message to be stored as part of the traces to give developers a
# better context when they go through traces.
TRACES_README_FORMAT = ('Date: %(now)s\n'
                        '\n'
                        'Change: https://%(gerrit_host)s/q/%(change_id)s\n'
                        'Title: %(title)s\n'
                        '\n'
                        '%(description)s\n'
                        '\n'
                        'Execution time: %(execution_time)s\n'
                        'Exit code: %(exit_code)s\n') + TRACES_MESSAGE

POSTUPSTREAM_HOOK = '.git/hooks/post-cl-land'
DESCRIPTION_BACKUP_FILE = '.git_cl_description_backup'
REFS_THAT_ALIAS_TO_OTHER_REFS = {
    'refs/remotes/origin/lkgr': 'refs/remotes/origin/main',
    'refs/remotes/origin/lkcr': 'refs/remotes/origin/main',
}

DEFAULT_OLD_BRANCH = 'refs/remotes/origin/master'
DEFAULT_NEW_BRANCH = 'refs/remotes/origin/main'

DEFAULT_BUILDBUCKET_HOST = 'cr-buildbucket.appspot.com'

# Valid extensions for files we want to lint.
DEFAULT_LINT_REGEX = r"(.*\.cpp|.*\.cc|.*\.h)"
DEFAULT_LINT_IGNORE_REGEX = r"$^"

# File name for yapf style config files.
YAPF_CONFIG_FILENAME = '.style.yapf'

# The issue, patchset and codereview server are stored on git config for each
# branch under branch.<branch-name>.<config-key>.
ISSUE_CONFIG_KEY = 'gerritissue'
PATCHSET_CONFIG_KEY = 'gerritpatchset'
CODEREVIEW_SERVER_CONFIG_KEY = 'gerritserver'
# When using squash workflow, _CMDUploadChange doesn't simply push the commit(s)
# you make to Gerrit. Instead, it creates a new commit object that contains all
# changes you've made, diffed against a parent/merge base.
# This is the hash of the new squashed commit and you can find this on Gerrit.
GERRIT_SQUASH_HASH_CONFIG_KEY = 'gerritsquashhash'
# This is the latest uploaded local commit hash.
LAST_UPLOAD_HASH_CONFIG_KEY = 'last-upload-hash'

# Shortcut since it quickly becomes repetitive.
Fore = colorama.Fore

# Used by tests/git_cl_test.py to add extra logging.
# Inside the weirdly failing test, add this:
# >>> self.mock(git_cl, '_IS_BEING_TESTED', True)
# And scroll up to see the stack trace printed.
_IS_BEING_TESTED = False

_GOOGLESOURCE = 'googlesource.com'

_KNOWN_GERRIT_TO_SHORT_URLS = {
    'https://chrome-internal-review.googlesource.com': 'https://crrev.com/i',
    'https://chromium-review.googlesource.com': 'https://crrev.com/c',
}
assert len(_KNOWN_GERRIT_TO_SHORT_URLS) == len(
    set(_KNOWN_GERRIT_TO_SHORT_URLS.values())), 'must have unique values'

# Maximum number of branches in a stack that can be traversed and uploaded
# at once. Picked arbitrarily.
_MAX_STACKED_BRANCHES_UPLOAD = 20

_NO_BRANCH_ERROR = (
    'Unable to determine base commit in detached HEAD state. '
    'Get on a branch or run `git cl upload --no-squash <base>` to '
    'upload all commits since base!')


class GitPushError(Exception):
    pass


def DieWithError(message, change_desc=None) -> NoReturn:
    if change_desc:
        SaveDescriptionBackup(change_desc)
        print('\n ** Content of CL description **\n' + '=' * 72 + '\n' +
              change_desc.description + '\n' + '=' * 72 + '\n')

    print(message, file=sys.stderr)
    sys.exit(1)


def SaveDescriptionBackup(change_desc):
    backup_path = os.path.join(DEPOT_TOOLS, DESCRIPTION_BACKUP_FILE)
    print('\nsaving CL description to %s\n' % backup_path)
    with open(backup_path, 'wb') as backup_file:
        backup_file.write(change_desc.description.encode('utf-8'))


def GetNoGitPagerEnv():
    env = os.environ.copy()
    # 'cat' is a magical git string that disables pagers on all platforms.
    env['GIT_PAGER'] = 'cat'
    return env


def RunCommand(args, error_ok=False, error_message=None, shell=False, **kwargs):
    try:
        stdout = subprocess2.check_output(args, shell=shell, **kwargs)
        return stdout.decode('utf-8', 'replace')
    except subprocess2.CalledProcessError as e:
        logging.debug('Failed running %s', args)
        if not error_ok:
            message = error_message or e.stdout.decode('utf-8', 'replace') or ''
            DieWithError('Command "%s" failed.\n%s' % (' '.join(args), message))
        out = e.stdout.decode('utf-8', 'replace')
        if e.stderr:
            out += e.stderr.decode('utf-8', 'replace')
        return out


def RunGit(args, **kwargs):
    """Returns stdout."""
    return RunCommand(['git'] + args, **kwargs)


def RunGitWithCode(args, suppress_stderr=False):
    """Returns return code and stdout."""
    if suppress_stderr:
        stderr = subprocess2.DEVNULL
    else:
        stderr = sys.stderr
    try:
        (out, _), code = subprocess2.communicate(['git'] + args,
                                                 env=GetNoGitPagerEnv(),
                                                 stdout=subprocess2.PIPE,
                                                 stderr=stderr)
        return code, out.decode('utf-8', 'replace')
    except subprocess2.CalledProcessError as e:
        logging.debug('Failed running %s', ['git'] + args)
        return e.returncode, e.stdout.decode('utf-8', 'replace')


def RunGitSilent(args):
    """Returns stdout, suppresses stderr and ignores the return code."""
    return RunGitWithCode(args, suppress_stderr=True)[1]


def time_sleep(seconds):
    # Use this so that it can be mocked in tests without interfering with python
    # system machinery.
    return time.sleep(seconds)


def time_time():
    # Use this so that it can be mocked in tests without interfering with python
    # system machinery.
    return time.time()


def datetime_now():
    # Use this so that it can be mocked in tests without interfering with python
    # system machinery.
    return datetime.datetime.now()


def confirm_or_exit(prefix='', action='confirm'):
    """Asks user to press enter to continue or press Ctrl+C to abort."""
    if not prefix or prefix.endswith('\n'):
        mid = 'Press'
    elif prefix.endswith('.') or prefix.endswith('?'):
        mid = ' Press'
    elif prefix.endswith(' '):
        mid = 'press'
    else:
        mid = ' press'
    gclient_utils.AskForData('%s%s Enter to %s, or Ctrl+C to abort' %
                             (prefix, mid, action))


def ask_for_explicit_yes(prompt):
    """Returns whether user typed 'y' or 'yes' to confirm the given prompt."""
    result = gclient_utils.AskForData(prompt + ' [Yes/No]: ').lower()
    while True:
        if 'yes'.startswith(result):
            return True
        if 'no'.startswith(result):
            return False
        result = gclient_utils.AskForData('Please, type yes or no: ').lower()


def _get_properties_from_options(options):
    prop_list = getattr(options, 'properties', [])
    properties = dict(x.split('=', 1) for x in prop_list)
    for key, val in properties.items():
        try:
            properties[key] = json.loads(val)
        except ValueError:
            pass  # If a value couldn't be evaluated, treat it as a string.
    return properties


def _call_buildbucket(http, buildbucket_host, method, request):
    """Calls a buildbucket v2 method and returns the parsed json response."""
    headers = {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
    }
    request = json.dumps(request)
    url = 'https://%s/prpc/buildbucket.v2.Builds/%s' % (buildbucket_host,
                                                        method)

    logging.info('POST %s with %s' % (url, request))

    attempts = 1
    time_to_sleep = 1
    while True:
        response, content = http.request(url,
                                         'POST',
                                         body=request,
                                         headers=headers)
        if response.status == 200:
            return json.loads(content[4:])
        if attempts >= MAX_ATTEMPTS or 400 <= response.status < 500:
            msg = '%s error when calling POST %s with %s: %s' % (
                response.status, url, request, content)
            raise BuildbucketResponseException(msg)
        logging.debug('%s error when calling POST %s with %s. '
                      'Sleeping for %d seconds and retrying...' %
                      (response.status, url, request, time_to_sleep))
        time.sleep(time_to_sleep)
        time_to_sleep *= 2
        attempts += 1

    assert False, 'unreachable'


def _parse_bucket(raw_bucket):
    legacy = True
    project = bucket = None
    if '/' in raw_bucket:
        legacy = False
        project, bucket = raw_bucket.split('/', 1)
    # Assume luci.<project>.<bucket>.
    elif raw_bucket.startswith('luci.'):
        project, bucket = raw_bucket[len('luci.'):].split('.', 1)
    # Otherwise, assume prefix is also the project name.
    elif '.' in raw_bucket:
        project = raw_bucket.split('.')[0]
        bucket = raw_bucket
    # Legacy buckets.
    if legacy and project and bucket:
        print('WARNING Please use %s/%s to specify the bucket.' %
              (project, bucket))
    return project, bucket


def _canonical_git_googlesource_host(host):
    """Normalizes Gerrit hosts (with '-review') to Git host."""
    assert host.endswith(_GOOGLESOURCE)
    # Prefix doesn't include '.' at the end.
    prefix = host[:-(1 + len(_GOOGLESOURCE))]
    if prefix.endswith('-review'):
        prefix = prefix[:-len('-review')]
    return prefix + '.' + _GOOGLESOURCE


def _canonical_gerrit_googlesource_host(host):
    git_host = _canonical_git_googlesource_host(host)
    prefix = git_host.split('.', 1)[0]
    return prefix + '-review.' + _GOOGLESOURCE


def _get_counterpart_host(host):
    assert host.endswith(_GOOGLESOURCE)
    git = _canonical_git_googlesource_host(host)
    gerrit = _canonical_gerrit_googlesource_host(git)
    return git if gerrit == host else gerrit


def _trigger_tryjobs(changelist, jobs, options, patchset):
    """Sends a request to Buildbucket to trigger tryjobs for a changelist.

    Args:
        changelist: Changelist that the tryjobs are associated with.
        jobs: A list of (project, bucket, builder).
        options: Command-line options.
    """
    print('Scheduling jobs on:')
    for project, bucket, builder in jobs:
        print('  %s/%s: %s' % (project, bucket, builder))
    print('To see results here, run:        git cl try-results')
    print('To see results in browser, run:  git cl web')

    requests = _make_tryjob_schedule_requests(changelist, jobs, options,
                                              patchset)
    if not requests:
        return

    http = auth.Authenticator().authorize(httplib2.Http())
    http.force_exception_to_status_code = True

    batch_request = {'requests': requests}
    batch_response = _call_buildbucket(http, DEFAULT_BUILDBUCKET_HOST, 'Batch',
                                       batch_request)

    errors = [
        '  ' + response['error']['message']
        for response in batch_response.get('responses', [])
        if 'error' in response
    ]
    if errors:
        raise BuildbucketResponseException(
            'Failed to schedule builds for some bots:\n%s' % '\n'.join(errors))


def _make_tryjob_schedule_requests(changelist, jobs, options, patchset):
    """Constructs requests for Buildbucket to trigger tryjobs."""
    gerrit_changes = [changelist.GetGerritChange(patchset)]
    shared_properties = {
        'category': options.ensure_value('category', 'git_cl_try')
    }
    shared_properties.update(_get_properties_from_options(options) or {})

    shared_tags = [{'key': 'user_agent', 'value': 'git_cl_try'}]
    if options.ensure_value('retry_failed', False):
        shared_tags.append({'key': 'retry_failed', 'value': '1'})

    requests = []
    for (project, bucket, builder) in jobs:
        properties = shared_properties.copy()
        if 'presubmit' in builder.lower():
            properties['dry_run'] = 'true'

        requests.append({
            'scheduleBuild': {
                'requestId': str(uuid.uuid4()),
                'builder': {
                    'project': getattr(options, 'project', None) or project,
                    'bucket': bucket,
                    'builder': builder,
                },
                'gerritChanges': gerrit_changes,
                'properties': properties,
                'tags': [
                    {
                        'key': 'builder',
                        'value': builder
                    },
                ] + shared_tags,
            }
        })

        if options.ensure_value('revision', None):
            remote, remote_branch = changelist.GetRemoteBranch()
            requests[-1]['scheduleBuild']['gitilesCommit'] = {
                'host':
                _canonical_git_googlesource_host(gerrit_changes[0]['host']),
                'project': gerrit_changes[0]['project'],
                'id': options.revision,
                'ref': GetTargetRef(remote, remote_branch, None)
            }

    return requests


def _fetch_tryjobs(changelist, buildbucket_host, patchset=None):
    """Fetches tryjobs from buildbucket.

    Returns list of buildbucket.v2.Build with the try jobs for the changelist.
    """
    fields = ['id', 'builder', 'status', 'createTime', 'tags']
    request = {
        'predicate': {
            'gerritChanges': [changelist.GetGerritChange(patchset)],
        },
        'fields': ','.join('builds.*.' + field for field in fields),
    }

    authenticator = auth.Authenticator()
    if authenticator.has_cached_credentials():
        http = authenticator.authorize(httplib2.Http())
    else:
        print('Warning: Some results might be missing because %s' %
              # Get the message on how to login.
              (
                  str(auth.LoginRequiredError()), ))
        http = httplib2.Http()
    http.force_exception_to_status_code = True

    response = _call_buildbucket(http, buildbucket_host, 'SearchBuilds',
                                 request)
    return response.get('builds', [])


def _fetch_latest_builds(changelist, buildbucket_host, latest_patchset=None):
    """Fetches builds from the latest patchset that has builds (within
    the last few patchsets).

    Args:
        changelist (Changelist): The CL to fetch builds for
        buildbucket_host (str): Buildbucket host, e.g. "cr-buildbucket.appspot.com"
        lastest_patchset(int|NoneType): the patchset to start fetching builds from.
            If None (default), starts with the latest available patchset.
    Returns:
        A tuple (builds, patchset) where builds is a list of buildbucket.v2.Build,
        and patchset is the patchset number where those builds came from.
    """
    assert buildbucket_host
    assert changelist.GetIssue(), 'CL must be uploaded first'
    assert changelist.GetCodereviewServer(), 'CL must be uploaded first'
    if latest_patchset is None:
        assert changelist.GetMostRecentPatchset()
        ps = changelist.GetMostRecentPatchset()
    else:
        assert latest_patchset > 0, latest_patchset
        ps = latest_patchset

    min_ps = max(1, ps - 5)
    while ps >= min_ps:
        builds = _fetch_tryjobs(changelist, buildbucket_host, patchset=ps)
        if len(builds):
            return builds, ps
        ps -= 1
    return [], 0


def _filter_failed_for_retry(all_builds):
    """Returns a list of buckets/builders that are worth retrying.

    Args:
        all_builds (list): Builds, in the format returned by _fetch_tryjobs,
            i.e. a list of buildbucket.v2.Builds which includes status and builder
            info.

    Returns:
        A dict {(proj, bucket): [builders]}. This is the same format accepted by
        _trigger_tryjobs.
    """
    grouped = {}
    for build in all_builds:
        builder = build['builder']
        key = (builder['project'], builder['bucket'], builder['builder'])
        grouped.setdefault(key, []).append(build)

    jobs = []
    for (project, bucket, builder), builds in grouped.items():
        if 'triggered' in builder:
            print(
                'WARNING: Not scheduling %s. Triggered bots require an initial job '
                'from a parent. Please schedule a manual job for the parent '
                'instead.')
            continue
        if any(b['status'] in ('STARTED', 'SCHEDULED') for b in builds):
            # Don't retry if any are running.
            continue
        # If builder had several builds, retry only if the last one failed.
        # This is a bit different from CQ, which would re-use *any* SUCCESS-full
        # build, but in case of retrying failed jobs retrying a flaky one makes
        # sense.
        builds = sorted(builds, key=lambda b: b['createTime'])
        if builds[-1]['status'] not in ('FAILURE', 'INFRA_FAILURE'):
            continue
        # Don't retry experimental build previously triggered by CQ.
        if any(t['key'] == 'cq_experimental' and t['value'] == 'true'
               for t in builds[-1]['tags']):
            continue
        jobs.append((project, bucket, builder))

    # Sort the jobs to make testing easier.
    return sorted(jobs)


def _print_tryjobs(options, builds):
    """Prints nicely result of _fetch_tryjobs."""
    if not builds:
        print('No tryjobs scheduled.')
        return

    longest_builder = max(len(b['builder']['builder']) for b in builds)
    name_fmt = '{builder:<%d}' % longest_builder
    if options.print_master:
        longest_bucket = max(len(b['builder']['bucket']) for b in builds)
        name_fmt = ('{bucket:>%d} ' % longest_bucket) + name_fmt

    builds_by_status = {}
    for b in builds:
        builds_by_status.setdefault(b['status'], []).append({
            'id':
            b['id'],
            'name':
            name_fmt.format(builder=b['builder']['builder'],
                            bucket=b['builder']['bucket']),
        })

    sort_key = lambda b: (b['name'], b['id'])

    def print_builds(title, builds, fmt=None, color=None):
        """Pop matching builds from `builds` dict and print them."""
        if not builds:
            return

        fmt = fmt or '{name} https://ci.chromium.org/b/{id}'
        if not options.color or color is None:
            colorize = lambda x: x
        else:
            colorize = lambda x: '%s%s%s' % (color, x, Fore.RESET)

        print(colorize(title))
        for b in sorted(builds, key=sort_key):
            print(' ', colorize(fmt.format(**b)))

    total = len(builds)
    print_builds('Successes:',
                 builds_by_status.pop('SUCCESS', []),
                 color=Fore.GREEN)
    print_builds('Infra Failures:',
                 builds_by_status.pop('INFRA_FAILURE', []),
                 color=Fore.MAGENTA)
    print_builds('Failures:',
                 builds_by_status.pop('FAILURE', []),
                 color=Fore.RED)
    print_builds('Canceled:',
                 builds_by_status.pop('CANCELED', []),
                 fmt='{name}',
                 color=Fore.MAGENTA)
    print_builds('Started:',
                 builds_by_status.pop('STARTED', []),
                 color=Fore.YELLOW)
    print_builds('Scheduled:',
                 builds_by_status.pop('SCHEDULED', []),
                 fmt='{name} id={id}')
    # The last section is just in case buildbucket API changes OR there is a
    # bug.
    print_builds('Other:',
                 sum(builds_by_status.values(), []),
                 fmt='{name} id={id}')
    print('Total: %d tryjobs' % total)


def _ComputeFormatDiffLineRanges(files, upstream_commit, expand=0):
    """Gets the changed line ranges for each file since upstream_commit.

    Parses a git diff on provided files and returns a dict that maps a file name
    to an ordered list of range tuples in the form (start_line, count).
    Ranges are in the same format as a git diff.

    Args:
        files: List of paths to diff.
        upstream_commit: Commit to diff against to find changed lines.
        expand: Expand diff ranges by this many lines before & after.

    Returns:
        A dict of path->[(start_line, end_line), ...]
    """
    # If files is empty then diff_output will be a full diff.
    if len(files) == 0:
        return {}

    # Take the git diff and find the line ranges where there are changes.
    diff_output = RunGitDiffCmd(['-U0'],
                                upstream_commit,
                                files,
                                allow_prefix=True)

    pattern = r'(?:^diff --git a/(?:.*) b/(.*))|(?:^@@.*\+(.*) @@)'
    # 2 capture groups
    # 0 == fname of diff file
    # 1 == 'diff_start,diff_count' or 'diff_start'
    # will match each of
    # diff --git a/foo.foo b/foo.py
    # @@ -12,2 +14,3 @@
    # @@ -12,2 +17 @@
    # running re.findall on the above string with pattern will give
    # [('foo.py', ''), ('', '14,3'), ('', '17')]

    curr_file = None
    line_diffs = {}
    for match in re.findall(pattern, diff_output, flags=re.MULTILINE):
        if match[0] != '':
            # Will match the second filename in diff --git a/a.py b/b.py.
            curr_file = match[0]
            line_diffs[curr_file] = []
            prev_end = 1
        else:
            # Matches +14,3
            if ',' in match[1]:
                diff_start, diff_count = match[1].split(',')
            else:
                # Single line changes are of the form +12 instead of +12,1.
                diff_start = match[1]
                diff_count = 1

            # if the original lines were removed without replacements,
            # the diff count is 0. Then, no formatting is necessary.
            if diff_count == 0:
                continue

            diff_start = int(diff_start)
            diff_count = int(diff_count)
            # diff_count contains the diff_start line, and the line numbers
            # given to formatter args are inclusive. For example, in
            # google-java-format "--lines 5:10" includes 5th-10th lines.
            diff_end = diff_start + diff_count - 1 + expand
            diff_start = max(prev_end + 1, diff_start - expand)
            if diff_start <= diff_end:
                prev_end = diff_end
                line_diffs[curr_file].append((diff_start, diff_end))

    return line_diffs


def _FindYapfConfigFile(fpath, yapf_config_cache, top_dir=None):
    """Checks if a yapf file is in any parent directory of fpath until top_dir.

    Recursively checks parent directories to find yapf file and if no yapf file
    is found returns None. Uses yapf_config_cache as a cache for previously found
    configs.
    """
    fpath = os.path.abspath(fpath)
    # Return result if we've already computed it.
    if fpath in yapf_config_cache:
        return yapf_config_cache[fpath]

    parent_dir = os.path.dirname(fpath)
    if os.path.isfile(fpath):
        ret = _FindYapfConfigFile(parent_dir, yapf_config_cache, top_dir)
    else:
        # Otherwise fpath is a directory
        yapf_file = os.path.join(fpath, YAPF_CONFIG_FILENAME)
        if os.path.isfile(yapf_file):
            ret = yapf_file
        elif fpath in (top_dir, parent_dir):
            # If we're at the top level directory, or if we're at root
            # there is no provided style.
            ret = None
        else:
            # Otherwise recurse on the current directory.
            ret = _FindYapfConfigFile(parent_dir, yapf_config_cache, top_dir)
    yapf_config_cache[fpath] = ret
    return ret


def _GetYapfIgnorePatterns(top_dir):
    """Returns all patterns in the .yapfignore file.

    yapf is supposed to handle the ignoring of files listed in .yapfignore itself,
    but this functionality appears to break when explicitly passing files to
    yapf for formatting. According to
    https://github.com/google/yapf/blob/HEAD/README.rst#excluding-files-from-formatting-yapfignore,
    the .yapfignore file should be in the directory that yapf is invoked from,
    which we assume to be the top level directory in this case.

    Args:
        top_dir: The top level directory for the repository being formatted.

    Returns:
        A set of all fnmatch patterns to be ignored.
    """
    yapfignore_file = os.path.join(top_dir, '.yapfignore')
    ignore_patterns = set()
    if not os.path.exists(yapfignore_file):
        return ignore_patterns

    for line in gclient_utils.FileRead(yapfignore_file).split('\n'):
        stripped_line = line.strip()
        # Comments and blank lines should be ignored.
        if stripped_line.startswith('#') or stripped_line == '':
            continue
        ignore_patterns.add(stripped_line)
    return ignore_patterns


def _FilterYapfIgnoredFiles(filepaths, patterns):
    """Filters out any filepaths that match any of the given patterns.

    Args:
        filepaths: An iterable of strings containing filepaths to filter.
        patterns: An iterable of strings containing fnmatch patterns to filter on.

    Returns:
        A list of strings containing all the elements of |filepaths| that did not
        match any of the patterns in |patterns|.
    """
    # Not inlined so that tests can use the same implementation.
    return [
        f for f in filepaths
        if not any(fnmatch.fnmatch(f, p) for p in patterns)
    ]


def _GetCommitCountSummary(begin_commit: str,
                           end_commit: str = "HEAD") -> Optional[str]:
    """Generate a summary of the number of commits in (begin_commit, end_commit).

    Returns a string containing the summary, or None if the range is empty.
    """
    count = int(
        RunGitSilent(['rev-list', '--count', f'{begin_commit}..{end_commit}']))

    if not count:
        return None

    return f'{count} commit{"s"[:count!=1]}'


def print_stats(args):
    """Prints statistics about the change to the user."""
    # --no-ext-diff is broken in some versions of Git, so try to work around
    # this by overriding the environment (but there is still a problem if the
    # git config key "diff.external" is used).
    env = GetNoGitPagerEnv()
    if 'GIT_EXTERNAL_DIFF' in env:
        del env['GIT_EXTERNAL_DIFF']

    return subprocess2.call(
        ['git', 'diff', '--no-ext-diff', '--stat', '-l100000', '-C50'] + args,
        env=env)


class BuildbucketResponseException(Exception):
    pass


class Settings(object):
    def __init__(self):
        self.cc = None
        self.root = None
        self.tree_status_url = None
        self.viewvc_url = None
        self.updated = False
        self.is_gerrit = None
        self.squash_gerrit_uploads = None
        self.gerrit_skip_ensure_authenticated = None
        self.git_editor = None
        self.format_full_by_default = None
        self.is_status_commit_order_by_date = None

    def _LazyUpdateIfNeeded(self):
        """Updates the settings from a codereview.settings file, if available."""
        if self.updated:
            return

        # The only value that actually changes the behavior is
        # autoupdate = "false". Everything else means "true".
        autoupdate = (scm.GIT.GetConfig(self.GetRoot(), 'rietveld.autoupdate',
                                        '').lower())

        cr_settings_file = FindCodereviewSettingsFile()
        if autoupdate != 'false' and cr_settings_file:
            LoadCodereviewSettingsFromFile(cr_settings_file)
            cr_settings_file.close()

        self.updated = True

    @staticmethod
    def GetRelativeRoot():
        return scm.GIT.GetCheckoutRoot('.')

    def GetRoot(self):
        if self.root is None:
            self.root = os.path.abspath(self.GetRelativeRoot())
        return self.root

    def GetTreeStatusUrl(self, error_ok=False):
        if not self.tree_status_url:
            self.tree_status_url = self._GetConfig('rietveld.tree-status-url')
            if self.tree_status_url is None and not error_ok:
                DieWithError(
                    'You must configure your tree status URL by running '
                    '"git cl config".')
        return self.tree_status_url

    def GetViewVCUrl(self) -> str:
        if not self.viewvc_url:
            self.viewvc_url = self._GetConfig('rietveld.viewvc-url')
        return self.viewvc_url

    def GetBugPrefix(self):
        return self._GetConfig('rietveld.bug-prefix')

    def GetRunPostUploadHook(self):
        run_post_upload_hook = self._GetConfig('rietveld.run-post-upload-hook')
        return run_post_upload_hook == "True"

    def GetDefaultCCList(self):
        return self._GetConfig('rietveld.cc')

    def GetSquashGerritUploads(self):
        """Returns True if uploads to Gerrit should be squashed by default."""
        if self.squash_gerrit_uploads is None:
            self.squash_gerrit_uploads = self.GetSquashGerritUploadsOverride()
        if self.squash_gerrit_uploads is None:
            # Default is squash now (http://crbug.com/611892#c23).
            self.squash_gerrit_uploads = self._GetConfig(
                'gerrit.squash-uploads').lower() != 'false'
        return self.squash_gerrit_uploads

    def GetSquashGerritUploadsOverride(self):
        """Return True or False if codereview.settings should be overridden.

        Returns None if no override has been defined.
        """
        # See also http://crbug.com/611892#c23
        result = self._GetConfig('gerrit.override-squash-uploads').lower()
        if result == 'true':
            return True
        if result == 'false':
            return False
        return None

    def GetIsGerrit(self):
        """Return True if gerrit.host is set."""
        if self.is_gerrit is None:
            self.is_gerrit = bool(self._GetConfig('gerrit.host', False))
        return self.is_gerrit

    def GetGerritSkipEnsureAuthenticated(self):
        """Return True if EnsureAuthenticated should not be done for Gerrit
        uploads."""
        if self.gerrit_skip_ensure_authenticated is None:
            self.gerrit_skip_ensure_authenticated = self._GetConfig(
                'gerrit.skip-ensure-authenticated').lower() == 'true'
        return self.gerrit_skip_ensure_authenticated

    def GetGitEditor(self):
        """Returns the editor specified in the git config, or None if none is."""
        if self.git_editor is None:
            # Git requires single quotes for paths with spaces. We need to
            # replace them with double quotes for Windows to treat such paths as
            # a single path.
            self.git_editor = self._GetConfig('core.editor').replace('\'', '"')
        return self.git_editor or None

    def GetLintRegex(self):
        return self._GetConfig('rietveld.cpplint-regex', DEFAULT_LINT_REGEX)

    def GetLintIgnoreRegex(self):
        return self._GetConfig('rietveld.cpplint-ignore-regex',
                               DEFAULT_LINT_IGNORE_REGEX)

    def GetFormatFullByDefault(self):
        if self.format_full_by_default is None:
            self.format_full_by_default = self._GetConfigBool(
                'rietveld.format-full-by-default')
        return self.format_full_by_default

    def IsStatusCommitOrderByDate(self):
        if self.is_status_commit_order_by_date is None:
            self.is_status_commit_order_by_date = self._GetConfigBool(
                'cl.date-order')
        return self.is_status_commit_order_by_date

    def _GetConfig(self, key, default=''):
        self._LazyUpdateIfNeeded()
        return scm.GIT.GetConfig(self.GetRoot(), key, default)

    def _GetConfigBool(self, key) -> bool:
        self._LazyUpdateIfNeeded()
        return scm.GIT.GetConfigBool(self.GetRoot(), key)


settings = Settings()


class _CQState(object):
    """Enum for states of CL with respect to CQ."""
    NONE = 'none'
    DRY_RUN = 'dry_run'
    COMMIT = 'commit'

    ALL_STATES = [NONE, DRY_RUN, COMMIT]


class _ParsedIssueNumberArgument(object):
    def __init__(self, issue=None, patchset=None, hostname=None):
        self.issue = issue
        self.patchset = patchset
        self.hostname = hostname

    @property
    def valid(self):
        return self.issue is not None


def ParseIssueNumberArgument(arg):
    """Parses the issue argument and returns _ParsedIssueNumberArgument."""
    fail_result = _ParsedIssueNumberArgument()

    if isinstance(arg, int):
        return _ParsedIssueNumberArgument(issue=arg)
    if not isinstance(arg, str):
        return fail_result

    if arg.isdigit():
        return _ParsedIssueNumberArgument(issue=int(arg))

    url = gclient_utils.UpgradeToHttps(arg)
    if not url.startswith('http'):
        return fail_result
    for gerrit_url, short_url in _KNOWN_GERRIT_TO_SHORT_URLS.items():
        if url.startswith(short_url):
            url = gerrit_url + url[len(short_url):]
            break

    try:
        parsed_url = urllib.parse.urlparse(url)
    except ValueError:
        return fail_result

    # If "https://" was automatically added, fail if `arg` looks unlikely to be
    # a URL.
    if not arg.startswith('http') and '.' not in parsed_url.netloc:
        return fail_result

    # Gerrit's new UI is https://domain/c/project/+/<issue_number>[/[patchset]]
    # But old GWT UI is https://domain/#/c/project/+/<issue_number>[/[patchset]]
    # Short urls like https://domain/<issue_number> can be used, but don't allow
    # specifying the patchset (you'd 404), but we allow that here.
    if parsed_url.path == '/':
        part = parsed_url.fragment
    else:
        part = parsed_url.path

    match = re.match(r'(/c(/.*/\+)?)?/(?P<issue>\d+)(/(?P<patchset>\d+)?/?)?$',
                     part)
    if not match:
        return fail_result

    issue = int(match.group('issue'))
    patchset = match.group('patchset')
    return _ParsedIssueNumberArgument(
        issue=issue,
        patchset=int(patchset) if patchset else None,
        hostname=parsed_url.netloc)


def _create_description_from_log(args):
    """Pulls out the commit log to use as a base for the CL description."""
    log_args = []
    if len(args) == 1 and args[0] == None:
        # Handle the case where None is passed as the branch.
        return ''
    if len(args) == 1 and not args[0].endswith('.'):
        log_args = [args[0] + '..']
    elif len(args) == 1 and args[0].endswith('...'):
        log_args = [args[0][:-1]]
    elif len(args) == 2:
        log_args = [args[0] + '..' + args[1]]
    else:
        log_args = args[:]  # Hope for the best!
    return RunGit(['log', '--pretty=format:%B%n'] + log_args)


class GerritChangeNotExists(Exception):
    def __init__(self, issue, url):
        self.issue = issue
        self.url = url
        super(GerritChangeNotExists, self).__init__()

    def __str__(self):
        return 'change %s at %s does not exist or you have no access to it' % (
            self.issue, self.url)


_CommentSummary = collections.namedtuple(
    '_CommentSummary',
    [
        'date',
        'message',
        'sender',
        'autogenerated',
        # TODO(tandrii): these two aren't known in Gerrit.
        'approval',
        'disapproval'
    ])

# TODO(b/265929888): Change `parent` to `pushed_commit_base`.
_NewUpload = collections.namedtuple('NewUpload', [
    'reviewers', 'ccs', 'commit_to_push', 'new_last_uploaded_commit', 'parent',
    'change_desc', 'prev_patchset'
])


class ChangeDescription(object):
    """Contains a parsed form of the change description."""
    R_LINE = r'^[ \t]*(TBR|R)[ \t]*=[ \t]*(.*?)[ \t]*$'
    CC_LINE = r'^[ \t]*(CC)[ \t]*=[ \t]*(.*?)[ \t]*$'
    BUG_LINE = r'^[ \t]*(?:(BUG)[ \t]*=|Bug:)[ \t]*(.*?)[ \t]*$'
    FIXED_LINE = r'^[ \t]*Fixed[ \t]*:[ \t]*(.*?)[ \t]*$'
    CHERRY_PICK_LINE = r'^\(cherry picked from commit [a-fA-F0-9]{40}\)$'
    STRIP_HASH_TAG_PREFIX = r'^(\s*(revert|reland)( "|:)?\s*)*'
    BRACKET_HASH_TAG = r'\s*\[([^\[\]]+)\]'
    COLON_SEPARATED_HASH_TAG = r'^([a-zA-Z0-9_\- ]+):($|[^:])'
    BAD_HASH_TAG_CHUNK = r'[^a-zA-Z0-9]+'

    def __init__(self, description, bug=None, fixed=None):
        self._description_lines = (description or '').strip().splitlines()
        if bug:
            regexp = re.compile(self.BUG_LINE)
            prefix = settings.GetBugPrefix()
            if not any(
                (regexp.match(line) for line in self._description_lines)):
                values = list(_get_bug_line_values(prefix, bug))
                self.append_footer('Bug: %s' % ', '.join(values))
        if fixed:
            regexp = re.compile(self.FIXED_LINE)
            prefix = settings.GetBugPrefix()
            if not any(
                (regexp.match(line) for line in self._description_lines)):
                values = list(_get_bug_line_values(prefix, fixed))
                self.append_footer('Fixed: %s' % ', '.join(values))

    @property  # www.logilab.org/ticket/89786
    def description(self):  # pylint: disable=method-hidden
        return '\n'.join(self._description_lines)

    def set_description(self, desc):
        if isinstance(desc, str):
            lines = desc.splitlines()
        else:
            lines = [line.rstrip() for line in desc]
        while lines and not lines[0]:
            lines.pop(0)
        while lines and not lines[-1]:
            lines.pop(-1)
        self._description_lines = lines

    def ensure_change_id(self, change_id):
        description = self.description
        footer_change_ids = git_footers.get_footer_change_id(description)
        # Make sure that the Change-Id in the description matches the given one.
        if footer_change_ids != [change_id]:
            if footer_change_ids:
                # Remove any existing Change-Id footers since they don't match
                # the expected change_id footer.
                description = git_footers.remove_footer(description,
                                                        'Change-Id')
                print(
                    'WARNING: Change-Id has been set to %s. Use `git cl issue 0` '
                    'if you want to set a new one.')
            # Add the expected Change-Id footer.
            description = git_footers.add_footer_change_id(
                description, change_id)
            self.set_description(description)

    def update_reviewers(self, reviewers):
        """Rewrites the R= line(s) as a single line each.

        Args:
            reviewers (list(str)) - list of additional emails to use for reviewers.
        """
        if not reviewers:
            return

        reviewers = set(reviewers)

        # Get the set of R= lines and remove them from the description.
        regexp = re.compile(self.R_LINE)
        matches = [regexp.match(line) for line in self._description_lines]
        new_desc = [
            l for i, l in enumerate(self._description_lines) if not matches[i]
        ]
        self.set_description(new_desc)

        # Construct new unified R= lines.

        # First, update reviewers with names from the R= lines (if any).
        for match in matches:
            if not match:
                continue
            reviewers.update(cleanup_list([match.group(2).strip()]))

        new_r_line = 'R=' + ', '.join(sorted(reviewers))

        # Put the new lines in the description where the old first R= line was.
        line_loc = next((i for i, match in enumerate(matches) if match), -1)
        if 0 <= line_loc < len(self._description_lines):
            self._description_lines.insert(line_loc, new_r_line)
        else:
            self.append_footer(new_r_line)

    def set_preserve_tryjobs(self):
        """Ensures description footer contains 'Cq-Do-Not-Cancel-Tryjobs: true'."""
        footers = git_footers.parse_footers(self.description)
        for v in footers.get('Cq-Do-Not-Cancel-Tryjobs', []):
            if v.lower() == 'true':
                return
        self.append_footer('Cq-Do-Not-Cancel-Tryjobs: true')

    def prompt(self):
        """Asks the user to update the description."""
        self.set_description([
            '# Enter a description of the change.',
            '# This will be displayed on the codereview site.',
            '# The first line will also be used as the subject of the review.',
            '#--------------------This line is 72 characters long'
            '--------------------',
        ] + self._description_lines)
        bug_regexp = re.compile(self.BUG_LINE)
        fixed_regexp = re.compile(self.FIXED_LINE)
        prefix = settings.GetBugPrefix()
        has_issue = lambda l: bug_regexp.match(l) or fixed_regexp.match(l)

        if not any((has_issue(line) for line in self._description_lines)):
            self.append_footer('Bug: %s' % prefix)

        print('Waiting for editor...')
        content = gclient_utils.RunEditor(self.description,
                                          True,
                                          git_editor=settings.GetGitEditor())
        if not content:
            DieWithError('Running editor failed')
        lines = content.splitlines()

        # Strip off comments and default inserted "Bug:" line.
        clean_lines = [
            line.rstrip() for line in lines
            if not (line.startswith('#') or line.rstrip() == "Bug:"
                    or line.rstrip() == "Bug: " + prefix)
        ]
        if not clean_lines:
            DieWithError('No CL description, aborting')
        self.set_description(clean_lines)

    def append_footer(self, line):
        """Adds a footer line to the description.

        Differentiates legacy "KEY=xxx" footers (used to be called tags) and
        Gerrit's footers in the form of "Footer-Key: footer any value" and ensures
        that Gerrit footers are always at the end.
        """
        parsed_footer_line = git_footers.parse_footer(line)
        if parsed_footer_line:
            # Line is a gerrit footer in the form: Footer-Key: any value.
            # Thus, must be appended observing Gerrit footer rules.
            self.set_description(
                git_footers.add_footer(self.description,
                                       key=parsed_footer_line[0],
                                       value=parsed_footer_line[1]))
            return

        if not self._description_lines:
            self._description_lines.append(line)
            return

        top_lines, gerrit_footers, _ = git_footers.split_footers(
            self.description)
        if gerrit_footers:
            # git_footers.split_footers ensures that there is an empty line
            # before actual (gerrit) footers, if any. We have to keep it that
            # way.
            assert top_lines and top_lines[-1] == ''
            top_lines, separator = top_lines[:-1], top_lines[-1:]
        else:
            separator = [
            ]  # No need for separator if there are no gerrit_footers.

        prev_line = top_lines[-1] if top_lines else ''
        if (not presubmit_support.Change.TAG_LINE_RE.match(prev_line)
                or not presubmit_support.Change.TAG_LINE_RE.match(line)):
            top_lines.append('')
        top_lines.append(line)
        self._description_lines = top_lines + separator + gerrit_footers

    def get_reviewers(self, tbr_only=False):
        """Retrieves the list of reviewers."""
        matches = [
            re.match(self.R_LINE, line) for line in self._description_lines
        ]
        reviewers = [
            match.group(2).strip() for match in matches
            if match and (not tbr_only or match.group(1).upper() == 'TBR')
        ]
        return cleanup_list(reviewers)

    def get_cced(self):
        """Retrieves the list of reviewers."""
        matches = [
            re.match(self.CC_LINE, line) for line in self._description_lines
        ]
        cced = [match.group(2).strip() for match in matches if match]
        return cleanup_list(cced)

    def get_hash_tags(self):
        """Extracts and sanitizes a list of Gerrit hashtags."""
        subject = (self._description_lines or ('', ))[0]
        subject = re.sub(self.STRIP_HASH_TAG_PREFIX,
                         '',
                         subject,
                         flags=re.IGNORECASE)

        tags = []
        start = 0
        bracket_exp = re.compile(self.BRACKET_HASH_TAG)
        while True:
            m = bracket_exp.match(subject, start)
            if not m:
                break
            tags.append(self.sanitize_hash_tag(m.group(1)))
            start = m.end()

        if not tags:
            # Try "Tag: " prefix.
            m = re.match(self.COLON_SEPARATED_HASH_TAG, subject)
            if m:
                tags.append(self.sanitize_hash_tag(m.group(1)))
        return tags

    @classmethod
    def sanitize_hash_tag(cls, tag):
        """Returns a sanitized Gerrit hash tag.

        A sanitized hashtag can be used as a git push refspec parameter value.
        """
        return re.sub(cls.BAD_HASH_TAG_CHUNK, '-', tag).strip('-').lower()


class Changelist(object):
    """Changelist works with one changelist in local branch.

    Notes:
        * Not safe for concurrent multi-{thread,process} use.
        * Caches values from current branch. Therefore, re-use after branch change
        with great care.
    """
    def __init__(self,
                 branchref=None,
                 issue=None,
                 codereview_host=None,
                 commit_date=None):
        """Create a new ChangeList instance.

        **kwargs will be passed directly to Gerrit implementation.
        """
        self.branchref = branchref
        if self.branchref:
            assert (branchref.startswith(('refs/heads/', 'refs/branch-heads/'))
                    or branchref == 'refs/meta/config')
            self.branch = scm.GIT.ShortBranchName(self.branchref)
        else:
            self.branch = None
        self.commit_date = commit_date
        self.upstream_branch = None
        self.lookedup_issue = False
        self.issue = issue or None
        self.description = None
        self.lookedup_patchset = False
        self.patchset = None
        self.cc = None
        self.more_cc = []
        self._remote = None
        self._cached_remote_url = (False, None)  # (is_cached, value)

        # Lazily cached values.
        self._gerrit_host = None  # e.g. chromium-review.googlesource.com
        self._gerrit_server = None  # e.g. https://chromium-review.googlesource.com
        self._owners_client = None
        # Map from change number (issue) to its detail cache.
        self._detail_cache = {}

        if codereview_host is not None:
            assert not codereview_host.startswith('https://'), codereview_host
            self._gerrit_host = codereview_host
            self._gerrit_server = 'https://%s' % codereview_host

    @property
    def owners_client(self):
        if self._owners_client is None:
            remote, remote_branch = self.GetRemoteBranch()
            branch = GetTargetRef(remote, remote_branch, None)
            self._owners_client = owners_client.GetCodeOwnersClient(
                host=self.GetGerritHost(),
                project=self.GetGerritProject(),
                branch=branch)
        return self._owners_client

    def GetCCList(self):
        """Returns the users cc'd on this CL.

        The return value is a string suitable for passing to git cl with the --cc
        flag.
        """
        if self.cc is None:
            base_cc = settings.GetDefaultCCList()
            more_cc = ','.join(self.more_cc)
            self.cc = ','.join(filter(None, (base_cc, more_cc))) or ''
        return self.cc

    def ExtendCC(self, more_cc):
        """Extends the list of users to cc on this CL based on the changed files."""
        self.more_cc.extend(more_cc)

    def GetCommitDate(self):
        """Returns the commit date as provided in the constructor"""
        return self.commit_date

    def GetBranch(self):
        """Returns the short branch name, e.g. 'main'."""
        if not self.branch:
            branchref = scm.GIT.GetBranchRef(settings.GetRoot())
            if not branchref:
                return None
            self.branchref = branchref
            self.branch = scm.GIT.ShortBranchName(self.branchref)
        return self.branch

    def GetBranchRef(self):
        """Returns the full branch name, e.g. 'refs/heads/main'."""
        self.GetBranch()  # Poke the lazy loader.
        return self.branchref

    def _GitGetBranchConfigValue(self, key, default=None):
        return scm.GIT.GetBranchConfig(settings.GetRoot(), self.GetBranch(),
                                       key, default)

    def _GitSetBranchConfigValue(self, key, value):
        action = 'set %s to %r' % (key, value)
        if not value:
            action = 'unset %s' % key
        assert self.GetBranch(), 'a branch is needed to ' + action
        return scm.GIT.SetBranchConfig(settings.GetRoot(), self.GetBranch(),
                                       key, value)

    @staticmethod
    def FetchUpstreamTuple(branch):
        """Returns a tuple containing remote and remote ref,
        e.g. 'origin', 'refs/heads/main'
        """
        remote, upstream_branch = scm.GIT.FetchUpstreamTuple(
            settings.GetRoot(), branch)
        if not remote or not upstream_branch:
            DieWithError(
                'Unable to determine default branch to diff against.\n'
                'Verify this branch is set up to track another \n'
                '(via the --track argument to "git checkout -b ..."). \n'
                'or pass complete "git diff"-style arguments if supported, like\n'
                '  git cl upload origin/main\n')

        return remote, upstream_branch

    def GetCommonAncestorWithUpstream(self):
        upstream_branch = self.GetUpstreamBranch()
        if not scm.GIT.IsValidRevision(settings.GetRoot(), upstream_branch):
            DieWithError(
                'The current branch (%s) has an upstream (%s) that does not exist '
                'anymore.\nPlease fix it and try again.' %
                (self.GetBranch(), upstream_branch))
        return git_common.get_or_create_merge_base(self.GetBranch(),
                                                   upstream_branch)

    def GetUpstreamBranch(self):
        if self.upstream_branch is None:
            remote, upstream_branch = self.FetchUpstreamTuple(self.GetBranch())
            if remote != '.':
                upstream_branch = upstream_branch.replace(
                    'refs/heads/', 'refs/remotes/%s/' % remote)
                upstream_branch = upstream_branch.replace(
                    'refs/branch-heads/', 'refs/remotes/branch-heads/')
            self.upstream_branch = upstream_branch
        return self.upstream_branch

    def GetRemoteBranch(self):
        if not self._remote:
            remote, branch = None, self.GetBranch()
            seen_branches = set()
            while branch not in seen_branches:
                seen_branches.add(branch)
                remote, branch = self.FetchUpstreamTuple(branch)
                branch = scm.GIT.ShortBranchName(branch)
                if remote != '.' or branch.startswith('refs/remotes'):
                    break
            else:
                remotes = RunGit(['remote'], error_ok=True).split()
                if len(remotes) == 1:
                    remote, = remotes
                elif 'origin' in remotes:
                    remote = 'origin'
                    logging.warning(
                        'Could not determine which remote this change is '
                        'associated with, so defaulting to "%s".' %
                        self._remote)
                else:
                    logging.warning(
                        'Could not determine which remote this change is '
                        'associated with.')
                branch = 'HEAD'
            if branch.startswith('refs/remotes'):
                self._remote = (remote, branch)
            elif branch.startswith('refs/branch-heads/'):
                self._remote = (remote, branch.replace('refs/',
                                                       'refs/remotes/'))
            else:
                self._remote = (remote, 'refs/remotes/%s/%s' % (remote, branch))
        return self._remote

    def GetRemoteUrl(self) -> Optional[str]:
        """Return the configured remote URL, e.g. 'git://example.org/foo.git/'.

        Returns None if there is no remote.
        """
        is_cached, value = self._cached_remote_url
        if is_cached:
            return value

        remote, _ = self.GetRemoteBranch()
        url = scm.GIT.GetConfig(settings.GetRoot(), 'remote.%s.url' % remote,
                                '')

        # Check if the remote url can be parsed as an URL.
        host = urllib.parse.urlparse(url).netloc
        if host:
            self._cached_remote_url = (True, url)
            return url

        # If it cannot be parsed as an url, assume it is a local directory,
        # probably a git cache.
        logging.warning(
            '"%s" doesn\'t appear to point to a git host. '
            'Interpreting it as a local directory.', url)
        if not os.path.isdir(url):
            logging.error(
                'Remote "%(remote)s" for branch "%(branch)s" points to "%(url)s", '
                'but it doesn\'t exist.', {
                    'remote': remote,
                    'branch': self.GetBranch(),
                    'url': url
                })
            return None

        cache_path = url
        url = scm.GIT.GetConfig(url, 'remote.%s.url' % remote, '')

        host = urllib.parse.urlparse(url).netloc
        if not host:
            logging.error(
                'Remote "%(remote)s" for branch "%(branch)s" points to '
                '"%(cache_path)s", but it is misconfigured.\n'
                '"%(cache_path)s" must be a git repo and must have a remote named '
                '"%(remote)s" pointing to the git host.', {
                    'remote': remote,
                    'cache_path': cache_path,
                    'branch': self.GetBranch()
                })
            return None

        self._cached_remote_url = (True, url)
        return url

    def _GetIssueFromTripletId(self):
        project = self.GetGerritProject()
        remote, branch = self.GetRemoteBranch()
        remote_ref = GetTargetRef(remote, branch,
                                  None).replace("refs/heads/", "")
        # _create_description_from_log calls into `git log
        # HEAD^..`. This will exclude everything reachable from `HEAD^`,
        # leaving just `HEAD`.
        change_ids = git_footers.get_footer_change_id(
            _create_description_from_log(["HEAD^"]))
        if len(change_ids) != 1:
            return None
        change_id = change_ids[0]
        triplet_id = urllib.parse.quote("%s~%s~%s" %
                                        (project, remote_ref, change_id),
                                        safe='')

        # GetGerritHost() would attempt to lookup the host from the
        # issue first, leading to infinite recursion. Instead, use the
        # fallback of getting the host from the remote URL.
        gerrit_host = self._GetGerritHostFromRemoteUrl()
        change = gerrit_util.GetChange(gerrit_host,
                                       triplet_id,
                                       accept_statuses=[200, 404])
        return change.get('_number')

    def GetIssue(self):
        """Returns the issue number as a int or None if not set."""
        if self.issue is None and not self.lookedup_issue:
            if self.GetBranch():
                self.issue = self._GitGetBranchConfigValue(ISSUE_CONFIG_KEY)

            if not settings.GetSquashGerritUploads() and self.issue is None:
                self.issue = self._GetIssueFromTripletId()

            if self.issue is not None:
                self.issue = int(self.issue)
            self.lookedup_issue = True
        return self.issue

    def GetIssueURL(self, short=False):
        """Get the URL for a particular issue."""
        issue = self.GetIssue()
        if not issue:
            return None
        server = self.GetCodereviewServer()
        if short:
            server = _KNOWN_GERRIT_TO_SHORT_URLS.get(server, server)
        return '%s/%s' % (server, issue)

    def FetchDescription(self, pretty=False):
        assert self.GetIssue(), 'issue is required to query Gerrit'

        if self.description is None:
            data = self._GetChangeDetail(['CURRENT_REVISION', 'CURRENT_COMMIT'])
            current_rev = data['current_revision']
            self.description = data['revisions'][current_rev]['commit'][
                'message']

        if not pretty:
            return self.description

        # Set width to 72 columns + 2 space indent.
        wrapper = textwrap.TextWrapper(width=74, replace_whitespace=True)
        wrapper.initial_indent = wrapper.subsequent_indent = '  '
        lines = self.description.splitlines()
        return '\n'.join([wrapper.fill(line) for line in lines])

    def GetPatchset(self):
        """Returns the patchset number as a int or None if not set."""
        if self.patchset is None and not self.lookedup_patchset:
            if self.GetBranch():
                self.patchset = self._GitGetBranchConfigValue(
                    PATCHSET_CONFIG_KEY)
            if self.patchset is not None:
                self.patchset = int(self.patchset)
            self.lookedup_patchset = True
        return self.patchset

    def GetAuthor(self):
        return scm.GIT.GetConfig(settings.GetRoot(), 'user.email')

    def SetPatchset(self, patchset):
        """Set this branch's patchset. If patchset=0, clears the patchset."""
        assert self.GetBranch()
        if not patchset:
            self.patchset = None
        else:
            self.patchset = int(patchset)
        self._GitSetBranchConfigValue(PATCHSET_CONFIG_KEY, str(self.patchset))

    def SetIssue(self, issue=None):
        """Set this branch's issue. If issue isn't given, clears the issue."""
        assert self.GetBranch()
        if issue:
            issue = int(issue)
            self._GitSetBranchConfigValue(ISSUE_CONFIG_KEY, str(issue))
            self.issue = issue
            codereview_server = self.GetCodereviewServer()
            if codereview_server:
                self._GitSetBranchConfigValue(CODEREVIEW_SERVER_CONFIG_KEY,
                                              codereview_server)
        else:
            # Reset all of these just to be clean.
            reset_suffixes = [
                LAST_UPLOAD_HASH_CONFIG_KEY,
                ISSUE_CONFIG_KEY,
                PATCHSET_CONFIG_KEY,
                CODEREVIEW_SERVER_CONFIG_KEY,
                GERRIT_SQUASH_HASH_CONFIG_KEY,
            ]
            for prop in reset_suffixes:
                try:
                    self._GitSetBranchConfigValue(prop, None)
                except subprocess2.CalledProcessError:
                    pass
            msg = RunGit(['log', '-1', '--format=%B']).strip()
            if msg and git_footers.get_footer_change_id(msg):
                print(
                    'WARNING: The change patched into this branch has a Change-Id. '
                    'Removing it.')
                RunGit([
                    'commit', '--amend', '-m',
                    git_footers.remove_footer(msg, 'Change-Id')
                ])
            self.lookedup_issue = True
            self.issue = None
            self.patchset = None

    def GetAffectedFiles(self,
                         upstream: str,
                         end_commit: Optional[str] = None) -> Sequence[str]:
        """Returns the list of affected files for the given commit range."""
        try:
            return [
                f for _, f in scm.GIT.CaptureStatus(
                    settings.GetRoot(), upstream, end_commit=end_commit)
            ]
        except subprocess2.CalledProcessError:
            DieWithError(
                ('\nFailed to diff against upstream branch %s\n\n'
                 'This branch probably doesn\'t exist anymore. To reset the\n'
                 'tracking branch, please run\n'
                 '    git branch --set-upstream-to origin/main %s\n'
                 'or replace origin/main with the relevant branch') %
                (upstream, self.GetBranch()))

    def UpdateDescription(self, description, force=False):
        assert self.GetIssue(), 'issue is required to update description'

        if gerrit_util.HasPendingChangeEdit(self.GetGerritHost(),
                                            self._GerritChangeIdentifier()):
            if not force:
                confirm_or_exit(
                    'The description cannot be modified while the issue has a pending '
                    'unpublished edit. Either publish the edit in the Gerrit web UI '
                    'or delete it.\n\n',
                    action='delete the unpublished edit')

            gerrit_util.DeletePendingChangeEdit(self.GetGerritHost(),
                                                self._GerritChangeIdentifier())
        gerrit_util.SetCommitMessage(self.GetGerritHost(),
                                     self._GerritChangeIdentifier(),
                                     description,
                                     notify='NONE')

        self.description = description

    def _GetCommonPresubmitArgs(self, verbose, upstream):
        args = [
            '--root',
            settings.GetRoot(),
            '--upstream',
            upstream,
        ]

        args.extend(['--verbose'] * verbose)

        remote, remote_branch = self.GetRemoteBranch()
        target_ref = GetTargetRef(remote, remote_branch, None)
        if settings.GetIsGerrit():
            args.extend(['--gerrit_url', self.GetCodereviewServer()])
            args.extend(['--gerrit_project', self.GetGerritProject()])
            args.extend(['--gerrit_branch', target_ref])

        author = self.GetAuthor()
        issue = self.GetIssue()
        patchset = self.GetPatchset()
        if author:
            args.extend(['--author', author])
        if issue:
            args.extend(['--issue', str(issue)])
        if patchset:
            args.extend(['--patchset', str(patchset)])

        return args

    def RunHook(self,
                committing: bool,
                may_prompt: bool,
                verbose: bool,
                parallel: bool,
                upstream: str,
                description: str,
                all_files: bool,
                files: Optional[Sequence[str]] = None,
                resultdb: Optional[bool] = None,
                realm: Optional[str] = None,
                end_commit: Optional[str] = None) -> Mapping[str, Any]:
        """Calls sys.exit() if the hook fails; returns a HookResults otherwise."""
        args = self._GetCommonPresubmitArgs(verbose, upstream)
        args.append('--commit' if committing else '--upload')
        if end_commit:
            args.extend(['--end_commit', end_commit])
        if may_prompt:
            args.append('--may_prompt')
        if parallel:
            args.append('--parallel')
        if all_files:
            args.append('--all_files')
        if files:
            args.extend(files.split(';'))
            args.append('--source_controlled_only')
        if files or all_files:
            args.append('--no_diffs')

        if resultdb and not realm:
            # TODO (crbug.com/1113463): store realm somewhere and look it up so
            # it is not required to pass the realm flag
            print(
                'Note: ResultDB reporting will NOT be performed because --realm'
                ' was not specified. To enable ResultDB, please run the command'
                ' again with the --realm argument to specify the LUCI realm.')

        return self._RunPresubmit(args,
                                  description,
                                  resultdb=resultdb,
                                  realm=realm)

    def _RunPresubmit(self,
                      args: Sequence[str],
                      description: str,
                      resultdb: bool = False,
                      realm: Optional[str] = None) -> Mapping[str, Any]:
        args = list(args)

        with gclient_utils.temporary_file() as description_file:
            with gclient_utils.temporary_file() as json_output:
                gclient_utils.FileWrite(description_file, description)
                args.extend(['--json_output', json_output])
                args.extend(['--description_file', description_file])
                start = time_time()
                cmd = ['vpython3', PRESUBMIT_SUPPORT] + args
                if resultdb and realm:
                    cmd = ['rdb', 'stream', '-new', '-realm', realm, '--'] + cmd

                p = subprocess2.Popen(cmd)
                exit_code = p.wait()

                metrics.collector.add_repeated(
                    'sub_commands', {
                        'command': 'presubmit',
                        'execution_time': time_time() - start,
                        'exit_code': exit_code,
                    })

                if exit_code:
                    sys.exit(exit_code)

                json_results = gclient_utils.FileRead(json_output)
                return json.loads(json_results)

    def RunPostUploadHook(self, verbose, upstream, description):
        args = self._GetCommonPresubmitArgs(verbose, upstream)
        args.append('--post_upload')

        with gclient_utils.temporary_file() as description_file:
            gclient_utils.FileWrite(description_file, description)
            args.extend(['--description_file', description_file])
            subprocess2.Popen(['vpython3', PRESUBMIT_SUPPORT] + args).wait()

    def _GetDescriptionForUpload(self, options: optparse.Values,
                                 git_diff_args: Sequence[str],
                                 files: Sequence[str]) -> ChangeDescription:
        """Get description message for upload."""
        if options.commit_description:
            description = options.commit_description
            if description == '-':
                description = '\n'.join(l.rstrip() for l in sys.stdin)
            elif description == '+':
                description = _create_description_from_log(git_diff_args)
        elif self.GetIssue():
            description = self.FetchDescription()
        elif options.message:
            description = options.message
        else:
            description = _create_description_from_log(git_diff_args)
            if options.title and options.squash:
                description = options.title + '\n\n' + description

        bug = options.bug
        fixed = options.fixed
        if not self.GetIssue():
            # Extract bug number from branch name, but only if issue is being
            # created. It must start with bug or fix, followed by _ or - and
            # number. Optionally, it may contain _ or - after number with
            # arbitrary text. Examples: bug-123 bug_123 fix-123
            # fix-123-some-description
            branch = self.GetBranch()
            if branch is not None:
                match = re.match(
                    r'^(?P<type>bug|fix(?:e[sd])?)[_-]?(?P<bugnum>\d+)([-_]|$)',
                    branch)
                if not bug and not fixed and match:
                    if match.group('type') == 'bug':
                        bug = match.group('bugnum')
                    else:
                        fixed = match.group('bugnum')

        change_description = ChangeDescription(description, bug, fixed)

        # Fill gaps in OWNERS coverage to reviewers if requested.
        if options.add_owners_to:
            assert options.add_owners_to in ('R'), options.add_owners_to
            status = self.owners_client.GetFilesApprovalStatus(
                files, [], options.reviewers)
            missing_files = [
                f for f in files
                if status[f] == self._owners_client.INSUFFICIENT_REVIEWERS
            ]
            owners = self.owners_client.SuggestOwners(
                missing_files, exclude=[self.GetAuthor()])
            assert isinstance(options.reviewers, list), options.reviewers
            options.reviewers.extend(owners)

        # Set the reviewer list now so that presubmit checks can access it.
        if options.reviewers:
            change_description.update_reviewers(options.reviewers)

        return change_description

    def _GetTitleForUpload(self, options, multi_change_upload=False):
        # type: (optparse.Values, Optional[bool]) -> str

        # Getting titles for multipl commits is not supported so we return the
        # default.
        if not options.squash or multi_change_upload or options.title:
            return options.title

        # On first upload, patchset title is always this string, while
        # options.title gets converted to first line of message.
        if not self.GetIssue():
            return 'Initial upload'

        # When uploading subsequent patchsets, options.message is taken as the
        # title if options.title is not provided.
        if options.message:
            return options.message.strip()

        # Use the subject of the last commit as title by default.
        title = RunGit(['show', '-s', '--format=%s', 'HEAD', '--']).strip()
        if options.force or options.skip_title:
            return title
        user_title = gclient_utils.AskForData(
            'Title for patchset (\'y\' for default) [%s]: ' % title)

        # Use the default title if the user confirms the default with a 'y'.
        if user_title.lower() == 'y':
            return title
        return user_title or title

    def _GetRefSpecOptions(self,
                           options: optparse.Values,
                           change_desc: ChangeDescription,
                           multi_change_upload: bool = False,
                           dogfood_path: bool = False) -> List[str]:
        # Extra options that can be specified at push time. Doc:
        # https://gerrit-review.googlesource.com/Documentation/user-upload.html
        refspec_opts = []

        # By default, new changes are started in WIP mode, and subsequent
        # patchsets don't send email. At any time, passing --send-mail or
        # --send-email will mark the change ready and send email for that
        # particular patch.
        if options.send_mail:
            refspec_opts.append('ready')
            refspec_opts.append('notify=ALL')
        elif (not self.GetIssue() and options.squash and not dogfood_path):
            refspec_opts.append('wip')

        # TODO(tandrii): options.message should be posted as a comment if
        # --send-mail or --send-email is set on non-initial upload as Rietveld
        # used to do it.

        # Set options.title in case user was prompted in _GetTitleForUpload and
        # _CMDUploadChange needs to be called again.
        options.title = self._GetTitleForUpload(
            options, multi_change_upload=multi_change_upload)

        if options.title:
            # Punctuation and whitespace in |title| must be percent-encoded.
            refspec_opts.append(
                'm=' + gerrit_util.PercentEncodeForGitRef(options.title))

        if options.private:
            refspec_opts.append('private')

        if options.topic:
            # Documentation on Gerrit topics is here:
            # https://gerrit-review.googlesource.com/Documentation/user-upload.html#topic
            refspec_opts.append('topic=%s' % options.topic)

        if options.enable_auto_submit:
            refspec_opts.append('l=Auto-Submit+1')
        if options.set_bot_commit:
            refspec_opts.append('l=Bot-Commit+1')
        if options.use_commit_queue:
            refspec_opts.append('l=Commit-Queue+2')
        elif options.cq_dry_run:
            refspec_opts.append('l=Commit-Queue+1')

        if change_desc.get_reviewers(tbr_only=True):
            score = gerrit_util.GetCodeReviewTbrScore(self.GetGerritHost(),
                                                      self.GetGerritProject())
            refspec_opts.append('l=Code-Review+%s' % score)

        # Gerrit sorts hashtags, so order is not important.
        hashtags = {change_desc.sanitize_hash_tag(t) for t in options.hashtags}
        # We check GetIssue because we only add hashtags from the
        # description on the first upload.
        # TODO(b/265929888): When we fully launch the new path:
        # 1) remove fetching hashtags from description alltogether
        # 2) Or use descrtiption hashtags for:
        #    `not (self.GetIssue() and multi_change_upload)`
        # 3) Or enabled change description tags for multi and single changes
        #    by adding them post `git push`.
        if not (self.GetIssue() and dogfood_path):
            hashtags.update(change_desc.get_hash_tags())
        refspec_opts.extend(['hashtag=%s' % t for t in hashtags])

        # Note: Reviewers, and ccs are handled individually for each
        # branch/change.
        return refspec_opts

    def PrepareSquashedCommit(self,
                              options: optparse.Values,
                              parent: str,
                              orig_parent: str,
                              end_commit: Optional[str] = None) -> _NewUpload:
        """Create a squashed commit to upload.

        Args:
            parent: The commit to use as the parent for the new squashed.
            orig_parent: The commit that is an actual ancestor of `end_commit`. It
                is part of the same original tree as end_commit, which does not
                contain squashed commits. This is used to create the change
                description for the new squashed commit with:
                `git log orig_parent..end_commit`.
            end_commit: The commit to use as the end of the new squashed commit.
        """

        if end_commit is None:
            end_commit = RunGit(['rev-parse', self.branchref]).strip()

        reviewers, ccs, change_desc = self._PrepareChange(
            options, orig_parent, end_commit)
        latest_tree = RunGit(['rev-parse', end_commit + ':']).strip()
        with gclient_utils.temporary_file() as desc_tempfile:
            gclient_utils.FileWrite(desc_tempfile, change_desc.description)
            commit_to_push = RunGit(
                ['commit-tree', latest_tree, '-p', parent, '-F',
                 desc_tempfile]).strip()

        # Gerrit may or may not update fast enough to return the correct
        # patchset number after we push. Get the pre-upload patchset and
        # increment later.
        prev_patchset = self.GetMostRecentPatchset(update=False) or 0
        return _NewUpload(reviewers, ccs, commit_to_push, end_commit, parent,
                          change_desc, prev_patchset)

    def PrepareCherryPickSquashedCommit(self, options: optparse.Values,
                                        parent: str) -> _NewUpload:
        """Create a commit cherry-picked on parent to push."""

        # The `parent` is what we will cherry-pick on top of.
        # The `cherry_pick_base` is the beginning range of what
        # we are cherry-picking.
        cherry_pick_base = self.GetCommonAncestorWithUpstream()
        reviewers, ccs, change_desc = self._PrepareChange(
            options, cherry_pick_base, self.branchref)

        new_upload_hash = RunGit(['rev-parse', self.branchref]).strip()
        latest_tree = RunGit(['rev-parse', self.branchref + ':']).strip()
        with gclient_utils.temporary_file() as desc_tempfile:
            gclient_utils.FileWrite(desc_tempfile, change_desc.description)
            commit_to_cp = RunGit([
                'commit-tree', latest_tree, '-p', cherry_pick_base, '-F',
                desc_tempfile
            ]).strip()

        RunGit(['checkout', '-q', parent])
        ret, _out = RunGitWithCode(['cherry-pick', commit_to_cp])
        if ret:
            RunGit(['cherry-pick', '--abort'])
            RunGit(['checkout', '-q', self.branch])
            DieWithError('Could not cleanly cherry-pick')

        commit_to_push = RunGit(['rev-parse', 'HEAD']).strip()
        RunGit(['checkout', '-q', self.branch])

        # Gerrit may or may not update fast enough to return the correct
        # patchset number after we push. Get the pre-upload patchset and
        # increment later.
        prev_patchset = self.GetMostRecentPatchset(update=False) or 0
        return _NewUpload(reviewers, ccs, commit_to_push, new_upload_hash,
                          cherry_pick_base, change_desc, prev_patchset)

    def _PrepareChange(
        self, options: optparse.Values, parent: str, end_commit: str
    ) -> Tuple[Sequence[str], Sequence[str], ChangeDescription]:
        """Prepares the change to be uploaded."""
        self.EnsureCanUploadPatchset(options.force)

        files = self.GetAffectedFiles(parent, end_commit=end_commit)
        change_desc = self._GetDescriptionForUpload(options,
                                                    [parent, end_commit], files)

        watchlist = watchlists.Watchlists(settings.GetRoot())
        self.ExtendCC(watchlist.GetWatchersForPaths(files))
        if not options.bypass_hooks:
            hook_results = self.RunHook(committing=False,
                                        may_prompt=not options.force,
                                        verbose=options.verbose,
                                        parallel=options.parallel,
                                        upstream=parent,
                                        description=change_desc.description,
                                        all_files=False,
                                        end_commit=end_commit)
            self.ExtendCC(hook_results['more_cc'])

        # Update the change description and ensure we have a Change Id.
        if self.GetIssue():
            if options.edit_description:
                change_desc.prompt()
            change_detail = self._GetChangeDetail(['CURRENT_REVISION'])
            change_id = change_detail['change_id']
            change_desc.ensure_change_id(change_id)

        else:  # No change issue. First time uploading
            if not options.force and not (options.message_file
                                          or options.commit_description):
                change_desc.prompt()

            # Check if user added a change_id in the descripiton.
            change_ids = git_footers.get_footer_change_id(
                change_desc.description)
            if len(change_ids) == 1:
                change_id = change_ids[0]
            else:
                change_id = GenerateGerritChangeId(change_desc.description)
                change_desc.ensure_change_id(change_id)

        if options.preserve_tryjobs:
            change_desc.set_preserve_tryjobs()

        SaveDescriptionBackup(change_desc)

        # Add ccs
        ccs = []
        # Add default, watchlist, presubmit ccs if this is the initial upload
        # and CL is not private and auto-ccing has not been disabled.
        if not options.private and not options.no_autocc and not self.GetIssue(
        ):
            ccs = self.GetCCList().split(',')

        # Add ccs from the --cc flag.
        if options.cc:
            ccs.extend(options.cc)

        ccs = [email.strip() for email in ccs if email.strip()]
        if change_desc.get_cced():
            ccs.extend(change_desc.get_cced())

        return change_desc.get_reviewers(), ccs, change_desc

    def PostUploadUpdates(self, options: optparse.Values,
                          new_upload: _NewUpload, change_number: str) -> None:
        """Makes necessary post upload changes to the local and remote cl."""
        if not self.GetIssue():
            self.SetIssue(change_number)

        self.SetPatchset(new_upload.prev_patchset + 1)

        self._GitSetBranchConfigValue(GERRIT_SQUASH_HASH_CONFIG_KEY,
                                      new_upload.commit_to_push)
        self._GitSetBranchConfigValue(LAST_UPLOAD_HASH_CONFIG_KEY,
                                      new_upload.new_last_uploaded_commit)

        if settings.GetRunPostUploadHook():
            self.RunPostUploadHook(options.verbose, new_upload.parent,
                                   new_upload.change_desc.description)

        if new_upload.reviewers or new_upload.ccs:
            gerrit_util.AddReviewers(self.GetGerritHost(),
                                     self._GerritChangeIdentifier(),
                                     reviewers=new_upload.reviewers,
                                     ccs=new_upload.ccs,
                                     notify=bool(options.send_mail))

    def CMDUpload(self, options, git_diff_args, orig_args):
        """Uploads a change to codereview."""
        custom_cl_base = None
        if git_diff_args:
            custom_cl_base = base_branch = git_diff_args[0]
        else:
            if self.GetBranch() is None:
                DieWithError(_NO_BRANCH_ERROR)

            # Default to diffing against common ancestor of upstream branch
            base_branch = self.GetCommonAncestorWithUpstream()
            git_diff_args = [base_branch, 'HEAD']

        # Fast best-effort checks to abort before running potentially expensive
        # hooks if uploading is likely to fail anyway. Passing these checks does
        # not guarantee that uploading will not fail.
        self.EnsureAuthenticated(force=options.force)
        self.EnsureCanUploadPatchset(force=options.force)

        print(f'Processing {_GetCommitCountSummary(*git_diff_args)}...')

        # Apply watchlists on upload.
        watchlist = watchlists.Watchlists(settings.GetRoot())
        files = self.GetAffectedFiles(base_branch)
        if not options.bypass_watchlists:
            self.ExtendCC(watchlist.GetWatchersForPaths(files))

        change_desc = self._GetDescriptionForUpload(options, git_diff_args,
                                                    files)

        # For options.squash, RunHook is called once for each branch in
        # PrepareChange().
        if not options.bypass_hooks and not options.squash:
            hook_results = self.RunHook(committing=False,
                                        may_prompt=not options.force,
                                        verbose=options.verbose,
                                        parallel=options.parallel,
                                        upstream=base_branch,
                                        description=change_desc.description,
                                        all_files=False)
            self.ExtendCC(hook_results['more_cc'])

        print_stats(git_diff_args)
        ret = self.CMDUploadChange(options, git_diff_args, custom_cl_base,
                                   change_desc)
        if not ret:
            if self.GetBranch() is not None:
                self._GitSetBranchConfigValue(
                    LAST_UPLOAD_HASH_CONFIG_KEY,
                    scm.GIT.ResolveCommit(settings.GetRoot(), 'HEAD'))
            # Run post upload hooks, if specified.
            if settings.GetRunPostUploadHook():
                self.RunPostUploadHook(options.verbose, base_branch,
                                       change_desc.description)

            # Upload all dependencies if specified.
            if options.dependencies:
                print()
                print('--dependencies has been specified.')
                print('All dependent local branches will be re-uploaded.')
                print()
                # Remove the dependencies flag from args so that we do not end
                # up in a loop.
                orig_args.remove('--dependencies')
                ret = upload_branch_deps(self, orig_args, options.force)
        return ret

    def SetCQState(self, new_state):
        """Updates the CQ state for the latest patchset.

        Issue must have been already uploaded and known.
        """
        assert new_state in _CQState.ALL_STATES
        assert self.GetIssue()
        try:
            vote_map = {
                _CQState.NONE: 0,
                _CQState.DRY_RUN: 1,
                _CQState.COMMIT: 2,
            }
            labels = {'Commit-Queue': vote_map[new_state]}
            notify = False if new_state == _CQState.DRY_RUN else None
            gerrit_util.SetReview(self.GetGerritHost(),
                                  self._GerritChangeIdentifier(),
                                  labels=labels,
                                  notify=notify)
            return 0
        except KeyboardInterrupt:
            raise
        except:
            print(
                'WARNING: Failed to %s.\n'
                'Either:\n'
                ' * Your project has no CQ,\n'
                ' * You don\'t have permission to change the CQ state,\n'
                ' * There\'s a bug in this code (see stack trace below).\n'
                'Consider specifying which bots to trigger manually or asking your '
                'project owners for permissions or contacting Chrome Infra at:\n'
                'https://www.chromium.org/infra\n\n' %
                ('cancel CQ' if new_state == _CQState.NONE else 'trigger CQ'))
            # Still raise exception so that stack trace is printed.
            raise

    def GetGerritHost(self) -> Optional[str]:
        # Populate self._gerrit_host
        self.GetCodereviewServer()

        if self._gerrit_host and '.' not in self._gerrit_host:
            # Abbreviated domain like "chromium" instead of
            # chromium.googlesource.com.
            parsed = urllib.parse.urlparse(self.GetRemoteUrl())
            if parsed.scheme == 'sso':
                self._gerrit_host = '%s.googlesource.com' % self._gerrit_host
                self._gerrit_server = 'https://%s' % self._gerrit_host

        return self._gerrit_host

    def _GetGitHost(self):
        """Returns git host to be used when uploading change to Gerrit."""
        remote_url = self.GetRemoteUrl()
        if not remote_url:
            return None
        return urllib.parse.urlparse(remote_url).netloc

    def _GetGerritHostFromRemoteUrl(self) -> str:
        url = urllib.parse.urlparse(self.GetRemoteUrl())
        parts = url.netloc.split('.')

        # We assume repo to be hosted on Gerrit, and hence Gerrit server
        # has "-review" suffix for lowest level subdomain.
        parts[0] = parts[0] + '-review'

        if url.scheme == 'sso' and len(parts) == 1:
            # sso:// uses abbreivated hosts, eg. sso://chromium instead
            # of chromium.googlesource.com. Hence, for code review
            # server, they need to be expanded.
            parts[0] += '.googlesource.com'

        return '.'.join(parts)

    def GetCodereviewServer(self) -> str:
        if not self._gerrit_server:
            # If we're on a branch then get the server potentially associated
            # with that branch.
            if self.GetIssue() and self.GetBranch():
                self._gerrit_server = self._GitGetBranchConfigValue(
                    CODEREVIEW_SERVER_CONFIG_KEY)
                if self._gerrit_server:
                    self._gerrit_host = urllib.parse.urlparse(
                        self._gerrit_server).netloc
            if not self._gerrit_server:
                self._gerrit_host = self._GetGerritHostFromRemoteUrl()
                self._gerrit_server = 'https://%s' % self._gerrit_host
        return self._gerrit_server

    def GetGerritProject(self):
        """Returns Gerrit project name based on remote git URL."""
        remote_url = self.GetRemoteUrl()
        if remote_url is None:
            logging.warning('can\'t detect Gerrit project.')
            return None
        project = urllib.parse.urlparse(remote_url).path.strip('/')
        if project.endswith('.git'):
            project = project[:-len('.git')]
        # *.googlesource.com hosts ensure that Git/Gerrit projects don't start
        # with 'a/' prefix, because 'a/' prefix is used to force authentication
        # in gitiles/git-over-https protocol. E.g.,
        # https://chromium.googlesource.com/a/v8/v8 refers to the same
        # repo/project as https://chromium.googlesource.com/v8/v8
        if project.startswith('a/'):
            project = project[len('a/'):]
        return project

    def _GerritChangeIdentifier(self):
        """Handy method for gerrit_util.ChangeIdentifier for a given CL.

        Not to be confused by value of "Change-Id:" footer.
        If Gerrit project can be determined, this will speed up Gerrit HTTP API RPC.
        """
        project = self.GetGerritProject()
        if project:
            return gerrit_util.ChangeIdentifier(project, self.GetIssue())
        # Fall back on still unique, but less efficient change number.
        return str(self.GetIssue())

    def EnsureAuthenticated(self, force: bool) -> None:
        """Best effort check that user is authenticated with Gerrit server."""
        if settings.GetGerritSkipEnsureAuthenticated():
            # For projects with unusual authentication schemes.
            # See http://crbug.com/603378.
            return

        remote_url = self.GetRemoteUrl()
        if remote_url is None:
            logging.warning('invalid remote')
            return

        parsed_url = urllib.parse.urlparse(remote_url)
        if parsed_url.scheme == 'sso':
            # Skip checking authentication for projects with sso:// scheme.
            return
        if parsed_url.scheme != 'https':
            logging.warning(
                'Ignoring branch %(branch)s with non-https remote '
                '%(remote)s', {
                    'branch': self.branch,
                    'remote': self.GetRemoteUrl()
                })
            return

        if newauth.Enabled():
            git_auth.AutoConfigure(os.getcwd(), Changelist())
            return

        # Lazy-loader to identify Gerrit and Git hosts.
        self.GetCodereviewServer()
        git_host = self._GetGitHost()
        assert self._gerrit_server and self._gerrit_host and git_host

        bypassable, msg = gerrit_util.ensure_authenticated(
            git_host, self._gerrit_host)
        if not msg:
            return  # OK
        if bypassable:
            if not force:
                confirm_or_exit(msg, action='continue')
        else:
            DieWithError(msg)

    def EnsureCanUploadPatchset(self, force):
        issue = self.GetIssue()
        if not issue:
            return

        status = self._GetChangeDetail()['status']
        if status == 'ABANDONED':
            DieWithError(
                'Change %s has been abandoned, new uploads are not allowed' %
                (self.GetIssueURL()))
        if status == 'MERGED':
            answer = gclient_utils.AskForData(
                'Change %s has been submitted, new uploads are not allowed. '
                'Would you like to start a new change (Y/n)?' %
                self.GetIssueURL()).lower()
            if answer not in ('y', ''):
                DieWithError('New uploads are not allowed.')
            self.SetIssue()
            return

        # Check to see if the currently authenticated account is the issue
        # owner.

        # first, grab the issue owner email
        owner = self.GetIssueOwner()

        # do a quick check to see if this matches the local git config's
        # configured email.
        git_config_email = scm.GIT.GetConfig(settings.GetRoot(), 'user.email')
        if git_config_email == owner:
            # Good enough - Gerrit will reject this if the user is doing funny things
            # with user.email.
            return

        # However, the user may have linked accounts in Gerrit, so pull up the
        # list of all known emails for the currently authenticated account.
        emails = gerrit_util.GetAccountEmails(self.GetGerritHost(), 'self')
        if not emails:
            print('WARNING: Gerrit does not have a record for your account.')
            print('Please browse to https://{self.GetGerritHost()} and log in.')
            return

        # If the issue owner is one of the emails for the currently
        # authenticated account, Gerrit will accept the upload.
        if any(owner == e['email'] for e in emails):
            return

        if not force:
            print(
                f'WARNING: Change {issue} is owned by {owner}, but Gerrit knows you as:'
            )
            for email in emails:
                tag = ' (preferred)' if email.get('preferred') else ''
                print(f'  * {email["email"]}{tag}')
            print('Uploading may fail due to lack of permissions.')
            confirm_or_exit(action='upload')

    def GetStatus(self):
        """Applies a rough heuristic to give a simple summary of an issue's review
        or CQ status, assuming adherence to a common workflow.

        Returns None if no issue for this branch, or one of the following keywords:
            * 'error'   - error from review tool (including deleted issues)
            * 'unsent'  - no reviewers added
            * 'waiting' - waiting for review
            * 'reply'   - waiting for uploader to reply to review
            * 'lgtm'    - Code-Review label has been set
            * 'dry-run' - dry-running in the CQ
            * 'commit'  - in the CQ
            * 'closed'  - successfully submitted or abandoned
        """
        if not self.GetIssue():
            return None

        try:
            data = self._GetChangeDetail(
                ['DETAILED_LABELS', 'CURRENT_REVISION', 'SUBMITTABLE'])
        except GerritChangeNotExists:
            return 'error'

        if data['status'] in ('ABANDONED', 'MERGED'):
            return 'closed'

        cq_label = data['labels'].get('Commit-Queue', {})
        max_cq_vote = 0
        for vote in cq_label.get('all', []):
            max_cq_vote = max(max_cq_vote, vote.get('value', 0))
        if max_cq_vote == 2:
            return 'commit'
        if max_cq_vote == 1:
            return 'dry-run'

        if data['labels'].get('Code-Review', {}).get('approved'):
            return 'lgtm'

        if not data.get('reviewers', {}).get('REVIEWER', []):
            return 'unsent'

        owner = data['owner'].get('_account_id')
        messages = sorted(data.get('messages', []), key=lambda m: m.get('date'))
        while messages:
            m = messages.pop()
            if (m.get('tag', '').startswith('autogenerated:cq')
                    or m.get('tag', '').startswith('autogenerated:cv')):
                # Ignore replies from LUCI CV/CQ.
                continue
            if m.get('author', {}).get('_account_id') == owner:
                # Most recent message was by owner.
                return 'waiting'

            # Some reply from non-owner.
            return 'reply'

        # Somehow there are no messages even though there are reviewers.
        return 'unsent'

    def GetMostRecentPatchset(self, update=True):
        if not self.GetIssue():
            return None

        data = self._GetChangeDetail(['CURRENT_REVISION'])
        patchset = data['revisions'][data['current_revision']]['_number']
        if update:
            self.SetPatchset(patchset)
        return patchset

    def _IsPatchsetRangeSignificant(self, lower, upper):
        """Returns True if the inclusive range of patchsets contains any reworks or
        rebases."""
        if not self.GetIssue():
            return False

        data = self._GetChangeDetail(['ALL_REVISIONS'])
        ps_kind = {}
        for rev_info in data.get('revisions', {}).values():
            ps_kind[rev_info['_number']] = rev_info.get('kind', '')

        for ps in range(lower, upper + 1):
            assert ps in ps_kind, 'expected patchset %d in change detail' % ps
            if ps_kind[ps] not in ('NO_CHANGE', 'NO_CODE_CHANGE'):
                return True
        return False

    def GetMostRecentDryRunPatchset(self):
        """Get patchsets equivalent to the most recent patchset and return
        the patchset with the latest dry run. If none have been dry run, return
        the latest patchset."""
        if not self.GetIssue():
            return None

        data = self._GetChangeDetail(['ALL_REVISIONS'])
        patchset = data['revisions'][data['current_revision']]['_number']
        dry_run = {
            int(m['_revision_number'])
            for m in data.get('messages', [])
            if m.get('tag', '').endswith('dry-run')
        }

        for revision_info in sorted(data.get('revisions', {}).values(),
                                    key=lambda c: c['_number'],
                                    reverse=True):
            if revision_info['_number'] in dry_run:
                patchset = revision_info['_number']
                break
            if revision_info.get('kind', '') not in \
                ('NO_CHANGE', 'NO_CODE_CHANGE', 'TRIVIAL_REBASE'):
                break
        self.SetPatchset(patchset)
        return patchset

    def AddComment(self, message, publish=None):
        gerrit_util.SetReview(self.GetGerritHost(),
                              self._GerritChangeIdentifier(),
                              msg=message,
                              ready=publish)

    def GetCommentsSummary(self, readable=True):
        # DETAILED_ACCOUNTS is to get emails in accounts.
        # CURRENT_REVISION is included to get the latest patchset so that
        # only the robot comments from the latest patchset can be shown.
        messages = self._GetChangeDetail(
            options=['MESSAGES', 'DETAILED_ACCOUNTS', 'CURRENT_REVISION']).get(
                'messages', [])
        file_comments = gerrit_util.GetChangeComments(
            self.GetGerritHost(), self._GerritChangeIdentifier())
        robot_file_comments = gerrit_util.GetChangeRobotComments(
            self.GetGerritHost(), self._GerritChangeIdentifier())

        # Add the robot comments onto the list of comments, but only
        # keep those that are from the latest patchset.
        latest_patch_set = self.GetMostRecentPatchset()
        for path, robot_comments in robot_file_comments.items():
            line_comments = file_comments.setdefault(path, [])
            line_comments.extend([
                c for c in robot_comments if c['patch_set'] == latest_patch_set
            ])

        # Build dictionary of file comments for easy access and sorting later.
        # {author+date: {path: {patchset: {line: url+message}}}}
        comments = collections.defaultdict(lambda: collections.defaultdict(
            lambda: collections.defaultdict(dict)))

        server = self.GetCodereviewServer()
        if server in _KNOWN_GERRIT_TO_SHORT_URLS:
            # /c/ is automatically added by short URL server.
            url_prefix = '%s/%s' % (_KNOWN_GERRIT_TO_SHORT_URLS[server],
                                    self.GetIssue())
        else:
            url_prefix = '%s/c/%s' % (server, self.GetIssue())

        for path, line_comments in file_comments.items():
            for comment in line_comments:
                tag = comment.get('tag', '')
                if tag.startswith(
                        'autogenerated') and 'robot_id' not in comment:
                    continue
                key = (comment['author']['email'], comment['updated'])
                if comment.get('side', 'REVISION') == 'PARENT':
                    patchset = 'Base'
                else:
                    patchset = 'PS%d' % comment['patch_set']
                line = comment.get('line', 0)
                url = ('%s/%s/%s#%s%s' %
                       (url_prefix, comment['patch_set'],
                        path, 'b' if comment.get('side') == 'PARENT' else '',
                        str(line) if line else ''))
                comments[key][path][patchset][line] = (url, comment['message'])

        summaries = []
        for msg in messages:
            summary = self._BuildCommentSummary(msg, comments, readable)
            if summary:
                summaries.append(summary)
        return summaries

    @staticmethod
    def _BuildCommentSummary(msg, comments, readable):
        if 'email' not in msg['author']:
            # Some bot accounts may not have an email associated.
            return None

        key = (msg['author']['email'], msg['date'])
        # Don't bother showing autogenerated messages that don't have associated
        # file or line comments. this will filter out most autogenerated
        # messages, but will keep robot comments like those from Tricium.
        is_autogenerated = msg.get('tag', '').startswith('autogenerated')
        if is_autogenerated and not comments.get(key):
            return None
        message = msg['message']
        # Gerrit spits out nanoseconds.
        assert len(msg['date'].split('.')[-1]) == 9
        date = datetime.datetime.strptime(msg['date'][:-3],
                                          '%Y-%m-%d %H:%M:%S.%f')
        if key in comments:
            message += '\n'
        for path, patchsets in sorted(comments.get(key, {}).items()):
            if readable:
                message += '\n%s' % path
            for patchset, lines in sorted(patchsets.items()):
                for line, (url, content) in sorted(lines.items()):
                    if line:
                        line_str = 'Line %d' % line
                        path_str = '%s:%d:' % (path, line)
                    else:
                        line_str = 'File comment'
                        path_str = '%s:0:' % path
                    if readable:
                        message += '\n  %s, %s: %s' % (patchset, line_str, url)
                        message += '\n  %s\n' % content
                    else:
                        message += '\n%s ' % path_str
                        message += '\n%s\n' % content

        return _CommentSummary(
            date=date,
            message=message,
            sender=msg['author']['email'],
            autogenerated=is_autogenerated,
            # These could be inferred from the text messages and correlated with
            # Code-Review label maximum, however this is not reliable.
            # Leaving as is until the need arises.
            approval=False,
            disapproval=False,
        )

    def CloseIssue(self):
        gerrit_util.AbandonChange(self.GetGerritHost(),
                                  self._GerritChangeIdentifier(),
                                  msg='')

    def SubmitIssue(self):
        gerrit_util.SubmitChange(self.GetGerritHost(),
                                 self._GerritChangeIdentifier())

    def _GetChangeDetail(self, options=None):
        """Returns details of associated Gerrit change and caching results."""
        options = options or []
        assert self.GetIssue(), 'issue is required to query Gerrit'

        # Optimization to avoid multiple RPCs:
        if 'CURRENT_REVISION' in options or 'ALL_REVISIONS' in options:
            options.append('CURRENT_COMMIT')

        # Normalize issue and options for consistent keys in cache.
        cache_key = str(self.GetIssue())
        options_set = frozenset(o.upper() for o in options)

        for cached_options_set, data in self._detail_cache.get(cache_key, []):
            # Assumption: data fetched before with extra options is suitable
            # for return for a smaller set of options.
            # For example, if we cached data for
            #     options=[CURRENT_REVISION, DETAILED_FOOTERS]
            #   and request is for options=[CURRENT_REVISION],
            # THEN we can return prior cached data.
            if options_set.issubset(cached_options_set):
                return data

        try:
            data = gerrit_util.GetChangeDetail(self.GetGerritHost(),
                                               self._GerritChangeIdentifier(),
                                               options_set)
        except gerrit_util.GerritError as e:
            if e.http_status == 404:
                raise GerritChangeNotExists(self.GetIssue(),
                                            self.GetCodereviewServer())
            raise

        self._detail_cache.setdefault(cache_key, []).append((options_set, data))
        return data

    def _GetChangeCommit(self, revision: str = 'current') -> dict:
        assert self.GetIssue(), 'issue must be set to query Gerrit'
        try:
            data: dict = gerrit_util.GetChangeCommit(
                self.GetGerritHost(), self._GerritChangeIdentifier(), revision)
        except gerrit_util.GerritError as e:
            if e.http_status == 404:
                raise GerritChangeNotExists(self.GetIssue(),
                                            self.GetCodereviewServer())
            raise
        return data

    def _IsCqConfigured(self):
        detail = self._GetChangeDetail(['LABELS'])
        return u'Commit-Queue' in detail.get('labels', {})

    def CMDLand(self, force, bypass_hooks, verbose, parallel, resultdb, realm):
        if git_common.is_dirty_git_tree('land'):
            return 1

        detail = self._GetChangeDetail(['CURRENT_REVISION', 'LABELS'])
        if not force and self._IsCqConfigured():
            confirm_or_exit(
                '\nIt seems this repository has a CQ, '
                'which can test and land changes for you. '
                'Are you sure you wish to bypass it?\n',
                action='bypass CQ')
        differs = True
        last_upload = self._GitGetBranchConfigValue(
            GERRIT_SQUASH_HASH_CONFIG_KEY)
        # Note: git diff outputs nothing if there is no diff.
        if not last_upload or RunGit(['diff', last_upload]).strip():
            print(
                'WARNING: Some changes from local branch haven\'t been uploaded.'
            )
        else:
            if detail['current_revision'] == last_upload:
                differs = False
            else:
                print(
                    'WARNING: Local branch contents differ from latest uploaded '
                    'patchset.')
        if differs:
            if not force:
                confirm_or_exit(
                    'Do you want to submit latest Gerrit patchset and bypass hooks?\n',
                    action='submit')
            print(
                'WARNING: Bypassing hooks and submitting latest uploaded patchset.'
            )
        elif not bypass_hooks:
            upstream = self.GetCommonAncestorWithUpstream()
            if self.GetIssue():
                description = self.FetchDescription()
            else:
                description = _create_description_from_log([upstream])
            self.RunHook(committing=True,
                         may_prompt=not force,
                         verbose=verbose,
                         parallel=parallel,
                         upstream=upstream,
                         description=description,
                         all_files=False,
                         resultdb=resultdb,
                         realm=realm)

        self.SubmitIssue()
        print('Issue %s has been submitted.' % self.GetIssueURL())
        links = self._GetChangeCommit().get('web_links', [])
        for link in links:
            if link.get('name') in ['gitiles', 'browse'] and link.get('url'):
                print('Landed as: %s' % link.get('url'))
                break
        return 0

    def CMDPatchWithParsedIssue(self, parsed_issue_arg, nocommit, force):
        assert parsed_issue_arg.valid

        self.issue = parsed_issue_arg.issue

        if parsed_issue_arg.hostname:
            self._gerrit_host = parsed_issue_arg.hostname
            self._gerrit_server = 'https://%s' % self._gerrit_host

        try:
            detail = self._GetChangeDetail(['ALL_REVISIONS'])
        except GerritChangeNotExists as e:
            DieWithError(str(e))

        if not parsed_issue_arg.patchset:
            # Use current revision by default.
            revision_info = detail['revisions'][detail['current_revision']]
            patchset = int(revision_info['_number'])
        else:
            patchset = parsed_issue_arg.patchset
            for revision_info in detail['revisions'].values():
                if int(revision_info['_number']) == parsed_issue_arg.patchset:
                    break
            else:
                DieWithError('Couldn\'t find patchset %i in change %i' %
                             (parsed_issue_arg.patchset, self.GetIssue()))

        remote_url = self.GetRemoteUrl()
        if remote_url.endswith('.git'):
            remote_url = remote_url[:-len('.git')]
        remote_url = remote_url.rstrip('/')

        fetch_info = revision_info['fetch']['http']
        fetch_info['url'] = fetch_info['url'].rstrip('/')

        if remote_url != fetch_info['url']:
            DieWithError(
                'Trying to patch a change from %s but this repo appears '
                'to be %s.' % (fetch_info['url'], remote_url))

        RunGit(['fetch', fetch_info['url'], fetch_info['ref']])

        # Set issue immediately in case the cherry-pick fails, which happens
        # when resolving conflicts.
        if self.GetBranch():
            self.SetIssue(parsed_issue_arg.issue)

        if force:
            RunGit(['reset', '--hard', 'FETCH_HEAD'])
            print('Checked out commit for change %i patchset %i locally' %
                  (parsed_issue_arg.issue, patchset))
        elif nocommit:
            RunGit(['cherry-pick', '--no-commit', 'FETCH_HEAD'])
            print('Patch applied to index.')
        else:
            RunGit(['cherry-pick', 'FETCH_HEAD'])
            print('Committed patch for change %i patchset %i locally.' %
                  (parsed_issue_arg.issue, patchset))
            print(
                'Note: this created a local commit on top of parent commit '
                'that is different from the one in Gerrit. If the patched CL '
                'is not yours and you cannot upload new patches to it, you '
                'will not be able to upload stacked changes created on top of '
                'this branch.\n'
                'If you want to do that, use "git cl patch --force" instead.')

        if self.GetBranch():
            self.SetPatchset(patchset)
            fetched_hash = scm.GIT.ResolveCommit(settings.GetRoot(),
                                                 'FETCH_HEAD')
            self._GitSetBranchConfigValue(LAST_UPLOAD_HASH_CONFIG_KEY,
                                          fetched_hash)
            self._GitSetBranchConfigValue(GERRIT_SQUASH_HASH_CONFIG_KEY,
                                          fetched_hash)
        else:
            print(
                'WARNING: You are in detached HEAD state.\n'
                'The patch has been applied to your checkout, but you will not be '
                'able to upload a new patch set to the gerrit issue.\n'
                'Try using the \'-b\' option if you would like to work on a '
                'branch and/or upload a new patch set.')

        return 0

    @staticmethod
    def _GerritCommitMsgHookCheck(offer_removal):
        # type: (bool) -> None
        """Checks for the gerrit's commit-msg hook and removes it if necessary."""
        hook = os.path.join(settings.GetRoot(), '.git', 'hooks', 'commit-msg')
        if not os.path.exists(hook):
            return
        # Crude attempt to distinguish Gerrit Codereview hook from a potentially
        # custom developer-made one.
        data = gclient_utils.FileRead(hook)
        if not ('From Gerrit Code Review' in data and 'add_ChangeId()' in data):
            return
        print('WARNING: You have Gerrit commit-msg hook installed.\n'
              'It is not necessary for uploading with git cl in squash mode, '
              'and may interfere with it in subtle ways.\n'
              'We recommend you remove the commit-msg hook.')
        if offer_removal:
            if ask_for_explicit_yes('Do you want to remove it now?'):
                gclient_utils.rm_file_or_tree(hook)
                print('Gerrit commit-msg hook removed.')
            else:
                print('OK, will keep Gerrit commit-msg hook in place.')

    def _CleanUpOldTraces(self):
        """Keep only the last |MAX_TRACES| traces."""
        try:
            traces = sorted([
                os.path.join(TRACES_DIR, f) for f in os.listdir(TRACES_DIR)
                if (os.path.isfile(os.path.join(TRACES_DIR, f))
                    and not f.startswith('tmp'))
            ])
            traces_to_delete = traces[:-MAX_TRACES]
            for trace in traces_to_delete:
                os.remove(trace)
        except OSError:
            print('WARNING: Failed to remove old git traces from\n'
                  '  %s'
                  'Consider removing them manually.' % TRACES_DIR)

    def _WriteGitPushTraces(self, trace_name, traces_dir, git_push_metadata):
        """Zip and write the git push traces stored in traces_dir."""
        gclient_utils.safe_makedirs(TRACES_DIR)
        traces_zip = trace_name + '-traces'
        traces_readme = trace_name + '-README'
        # Create a temporary dir to store git config and gitcookies in. It will
        # be compressed and stored next to the traces.
        git_info_dir = tempfile.mkdtemp()
        git_info_zip = trace_name + '-git-info'

        git_push_metadata['now'] = datetime_now().strftime(
            '%Y-%m-%dT%H:%M:%S.%f')

        git_push_metadata['trace_name'] = trace_name
        gclient_utils.FileWrite(traces_readme,
                                TRACES_README_FORMAT % git_push_metadata)

        # Keep only the first 6 characters of the git hashes on the packet
        # trace. This greatly decreases size after compression.
        packet_traces = os.path.join(traces_dir, 'trace-packet')
        if os.path.isfile(packet_traces):
            contents = gclient_utils.FileRead(packet_traces)
            gclient_utils.FileWrite(packet_traces,
                                    GIT_HASH_RE.sub(r'\1', contents))
        shutil.make_archive(traces_zip, 'zip', traces_dir)

        # Collect and compress the git config and gitcookies.
        git_config = '\n'.join(
            f'{k}={v}' for k, v in scm.GIT.YieldConfigRegexp(settings.GetRoot()))
        gclient_utils.FileWrite(os.path.join(git_info_dir, 'git-config'),
                                git_config)

        auth_name, debug_state = gerrit_util.debug_auth()
        # Writes a file like CookiesAuthenticator.debug_summary_state
        gclient_utils.FileWrite(
            os.path.join(git_info_dir, f'{auth_name}.debug_summary_state'),
            debug_state)
        shutil.make_archive(git_info_zip, 'zip', git_info_dir)

        gclient_utils.rmtree(git_info_dir)

    def _RunGitPushWithTraces(self,
                              refspec,
                              refspec_opts,
                              git_push_metadata,
                              git_push_options=None):
        """Run git push and collect the traces resulting from the execution."""
        # Create a temporary directory to store traces in. Traces will be
        # compressed and stored in a 'traces' dir inside depot_tools.
        traces_dir = tempfile.mkdtemp()
        trace_name = os.path.join(TRACES_DIR,
                                  datetime_now().strftime('%Y%m%dT%H%M%S.%f'))

        env = os.environ.copy()
        env['GIT_REDACT_COOKIES'] = 'o,SSO,GSSO_Uberproxy'
        env['GIT_TR2_EVENT'] = os.path.join(traces_dir, 'tr2-event')
        env['GIT_TRACE2_EVENT'] = os.path.join(traces_dir, 'tr2-event')
        env['GIT_TRACE_CURL'] = os.path.join(traces_dir, 'trace-curl')
        env['GIT_TRACE_CURL_NO_DATA'] = '1'
        env['GIT_TRACE_PACKET'] = os.path.join(traces_dir, 'trace-packet')

        push_returncode = 0
        before_push = time_time()
        try:
            remote_url = self.GetRemoteUrl()
            push_cmd = ['git', 'push', remote_url, refspec]
            if git_push_options:
                for opt in git_push_options:
                    push_cmd.extend(['-o', opt])

            push_stdout = gclient_utils.CheckCallAndFilter(
                push_cmd,
                env=env,
                print_stdout=True,
                # Flush after every line: useful for seeing progress when
                # running as recipe.
                filter_fn=lambda _: sys.stdout.flush())
            push_stdout = push_stdout.decode('utf-8', 'replace')
        except subprocess2.CalledProcessError as e:
            push_returncode = e.returncode
            if 'blocked keyword' in str(e.stdout) or 'banned word' in str(
                    e.stdout):
                raise GitPushError(
                    'Failed to create a change, very likely due to blocked keyword. '
                    'Please examine output above for the reason of the failure.\n'
                    'If this is a false positive, you can try to bypass blocked '
                    'keyword by using push option '
                    '-o banned-words~skip, e.g.:\n'
                    'git cl upload -o banned-words~skip\n\n'
                    'If git-cl is not working correctly, file a bug under the '
                    'Infra>SDK component.')
            if 'git push -o nokeycheck' in str(e.stdout):
                raise GitPushError(
                    'Failed to create a change, very likely due to a private key being '
                    'detected. Please examine output above for the reason of the '
                    'failure.\n'
                    'If this is a false positive, you can try to bypass private key '
                    'detection by using push option '
                    '-o nokeycheck, e.g.:\n'
                    'git cl upload -o nokeycheck\n\n'
                    'If git-cl is not working correctly, file a bug under the '
                    'Infra>SDK component.')

            raise GitPushError(
                'Failed to create a change. Please examine output above for the '
                'reason of the failure.\n'
                'For emergencies, Googlers can escalate to '
                'go/gob-support or go/notify#gob\n'
                'Hint: run command below to diagnose common Git/Gerrit '
                'credential problems:\n'
                '  git cl creds-check\n'
                '\n'
                'If git-cl is not working correctly, file a bug under the Infra>SDK '
                'component including the files below.\n'
                'Review the files before upload, since they might contain sensitive '
                'information.\n'
                'Set the Restrict-View-Google label so that they are not publicly '
                'accessible.\n' + TRACES_MESSAGE % {'trace_name': trace_name})
        finally:
            execution_time = time_time() - before_push
            metrics.collector.add_repeated(
                'sub_commands', {
                    'command':
                    'git push',
                    'execution_time':
                    execution_time,
                    'exit_code':
                    push_returncode,
                    'arguments':
                    metrics_utils.extract_known_subcommand_args(refspec_opts),
                })

            git_push_metadata['execution_time'] = execution_time
            git_push_metadata['exit_code'] = push_returncode
            self._WriteGitPushTraces(trace_name, traces_dir, git_push_metadata)

            self._CleanUpOldTraces()
            gclient_utils.rmtree(traces_dir)

        return push_stdout

    def CMDUploadChange(self, options, git_diff_args, custom_cl_base,
                        change_desc):
        """Upload the current branch to Gerrit, retry if new remote HEAD is
        found. options and change_desc may be mutated."""
        remote, remote_branch = self.GetRemoteBranch()
        branch = GetTargetRef(remote, remote_branch, options.target_branch)

        try:
            return self._CMDUploadChange(options, git_diff_args, custom_cl_base,
                                         change_desc, branch)
        except GitPushError as e:
            # Repository might be in the middle of transition to main branch as
            # default, and uploads to old default might be blocked.
            if remote_branch not in [DEFAULT_OLD_BRANCH, DEFAULT_NEW_BRANCH]:
                DieWithError(str(e), change_desc)

            project_head = gerrit_util.GetProjectHead(self._gerrit_host,
                                                      self.GetGerritProject())
            if project_head == branch:
                DieWithError(str(e), change_desc)
            branch = project_head

        print("WARNING: Fetching remote state and retrying upload to default "
              "branch...")
        RunGit(['fetch', '--prune', remote])
        options.edit_description = False
        options.force = True
        try:
            self._CMDUploadChange(options, git_diff_args, custom_cl_base,
                                  change_desc, branch)
        except GitPushError as e:
            DieWithError(str(e), change_desc)

    def _CMDUploadChange(self, options, git_diff_args, custom_cl_base,
                         change_desc, branch):
        """Upload the current branch to Gerrit."""
        if options.squash:
            Changelist._GerritCommitMsgHookCheck(
                offer_removal=not options.force)
            external_parent = None
            if self.GetIssue():
                # User requested to change description
                if options.edit_description:
                    change_desc.prompt()
                change_detail = self._GetChangeDetail(['CURRENT_REVISION'])
                change_id = change_detail['change_id']
                change_desc.ensure_change_id(change_id)

                # Check if changes outside of this workspace have been uploaded.
                current_rev = change_detail['current_revision']
                last_uploaded_rev = self._GitGetBranchConfigValue(
                    GERRIT_SQUASH_HASH_CONFIG_KEY)
                if last_uploaded_rev and current_rev != last_uploaded_rev:
                    external_parent = self._UpdateWithExternalChanges()
            else:  # if not self.GetIssue()
                if not options.force and not (options.message_file
                                              or options.commit_description):
                    change_desc.prompt()
                change_ids = git_footers.get_footer_change_id(
                    change_desc.description)
                if len(change_ids) == 1:
                    change_id = change_ids[0]
                else:
                    change_id = GenerateGerritChangeId(change_desc.description)
                    change_desc.ensure_change_id(change_id)

            if options.preserve_tryjobs:
                change_desc.set_preserve_tryjobs()

            remote, upstream_branch = self.FetchUpstreamTuple(self.GetBranch())
            parent = external_parent or self._ComputeParent(
                remote, upstream_branch, custom_cl_base, options.force,
                change_desc)
            tree = RunGit(['rev-parse', 'HEAD:']).strip()
            with gclient_utils.temporary_file() as desc_tempfile:
                gclient_utils.FileWrite(desc_tempfile, change_desc.description)
                ref_to_push = RunGit(
                    ['commit-tree', tree, '-p', parent, '-F',
                     desc_tempfile]).strip()
        else:  # if not options.squash
            if options.no_add_changeid:
                pass
            else:  # adding Change-Ids is okay.
                if not git_footers.get_footer_change_id(
                        change_desc.description):
                    DownloadGerritHook(False)
                    change_desc.set_description(
                        self._AddChangeIdToCommitMessage(
                            change_desc.description, git_diff_args))
            ref_to_push = 'HEAD'
            # For no-squash mode, we assume the remote called "origin" is the
            # one we want. It is not worthwhile to support different workflows
            # for no-squash mode.
            parent = 'origin/%s' % branch
            # attempt to extract the changeid from the current description
            # fail informatively if not possible.
            change_id_candidates = git_footers.get_footer_change_id(
                change_desc.description)
            if not change_id_candidates:
                DieWithError("Unable to extract change-id from message.")
            change_id = change_id_candidates[0]

        SaveDescriptionBackup(change_desc)
        commits = RunGitSilent(['rev-list',
                                '%s..%s' % (parent, ref_to_push)]).splitlines()
        if len(commits) > 1:
            print(
                'WARNING: This will upload %d commits. Run the following command '
                'to see which commits will be uploaded: ' % len(commits))
            print('git log %s..%s' % (parent, ref_to_push))
            print('You can also use `git squash-branch` to squash these into a '
                  'single commit.')
            confirm_or_exit(action='upload')

        reviewers = sorted(change_desc.get_reviewers())
        cc = []
        # Add default, watchlist, presubmit ccs if this is the initial upload
        # and CL is not private and auto-ccing has not been disabled.
        if not options.private and not options.no_autocc and not self.GetIssue(
        ):
            cc = self.GetCCList().split(',')
        # Add cc's from the --cc flag.
        if options.cc:
            cc.extend(options.cc)
        cc = [email.strip() for email in cc if email.strip()]
        if change_desc.get_cced():
            cc.extend(change_desc.get_cced())
        if self.GetGerritHost() == 'chromium-review.googlesource.com':
            valid_accounts = set(reviewers + cc)
            # TODO(crbug/877717): relax this for all hosts.
        else:
            valid_accounts = gerrit_util.ValidAccounts(self.GetGerritHost(),
                                                       reviewers + cc)
        logging.info('accounts %s are recognized, %s invalid',
                     sorted(valid_accounts),
                     set(reviewers + cc).difference(set(valid_accounts)))

        # Extra options that can be specified at push time. Doc:
        # https://gerrit-review.googlesource.com/Documentation/user-upload.html
        refspec_opts = self._GetRefSpecOptions(options, change_desc)

        for r in sorted(reviewers):
            if r in valid_accounts:
                refspec_opts.append('r=%s' % r)
                reviewers.remove(r)
            else:
                # TODO(tandrii): this should probably be a hard failure.
                print(
                    'WARNING: reviewer %s doesn\'t have a Gerrit account, skipping'
                    % r)
        for c in sorted(cc):
            # refspec option will be rejected if cc doesn't correspond to an
            # account, even though REST call to add such arbitrary cc may
            # succeed.
            if c in valid_accounts:
                refspec_opts.append('cc=%s' % c)
                cc.remove(c)

        refspec_suffix = ''
        if refspec_opts:
            refspec_suffix = '%' + ','.join(refspec_opts)
            assert ' ' not in refspec_suffix, (
                'spaces not allowed in refspec: "%s"' % refspec_suffix)
        refspec = '%s:refs/for/%s%s' % (ref_to_push, branch, refspec_suffix)

        git_push_metadata = {
            'gerrit_host': self.GetGerritHost(),
            'title': options.title or '<untitled>',
            'change_id': change_id,
            'description': change_desc.description,
        }

        # Gerrit may or may not update fast enough to return the correct
        # patchset number after we push. Get the pre-upload patchset and
        # increment later.
        latest_ps = self.GetMostRecentPatchset(update=False) or 0

        push_stdout = self._RunGitPushWithTraces(refspec, refspec_opts,
                                                 git_push_metadata,
                                                 options.push_options)

        if options.squash:
            regex = re.compile(r'remote:\s+https?://[\w\-\.\+\/#]*/(\d+)\s.*')
            change_numbers = [
                m.group(1) for m in map(regex.match, push_stdout.splitlines())
                if m
            ]
            if len(change_numbers) != 1:
                DieWithError((
                    'Created|Updated %d issues on Gerrit, but only 1 expected.\n'
                    'Change-Id: %s') % (len(change_numbers), change_id),
                             change_desc)
            self.SetIssue(change_numbers[0])
            self.SetPatchset(latest_ps + 1)
            self._GitSetBranchConfigValue(GERRIT_SQUASH_HASH_CONFIG_KEY,
                                          ref_to_push)

        if self.GetIssue() and (reviewers or cc):
            # GetIssue() is not set in case of non-squash uploads according to
            # tests. TODO(crbug.com/751901): non-squash uploads in git cl should
            # be removed.
            gerrit_util.AddReviewers(self.GetGerritHost(),
                                     self._GerritChangeIdentifier(),
                                     reviewers,
                                     cc,
                                     notify=bool(options.send_mail))

        return 0

    def _ComputeParent(self, remote, upstream_branch, custom_cl_base, force,
                       change_desc):
        """Computes parent of the generated commit to be uploaded to Gerrit.

        Returns revision or a ref name.
        """
        if custom_cl_base:
            # Try to avoid creating additional unintended CLs when uploading,
            # unless user wants to take this risk.
            local_ref_of_target_remote = self.GetRemoteBranch()[1]
            code, _ = RunGitWithCode([
                'merge-base', '--is-ancestor', custom_cl_base,
                local_ref_of_target_remote
            ])
            if code == 1:
                print(
                    '\nWARNING: Manually specified base of this CL `%s` '
                    'doesn\'t seem to belong to target remote branch `%s`.\n\n'
                    'If you proceed with upload, more than 1 CL may be created by '
                    'Gerrit as a result, in turn confusing or crashing git cl.\n\n'
                    'If you are certain that specified base `%s` has already been '
                    'uploaded to Gerrit as another CL, you may proceed.\n' %
                    (custom_cl_base, local_ref_of_target_remote,
                     custom_cl_base))
                if not force:
                    confirm_or_exit(
                        'Do you take responsibility for cleaning up potential mess '
                        'resulting from proceeding with upload?',
                        action='upload')
            return custom_cl_base

        if remote != '.':
            return self.GetCommonAncestorWithUpstream()

        # If our upstream branch is local, we base our squashed commit on its
        # squashed version.
        upstream_branch_name = scm.GIT.ShortBranchName(upstream_branch)

        if upstream_branch_name == 'master':
            return self.GetCommonAncestorWithUpstream()
        if upstream_branch_name == 'main':
            return self.GetCommonAncestorWithUpstream()

        # Check the squashed hash of the parent.
        # TODO(tandrii): consider checking parent change in Gerrit and using its
        # hash if tree hash of latest parent revision (patchset) in Gerrit
        # matches the tree hash of the parent branch. The upside is less likely
        # bogus requests to reupload parent change just because it's uploadhash
        # is missing, yet the downside likely exists, too (albeit unknown to me
        # yet).
        parent = scm.GIT.GetBranchConfig(settings.GetRoot(),
                                         upstream_branch_name,
                                         GERRIT_SQUASH_HASH_CONFIG_KEY)
        # Verify that the upstream branch has been uploaded too, otherwise
        # Gerrit will create additional CLs when uploading.
        if not parent or (RunGitSilent(['rev-parse', upstream_branch + ':']) !=
                          RunGitSilent(['rev-parse', parent + ':'])):
            DieWithError(
                '\nUpload upstream branch %s first.\n'
                'It is likely that this branch has been rebased since its last '
                'upload, so you just need to upload it again.\n'
                '(If you uploaded it with --no-squash, then branch dependencies '
                'are not supported, and you should reupload with --squash.)' %
                upstream_branch_name, change_desc)
        return parent

    def _UpdateWithExternalChanges(self) -> Optional[str]:
        """Updates workspace with external changes.

        Returns the commit hash that should be used as the merge base on upload.
        """
        local_ps = self.GetPatchset()
        if local_ps is None:
            return

        external_ps = self.GetMostRecentPatchset(update=False)
        if external_ps is None or local_ps == external_ps or \
            not self._IsPatchsetRangeSignificant(local_ps + 1, external_ps):
            return

        num_changes = external_ps - local_ps
        if num_changes > 1:
            change_words = 'changes were'
        else:
            change_words = 'change was'
        print('\n%d external %s published to %s:\n' %
              (num_changes, change_words, self.GetIssueURL(short=True)))

        # Print an overview of external changes.
        ps_to_commit = {}
        ps_to_info = {}
        revisions = self._GetChangeDetail(['ALL_REVISIONS'])
        for commit_id, revision_info in revisions.get('revisions', {}).items():
            ps_num = revision_info['_number']
            ps_to_commit[ps_num] = commit_id
            ps_to_info[ps_num] = revision_info

        for ps in range(external_ps, local_ps, -1):
            commit = ps_to_commit[ps][:8]
            desc = ps_to_info[ps].get('description', '')
            print('Patchset %d [%s] %s' % (ps, commit, desc))

        print('\nSee diff at: %s/%d..%d' %
              (self.GetIssueURL(short=True), local_ps, external_ps))
        print('\nUploading without applying patches will override them.')

        if not ask_for_explicit_yes('Get the latest changes and apply on top?'):
            return

        # Get latest Gerrit merge base. Use the first parent even if multiple
        # exist.
        external_parent: dict = self._GetChangeCommit(
            revision=external_ps)['parents'][0]
        external_base: str = external_parent['commit']

        branch = git_common.current_branch()
        local_base = self.GetCommonAncestorWithUpstream()
        if local_base != external_base:
            print('\nLocal merge base %s is different from Gerrit %s.\n' %
                  (local_base, external_base))
            if git_common.upstream(branch):
                confirm_or_exit(
                    'Can\'t apply the latest changes from Gerrit.\n'
                    'Continue with upload and override the latest changes?')
                return
            print(
                'No upstream branch set. Continuing upload with Gerrit merge base.'
            )

        external_parent_last_uploaded = self._GetChangeCommit(
            revision=local_ps)['parents'][0]
        external_base_last_uploaded = external_parent_last_uploaded['commit']

        if external_base != external_base_last_uploaded:
            print('\nPatch set merge bases are different (%s, %s).\n' %
                  (external_base_last_uploaded, external_base))
            confirm_or_exit(
                'Can\'t apply the latest changes from Gerrit.\n'
                'Continue with upload and override the latest changes?')
            return

        # Fetch Gerrit's CL base if it doesn't exist locally.
        remote, _ = self.GetRemoteBranch()
        if not scm.GIT.IsValidRevision(settings.GetRoot(), external_base):
            RunGitSilent(['fetch', remote, external_base])

        # Get the diff between local_ps and external_ps.
        print('Fetching changes...')
        issue = self.GetIssue()
        changes_ref = 'refs/changes/%02d/%d/' % (issue % 100, issue)
        RunGitSilent(['fetch', remote, changes_ref + str(local_ps)])
        last_uploaded = RunGitSilent(['rev-parse', 'FETCH_HEAD']).strip()
        RunGitSilent(['fetch', remote, changes_ref + str(external_ps)])
        latest_external = RunGitSilent(['rev-parse', 'FETCH_HEAD']).strip()

        # If the commit parents are different, don't apply the diff as it very
        # likely contains many more changes not relevant to this CL.
        parents = RunGitSilent(
            ['rev-parse',
             '%s~1' % (last_uploaded),
             '%s~1' % (latest_external)]).strip().split()
        assert len(parents) == 2, 'Expected two parents.'
        if parents[0] != parents[1]:
            confirm_or_exit(
                'Can\'t apply the latest changes from Gerrit (parent mismatch '
                'between PS).\n'
                'Continue with upload and override the latest changes?')
            return

        diff = RunGitSilent([
            'diff', '--no-ext-diff',
            '%s..%s' % (last_uploaded, latest_external)
        ])

        # Diff can be empty in the case of trivial rebases.
        if not diff:
            return external_base

        # Apply the diff.
        with gclient_utils.temporary_file() as diff_tempfile:
            gclient_utils.FileWrite(diff_tempfile, diff)
            clean_patch = RunGitWithCode(['apply', '--check',
                                          diff_tempfile])[0] == 0
            RunGitSilent(['apply', '-3', '--intent-to-add', diff_tempfile])
            if not clean_patch:
                # Normally patchset is set after upload. But because we exit,
                # that never happens. Updating here makes sure that subsequent
                # uploads don't need to fetch/apply the same diff again.
                self.SetPatchset(external_ps)
                DieWithError(
                    '\nPatch did not apply cleanly. Please resolve any '
                    'conflicts and reupload.')

            message = 'Incorporate external changes from '
            if num_changes == 1:
                message += 'patchset %d' % external_ps
            else:
                message += 'patchsets %d to %d' % (local_ps + 1, external_ps)
            RunGitSilent(['commit', '-am', message])
            # TODO(crbug.com/1382528): Use the previous commit's message as a
            # default patchset title instead of this 'Incorporate' message.
        return external_base

    def _AddChangeIdToCommitMessage(self, log_desc, args):
        """Re-commits using the current message, assumes the commit hook is in
        place.
        """
        RunGit(['commit', '--amend', '-m', log_desc])
        new_log_desc = _create_description_from_log(args)
        if git_footers.get_footer_change_id(new_log_desc):
            print('git-cl: Added Change-Id to commit message.')
            return new_log_desc

        DieWithError('ERROR: Gerrit commit-msg hook not installed.')

    def CannotTriggerTryJobReason(self):
        try:
            data = self._GetChangeDetail()
        except GerritChangeNotExists:
            return 'Gerrit doesn\'t know about your change %s' % self.GetIssue()

        if data['status'] in ('ABANDONED', 'MERGED'):
            return 'CL %s is closed' % self.GetIssue()

    def GetGerritChange(self, patchset=None):
        """Returns a buildbucket.v2.GerritChange message for the current issue."""
        host = urllib.parse.urlparse(self.GetCodereviewServer()).hostname
        issue = self.GetIssue()
        patchset = int(patchset or self.GetPatchset())
        data = self._GetChangeDetail(['ALL_REVISIONS'])

        assert host and issue and patchset, 'CL must be uploaded first'

        has_patchset = any(
            int(revision_data['_number']) == patchset
            for revision_data in data['revisions'].values())
        if not has_patchset:
            raise Exception('Patchset %d is not known in Gerrit change %d' %
                            (patchset, self.GetIssue()))

        return {
            'host': host,
            'change': issue,
            'project': data['project'],
            'patchset': patchset,
        }

    def GetIssueOwner(self):
        return self._GetChangeDetail(['DETAILED_ACCOUNTS'])['owner']['email']

    def GetReviewers(self):
        details = self._GetChangeDetail(['DETAILED_ACCOUNTS'])
        return [r['email'] for r in details['reviewers'].get('REVIEWER', [])]


def _get_bug_line_values(default_project_prefix, bugs):
    """Given default_project_prefix and comma separated list of bugs, yields bug
    line values.

    Each bug can be either:
        * a number, which is combined with default_project_prefix
        * string, which is left as is.

    This function may produce more than one line, because bugdroid expects one
    project per line.

    >>> list(_get_bug_line_values('v8:', '123,chromium:789'))
        ['v8:123', 'chromium:789']
    """
    default_bugs = []
    others = []
    for bug in bugs.split(','):
        bug = bug.strip()
        if bug:
            try:
                default_bugs.append(int(bug))
            except ValueError:
                others.append(bug)

    if default_bugs:
        default_bugs = ','.join(map(str, default_bugs))
        if default_project_prefix:
            if not default_project_prefix.endswith(':'):
                default_project_prefix += ':'
            yield '%s%s' % (default_project_prefix, default_bugs)
        else:
            yield default_bugs
    for other in sorted(others):
        # Don't bother finding common prefixes, CLs with >2 bugs are very very
        # rare.
        yield other


def FindCodereviewSettingsFile(filename='codereview.settings'):
    """Finds the given file starting in the cwd and going up.

    Only looks up to the top of the repository unless an
    'inherit-review-settings-ok' file exists in the root of the repository.
    """
    inherit_ok_file = 'inherit-review-settings-ok'
    cwd = os.getcwd()
    root = settings.GetRoot()
    if os.path.isfile(os.path.join(root, inherit_ok_file)):
        root = None
    while True:
        if os.path.isfile(os.path.join(cwd, filename)):
            return open(os.path.join(cwd, filename))
        if cwd == root:
            break
        parent_dir = os.path.dirname(cwd)
        if parent_dir == cwd:
            # We hit the system root directory.
            break
        cwd = parent_dir
    return None


def LoadCodereviewSettingsFromFile(fileobj):
    """Parses a codereview.settings file and updates hooks."""
    keyvals = gclient_utils.ParseCodereviewSettingsContent(fileobj.read())
    root = settings.GetRoot()

    def SetProperty(name, setting):
        fullname = 'rietveld.' + name
        if setting in keyvals:
            scm.GIT.SetConfig(root, fullname, keyvals[setting])
        else:
            scm.GIT.SetConfig(root, fullname, None, modify_all=True)

    if not keyvals.get('GERRIT_HOST', False):
        SetProperty('server', 'CODE_REVIEW_SERVER')
    # Only server setting is required. Other settings can be absent.
    # In that case, we ignore errors raised during option deletion attempt.
    SetProperty('cc', 'CC_LIST')
    SetProperty('tree-status-url', 'STATUS')
    SetProperty('viewvc-url', 'VIEW_VC')
    SetProperty('bug-prefix', 'BUG_PREFIX')
    SetProperty('cpplint-regex', 'LINT_REGEX')
    SetProperty('cpplint-ignore-regex', 'LINT_IGNORE_REGEX')
    SetProperty('run-post-upload-hook', 'RUN_POST_UPLOAD_HOOK')
    SetProperty('format-full-by-default', 'FORMAT_FULL_BY_DEFAULT')

    if 'GERRIT_HOST' in keyvals:
        scm.GIT.SetConfig(root, 'gerrit.host', keyvals['GERRIT_HOST'])

    if 'GERRIT_SQUASH_UPLOADS' in keyvals:
        scm.GIT.SetConfig(root, 'gerrit.squash-uploads',
                          keyvals['GERRIT_SQUASH_UPLOADS'])

    if 'GERRIT_SKIP_ENSURE_AUTHENTICATED' in keyvals:
        scm.GIT.SetConfig('gerrit.skip-ensure-authenticated',
                          keyvals['GERRIT_SKIP_ENSURE_AUTHENTICATED'])

    if 'PUSH_URL_CONFIG' in keyvals and 'ORIGIN_URL_CONFIG' in keyvals:
        # should be of the form
        # PUSH_URL_CONFIG: url.ssh://gitrw.chromium.org.pushinsteadof
        # ORIGIN_URL_CONFIG: http://src.chromium.org/git
        scm.GIT.SetConfig(keyvals['PUSH_URL_CONFIG'],
                          keyvals['ORIGIN_URL_CONFIG'])


def urlretrieve(source, destination):
    """Downloads a network object to a local file, like urllib.urlretrieve.

    This is necessary because urllib is broken for SSL connections via a proxy.
    """
    with open(destination, 'wb') as f:
        f.write(urllib.request.urlopen(source).read())


def hasSheBang(fname):
    """Checks fname is a #! script."""
    with open(fname) as f:
        return f.read(2).startswith('#!')


def DownloadGerritHook(force):
    """Downloads and installs a Gerrit commit-msg hook.

    Args:
        force: True to update hooks. False to install hooks if not present.
    """
    src = 'https://gerrit-review.googlesource.com/tools/hooks/commit-msg'
    dst = os.path.join(settings.GetRoot(), '.git', 'hooks', 'commit-msg')
    if not os.access(dst, os.X_OK):
        if os.path.exists(dst):
            if not force:
                return
        try:
            urlretrieve(src, dst)
            if not hasSheBang(dst):
                DieWithError('Not a script: %s\n'
                             'You need to download from\n%s\n'
                             'into .git/hooks/commit-msg and '
                             'chmod +x .git/hooks/commit-msg' % (dst, src))
            os.chmod(dst, stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR)
        except Exception:
            if os.path.exists(dst):
                os.remove(dst)
            DieWithError('\nFailed to download hooks.\n'
                         'You need to download from\n%s\n'
                         'into .git/hooks/commit-msg and '
                         'chmod +x .git/hooks/commit-msg' % src)


class _GitCookiesChecker(object):
    """Provides facilities for validating and suggesting fixes to .gitcookies."""
    def __init__(self):
        # Cached list of [host, identity, source], where source is
        # .gitcookies
        self._all_hosts = None

    def ensure_configured_gitcookies(self):
        """Runs checks and suggests fixes to make git use .gitcookies from default
        path."""
        default = gerrit_util.CookiesAuthenticator.get_gitcookies_path()
        configured_path = scm.GIT.GetConfig(settings.GetRoot(),
                                            'http.cookiefile', '')
        configured_path = os.path.expanduser(configured_path)
        if configured_path:
            self._ensure_default_gitcookies_path(configured_path, default)
        else:
            self._configure_gitcookies_path(default)

    @staticmethod
    def _ensure_default_gitcookies_path(configured_path, default_path):
        assert configured_path
        if configured_path == default_path:
            print('git is already configured to use your .gitcookies from %s' %
                  configured_path)
            return

        print('WARNING: You have configured custom path to .gitcookies: %s\n'
              'Gerrit and other depot_tools expect .gitcookies at %s\n' %
              (configured_path, default_path))

        if not os.path.exists(configured_path):
            print('However, your configured .gitcookies file is missing.')
            confirm_or_exit('Reconfigure git to use default .gitcookies?',
                            action='reconfigure')
            scm.GIT.SetConfig(settings.GetRoot(),
                              'http.cookiefile',
                              default_path,
                              scope='global')
            return

        if os.path.exists(default_path):
            print('WARNING: default .gitcookies file already exists %s' %
                  default_path)
            DieWithError(
                'Please delete %s manually and re-run git cl creds-check' %
                default_path)

        confirm_or_exit('Move existing .gitcookies to default location?',
                        action='move')
        shutil.move(configured_path, default_path)
        scm.GIT.SetConfig(settings.GetRoot(),
                          'http.cookiefile',
                          default_path,
                          scope='global')
        print('Moved and reconfigured git to use .gitcookies from %s' %
              default_path)

    @staticmethod
    def _configure_gitcookies_path(default_path):
        print(
            'This tool will guide you through setting up recommended '
            '.gitcookies store for git credentials.\n'
            '\n'
            'IMPORTANT: If something goes wrong and you decide to go back, do:\n'
            '  git config --global --unset http.cookiefile\n'
            '  mv %s %s.backup\n\n' % (default_path, default_path))
        confirm_or_exit(action='setup .gitcookies')
        scm.GIT.SetConfig(settings.GetRoot(),
                          'http.cookiefile',
                          default_path,
                          scope='global')
        print('Configured git to use .gitcookies from %s' % default_path)

    def get_hosts_with_creds(self):
        if self._all_hosts is None:
            a = gerrit_util.CookiesAuthenticator()
            self._all_hosts = [(h, u, '.gitcookies')
                               for h, (u, _) in a.gitcookies.items()
                               if h.endswith(_GOOGLESOURCE)]

        return self._all_hosts

    def print_current_creds(self):
        hosts = sorted(self.get_hosts_with_creds())
        if not hosts:
            print('No Git/Gerrit credentials found.')
            return
        lengths = [max(map(len, (row[i] for row in hosts))) for i in range(3)]
        header = [('Host', 'User', 'Which file'), ['=' * l for l in lengths]]
        print('Your .gitcookies have credentials for these hosts:')
        for row in (header + hosts):
            print('\t'.join((('%%+%ds' % l) % s) for l, s in zip(lengths, row)))

    @staticmethod
    def _parse_identity(identity):
        """Parses identity "git-<username>.domain" into <username> and domain."""
        # Special case: usernames that contain ".", which are generally not
        # distinguishable from sub-domains. But we do know typical domains:
        if identity.endswith('.chromium.org'):
            domain = 'chromium.org'
            username = identity[:-len('.chromium.org')]
        else:
            username, domain = identity.split('.', 1)
        if username.startswith('git-'):
            username = username[len('git-'):]
        return username, domain

    def has_generic_host(self):
        """Returns whether generic .googlesource.com has been configured.

        Chrome Infra recommends to use explicit ${host}.googlesource.com instead.
        """
        for host, _, _ in self.get_hosts_with_creds():
            if host == '.' + _GOOGLESOURCE:
                return True
        return False

    def _get_git_gerrit_identity_pairs(self):
        """Returns map from canonic host to pair of identities (Git, Gerrit).

        One of identities might be None, meaning not configured.
        """
        host_to_identity_pairs = {}
        for host, identity, _ in self.get_hosts_with_creds():
            canonical = _canonical_git_googlesource_host(host)
            pair = host_to_identity_pairs.setdefault(canonical, [None, None])
            idx = 0 if canonical == host else 1
            pair[idx] = identity
        return host_to_identity_pairs

    def get_partially_configured_hosts(self):
        return set(
            (host if i1 else _canonical_gerrit_googlesource_host(host))
            for host, (i1, i2) in self._get_git_gerrit_identity_pairs().items()
            if None in (i1, i2) and host != '.' + _GOOGLESOURCE)

    def get_conflicting_hosts(self):
        return set(
            host
            for host, (i1, i2) in self._get_git_gerrit_identity_pairs().items()
            if None not in (i1, i2) and i1 != i2)

    def get_duplicated_hosts(self):
        counters = collections.Counter(
            h for h, _, _ in self.get_hosts_with_creds())
        return set(host for host, count in counters.items() if count > 1)

    @staticmethod
    def _format_hosts(hosts, extra_column_func=None):
        hosts = sorted(hosts)
        assert hosts
        if extra_column_func is None:
            extras = [''] * len(hosts)
        else:
            extras = [extra_column_func(host) for host in hosts]
        tmpl = '%%-%ds    %%-%ds' % (max(map(len, hosts)), max(map(len,
                                                                   extras)))
        lines = []
        for he in zip(hosts, extras):
            lines.append(tmpl % he)
        return lines

    def _find_problems(self):
        if self.has_generic_host():
            yield ('.googlesource.com wildcard record detected', [
                'Chrome Infrastructure team recommends to list full host names '
                'explicitly.'
            ], None)

        dups = self.get_duplicated_hosts()
        if dups:
            yield ('The following hosts were defined twice',
                   self._format_hosts(dups), None)

        partial = self.get_partially_configured_hosts()
        if partial:
            yield ('Credentials should come in pairs for Git and Gerrit hosts. '
                   'These hosts are missing',
                   self._format_hosts(
                       partial, lambda host: 'but %s defined' %
                       _get_counterpart_host(host)), partial)

        conflicting = self.get_conflicting_hosts()
        if conflicting:
            yield (
                'The following Git hosts have differing credentials from their '
                'Gerrit counterparts',
                self._format_hosts(
                    conflicting, lambda host: '%s vs %s' % tuple(
                        self._get_git_gerrit_identity_pairs()[host])),
                conflicting)

    def find_and_report_problems(self):
        """Returns True if there was at least one problem, else False."""
        found = False
        bad_hosts = set()
        for title, sublines, hosts in self._find_problems():
            if not found:
                found = True
                print('\n\n.gitcookies problem report:\n')
            bad_hosts.update(hosts or [])
            print('  %s%s' % (title, (':' if sublines else '')))
            if sublines:
                print()
                print('    %s' % '\n    '.join(sublines))
            print()

        if bad_hosts:
            assert found
            print(
                '  You can manually remove corresponding lines in your %s file and '
                'visit the following URLs with correct account to generate '
                'correct credential lines:\n' %
                gerrit_util.CookiesAuthenticator.get_gitcookies_path())
            print('    %s' % '\n    '.join(
                sorted(
                    set(gerrit_util.CookiesAuthenticator().get_new_password_url(
                        _canonical_git_googlesource_host(host))
                        for host in bad_hosts))))
        return found


@metrics.collector.collect_metrics('git cl creds-check')
def CMDcreds_check(parser, args):
    """Checks credentials and suggests changes."""
    _, _ = parser.parse_args(args)

    if newauth.Enabled():
        cl = Changelist()
        host = cl.GetGerritHost()
        print(f'Using Gerrit host: {host!r}')
        git_auth.Configure(os.getcwd(), cl)
        # Perform some advisory checks
        email = scm.GIT.GetConfig(os.getcwd(), 'user.email') or ''
        print(f'Using email (configured in Git): {email!r}')
        if gerrit_util.ShouldUseSSO(host, email):
            print('Detected that we should be using SSO.')
        else:
            print('Detected that we should be using git-credential-luci.')
            a = gerrit_util.GitCredsAuthenticator()
            try:
                a.luci_auth.get_access_token()
            except auth.GitLoginRequiredError as e:
                print('NOTE: You are not logged in with git-credential-luci.')
                print(
                    'You will not be able to perform many actions without logging in.'
                )
                print('If you wish to log in, run:')
                print('   ' + e.login_command)
                print('and re-run this command.')
        return 0
    if newauth.ExplicitlyDisabled():
        git_auth.ClearRepoConfig(os.getcwd(), Changelist())

    # Code below checks .gitcookies. Abort if using something else.
    auth_name, _ = gerrit_util.debug_auth()
    if auth_name != "CookiesAuthenticator":
        message = (
            'This command is not designed for bot environment. It checks '
            '~/.gitcookies file not generally used on bots.')
        # TODO(crbug.com/1059384): Automatically detect when running on
        # cloudtop.
        if auth_name == "GceAuthenticator":
            message += (
                '\n'
                'If you need to run this on GCE or a cloudtop instance, '
                'export SKIP_GCE_AUTH_FOR_GIT=1 in your env.')
        DieWithError(message)

    checker = _GitCookiesChecker()
    checker.ensure_configured_gitcookies()

    checker.print_current_creds()

    if not checker.find_and_report_problems():
        print('\nNo problems detected in your .gitcookies file.')
        return 0
    return 1


@metrics.collector.collect_metrics('git cl baseurl')
def CMDbaseurl(parser, args):
    """Gets or sets base-url for this branch."""
    if gclient_utils.IsEnvCog():
        print('baseurl command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    _, args = parser.parse_args(args)
    branchref = scm.GIT.GetBranchRef(settings.GetRoot())
    branch = scm.GIT.ShortBranchName(branchref)
    if not args:
        print('Current base-url:')
        url = scm.GIT.GetConfig(settings.GetRoot(), f'branch.{branch}.base-url')
        if url is None:
            raise ValueError('Missing base URL.')
        return url

    print('Setting base-url to %s' % args[0])
    scm.GIT.SetConfig(settings.GetRoot(), f'branch.{branch}.base-url', args[0])
    return 0


def color_for_status(status):
    """Maps a Changelist status to color, for CMDstatus and other tools."""
    BOLD = '\033[1m'
    return {
        'unsent': BOLD + Fore.YELLOW,
        'waiting': BOLD + Fore.RED,
        'reply': BOLD + Fore.YELLOW,
        'not lgtm': BOLD + Fore.RED,
        'lgtm': BOLD + Fore.GREEN,
        'commit': BOLD + Fore.MAGENTA,
        'closed': BOLD + Fore.CYAN,
        'error': BOLD + Fore.WHITE,
    }.get(status, Fore.WHITE)


def get_cl_statuses(changes: List[Changelist],
                    fine_grained,
                    max_processes=None):
    """Returns a blocking iterable of (cl, status) for given branches.

    If fine_grained is true, this will fetch CL statuses from the server.
    Otherwise, simply indicate if there's a matching url for the given branches.

    If max_processes is specified, it is used as the maximum number of processes
    to spawn to fetch CL status from the server. Otherwise 1 process per branch
    is spawned, up to max of gerrit_util.MAX_CONCURRENT_CONNECTION.

    See GetStatus() for a list of possible statuses.
    """
    if not changes:
        return

    if not fine_grained:
        # Fast path which doesn't involve querying codereview servers.
        # Do not use get_approving_reviewers(), since it requires an HTTP
        # request.
        for cl in changes:
            yield (cl, 'waiting' if cl.GetIssueURL() else 'error')
        return

    # First, sort out authentication issues.
    logging.debug('ensuring credentials exist')
    for cl in changes:
        cl.EnsureAuthenticated(force=False)

    def fetch(cl):
        try:
            return (cl, cl.GetStatus())
        except:
            # See http://crbug.com/629863.
            logging.exception('failed to fetch status for cl %s:',
                              cl.GetIssue())
            raise

    threads_count = min(gerrit_util.MAX_CONCURRENT_CONNECTION, len(changes))
    if max_processes:
        threads_count = max(1, min(threads_count, max_processes))
    logging.debug('querying %d CLs using %d threads', len(changes),
                  threads_count)

    pool = multiprocessing.pool.ThreadPool(threads_count)
    fetched_cls = set()
    try:
        it = pool.imap_unordered(fetch, changes).__iter__()
        while True:
            try:
                cl, status = it.next(timeout=5)
            except (multiprocessing.TimeoutError, StopIteration):
                break
            fetched_cls.add(cl)
            yield cl, status
    finally:
        pool.close()

    # Add any branches that failed to fetch.
    for cl in set(changes) - fetched_cls:
        yield (cl, 'error')


def upload_branch_deps(cl, args, force=False):
    """Uploads CLs of local branches that are dependents of the current branch.

    If the local branch dependency tree looks like:

        test1 -> test2.1 -> test3.1
                         -> test3.2
              -> test2.2 -> test3.3

    and you run "git cl upload --dependencies" from test1 then "git cl upload" is
    run on the dependent branches in this order:
    test2.1, test3.1, test3.2, test2.2, test3.3

    Note: This function does not rebase your local dependent branches. Use it
        when you make a change to the parent branch that will not conflict
        with its dependent branches, and you would like their dependencies
        updated in Gerrit.
        If the new stacked change flow is used, and ancestor diverged, upload
        will fail. To recover, `git rebase-update [-n]` must be executed.
    """
    if git_common.is_dirty_git_tree('upload-branch-deps'):
        return 1

    root_branch = cl.GetBranch()
    if root_branch is None:
        DieWithError('Can\'t find dependent branches from detached HEAD state. '
                     'Get on a branch!')
    if not cl.GetIssue():
        DieWithError(
            'Current branch does not have an uploaded CL. We cannot set '
            'patchset dependencies without an uploaded CL.')

    branches = RunGit([
        'for-each-ref', '--format=%(refname:short) %(upstream:short)',
        'refs/heads'
    ])
    if not branches:
        print('No local branches found.')
        return 0

    # Create a dictionary of all local branches to the branches that are
    # dependent on it.
    tracked_to_dependents = collections.defaultdict(list)
    for b in branches.splitlines():
        tokens = b.split()
        if len(tokens) == 2:
            branch_name, tracked = tokens
            tracked_to_dependents[tracked].append(branch_name)

    print()
    print('The dependent local branches of %s are:' % root_branch)
    dependents = []

    def traverse_dependents_preorder(branch, padding=''):
        dependents_to_process = tracked_to_dependents.get(branch, [])
        padding += '  '
        for dependent in dependents_to_process:
            print('%s%s' % (padding, dependent))
            dependents.append(dependent)
            traverse_dependents_preorder(dependent, padding)

    traverse_dependents_preorder(root_branch)
    print()

    if not dependents:
        print('There are no dependent local branches for %s' % root_branch)
        return 0

    # Record all dependents that failed to upload.
    failures = {}
    # Go through all dependents, checkout the branch and upload.
    try:
        for dependent_branch in dependents:
            print()
            print('--------------------------------------')
            print('Running "git cl upload" from %s:' % dependent_branch)
            RunGit(['checkout', '-q', dependent_branch])
            print()
            try:
                if CMDupload(OptionParser(), args) != 0:
                    print('Upload failed for %s!' % dependent_branch)
                    failures[dependent_branch] = 1
            except Exception as e:
                print(f"Unexpected exception: {e}")
                failures[dependent_branch] = 1
            print()
    finally:
        # Swap back to the original root branch.
        RunGit(['checkout', '-q', root_branch])

    print()
    print('Upload complete for dependent branches!')
    for dependent_branch in dependents:
        upload_status = 'failed' if failures.get(
            dependent_branch) else 'succeeded'
        print('  %s : %s' % (dependent_branch, upload_status))
    print()

    return 0


def GetArchiveTagForBranch(issue_num, branch_name, existing_tags, pattern):
    """Given a proposed tag name, returns a tag name that is guaranteed to be
    unique. If 'foo' is proposed but already exists, then 'foo-2' is used,
    or 'foo-3', and so on."""

    proposed_tag = pattern.format(**{'issue': issue_num, 'branch': branch_name})
    for suffix_num in itertools.count(1):
        if suffix_num == 1:
            to_check = proposed_tag
        else:
            to_check = '%s-%d' % (proposed_tag, suffix_num)

        if to_check not in existing_tags:
            return to_check


@metrics.collector.collect_metrics('git cl archive')
def CMDarchive(parser, args):
    """Archives and deletes branches associated with closed changelists."""
    if gclient_utils.IsEnvCog():
        print('archive command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    parser.add_option(
        '-j',
        '--maxjobs',
        action='store',
        type=int,
        help='The maximum number of jobs to use when retrieving review status.')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='Bypasses the confirmation prompt.')
    parser.add_option('-d',
                      '--dry-run',
                      action='store_true',
                      help='Skip the branch tagging and removal steps.')
    parser.add_option('-t',
                      '--notags',
                      action='store_true',
                      help='Do not tag archived branches. '
                      'Note: local commit history may be lost.')
    parser.add_option('-p',
                      '--pattern',
                      default='git-cl-archived-{issue}-{branch}',
                      help='Format string for archive tags. '
                      'E.g. \'archived-{issue}-{branch}\'.')

    options, args = parser.parse_args(args)
    if args:
        parser.error('Unsupported args: %s' % ' '.join(args))

    branches = RunGit(['for-each-ref', '--format=%(refname)', 'refs/heads'])
    if not branches:
        return 0

    tags = RunGit(['for-each-ref', '--format=%(refname)', 'refs/tags'
                   ]).splitlines() or []
    tags = [t.split('/')[-1] for t in tags]

    print('Finding all branches associated with closed issues...')
    changes = [Changelist(branchref=b) for b in branches.splitlines()]
    alignment = max(5, max(len(c.GetBranch()) for c in changes))
    statuses = get_cl_statuses(changes,
                               fine_grained=True,
                               max_processes=options.maxjobs)
    proposal = [(cl.GetBranch(),
                 GetArchiveTagForBranch(cl.GetIssue(), cl.GetBranch(), tags,
                                        options.pattern))
                for cl, status in statuses
                if status in ('closed', 'rietveld-not-supported')]
    proposal.sort()

    if not proposal:
        print('No branches with closed codereview issues found.')
        return 0

    current_branch = scm.GIT.GetBranch(settings.GetRoot())

    print('\nBranches with closed issues that will be archived:\n')
    if options.notags:
        for next_item in proposal:
            print('  ' + next_item[0])
    else:
        print('%*s | %s' % (alignment, 'Branch name', 'Archival tag name'))
        for next_item in proposal:
            print('%*s   %s' % (alignment, next_item[0], next_item[1]))

    # Quit now on precondition failure or if instructed by the user, either
    # via an interactive prompt or by command line flags.
    if options.dry_run:
        print('\nNo changes were made (dry run).\n')
        return 0

    if any(branch == current_branch for branch, _ in proposal):
        print('You are currently on a branch \'%s\' which is associated with a '
              'closed codereview issue, so archive cannot proceed. Please '
              'checkout another branch and run this command again.' %
              current_branch)
        return 1

    if not options.force:
        answer = gclient_utils.AskForData(
            '\nProceed with deletion (Y/n)? ').lower()
        if answer not in ('y', ''):
            print('Aborted.')
            return 1

    for branch, tagname in proposal:
        if not options.notags:
            RunGit(['tag', tagname, branch])

        if RunGitWithCode(['branch', '-D', branch])[0] != 0:
            # Clean up the tag if we failed to delete the branch.
            RunGit(['tag', '-d', tagname])

    print('\nJob\'s done!')

    return 0


@metrics.collector.collect_metrics('git cl status')
def CMDstatus(parser, args):
    """Show status of changelists.

    Colors are used to tell the state of the CL unless --fast is used:
        - Blue     waiting for review
        - Yellow   waiting for you to reply to review, or not yet sent
        - Green    LGTM'ed
        - Red      'not LGTM'ed
        - Magenta  in the CQ
        - Cyan     was committed, branch can be deleted
        - White    error, or unknown status

    Also see 'git cl comments'.
    """
    if gclient_utils.IsEnvCog():
        print(
            'status command is not supported. Please go to the Gerrit CL '
            'page to check the CL status directly.',
            file=sys.stderr)
        return 1

    parser.add_option('--no-branch-color',
                      action='store_true',
                      help='Disable colorized branch names')
    parser.add_option(
        '--field', help='print only specific field (desc|id|patch|status|url)')
    parser.add_option('-f',
                      '--fast',
                      action='store_true',
                      help='Do not retrieve review status')
    parser.add_option(
        '-j',
        '--maxjobs',
        action='store',
        type=int,
        help='The maximum number of jobs to use when retrieving review status')
    parser.add_option(
        '-i',
        '--issue',
        type=int,
        help='Operate on this issue instead of the current branch\'s implicit '
        'issue. Requires --field to be set.')
    parser.add_option('-d',
                      '--date-order',
                      action='store_true',
                      help='Order branches by committer date.')
    options, args = parser.parse_args(args)
    if args:
        parser.error('Unsupported args: %s' % args)

    if options.issue is not None and not options.field:
        parser.error('--field must be given when --issue is set.')

    if options.field:
        cl = Changelist(issue=options.issue)
        if options.field.startswith('desc'):
            if cl.GetIssue():
                print(cl.FetchDescription())
        elif options.field == 'id':
            issueid = cl.GetIssue()
            if issueid:
                print(issueid)
        elif options.field == 'patch':
            patchset = cl.GetMostRecentPatchset()
            if patchset:
                print(patchset)
        elif options.field == 'status':
            print(cl.GetStatus())
        elif options.field == 'url':
            url = cl.GetIssueURL()
            if url:
                print(url)
        return 0

    branches = RunGit([
        'for-each-ref', '--format=%(refname) %(committerdate:unix)',
        'refs/heads'
    ])
    if not branches:
        print('No local branch found.')
        return 0

    changes = [
        Changelist(branchref=b, commit_date=ct)
        for b, ct in map(lambda line: line.split(' '), branches.splitlines())
    ]
    print('Branches associated with reviews:')
    output = get_cl_statuses(changes,
                             fine_grained=not options.fast,
                             max_processes=options.maxjobs)

    current_branch = scm.GIT.GetBranch(settings.GetRoot())

    def FormatBranchName(branch, colorize=False):
        """Simulates 'git branch' behavior. Colorizes and prefixes branch name with
        an asterisk when it is the current branch."""

        asterisk = ""
        color = Fore.RESET
        if branch == current_branch:
            asterisk = "* "
            color = Fore.GREEN
        branch_name = scm.GIT.ShortBranchName(branch)

        if colorize:
            return asterisk + color + branch_name + Fore.RESET
        return asterisk + branch_name

    branch_statuses = {}

    alignment = max(5,
                    max(len(FormatBranchName(c.GetBranch())) for c in changes))

    if options.date_order or settings.IsStatusCommitOrderByDate():
        sorted_changes = sorted(changes,
                                key=lambda c: c.GetCommitDate(),
                                reverse=True)
    else:
        sorted_changes = sorted(changes, key=lambda c: c.GetBranch())
    for cl in sorted_changes:
        branch = cl.GetBranch()
        while branch not in branch_statuses:
            c, status = next(output)
            branch_statuses[c.GetBranch()] = status
        status = branch_statuses.pop(branch)
        url = cl.GetIssueURL(short=True)
        if url and (not status or status == 'error'):
            # The issue probably doesn't exist anymore.
            url += ' (broken)'

        color = color_for_status(status)
        # Turn off bold as well as colors.
        END = '\033[0m'
        reset = Fore.RESET + END
        if not setup_color.IS_TTY:
            color = ''
            reset = ''
        status_str = '(%s)' % status if status else ''

        branch_display = FormatBranchName(branch)
        padding = ' ' * (alignment - len(branch_display))
        if not options.no_branch_color:
            branch_display = FormatBranchName(branch, colorize=True)

        print('  %s : %s%s %s%s' %
              (padding + branch_display, color, url, status_str, reset))

    print()
    print('Current branch: %s' % current_branch)
    for cl in changes:
        if cl.GetBranch() == current_branch:
            break
    if not cl.GetIssue():
        print('No issue assigned.')
        return 0
    print('Issue number: %s (%s)' % (cl.GetIssue(), cl.GetIssueURL()))
    if not options.fast:
        print('Issue description:')
        print(cl.FetchDescription(pretty=True))
    return 0


def colorize_CMDstatus_doc():
    """To be called once in main() to add colors to git cl status help."""
    colors = [i for i in dir(Fore) if i[0].isupper()]

    def colorize_line(line):
        for color in colors:
            if color in line.upper():
                # Extract whitespace first and the leading '-'.
                indent = len(line) - len(line.lstrip(' ')) + 1
                return line[:indent] + getattr(
                    Fore, color) + line[indent:] + Fore.RESET
        return line

    lines = CMDstatus.__doc__.splitlines()
    CMDstatus.__doc__ = '\n'.join(colorize_line(l) for l in lines)


def write_json(path, contents):
    if path == '-':
        json.dump(contents, sys.stdout)
    else:
        with open(path, 'w') as f:
            json.dump(contents, f)


@subcommand.usage('[issue_number]')
@metrics.collector.collect_metrics('git cl issue')
def CMDissue(parser, args):
    """Sets or displays the current code review issue number.

    Pass issue number 0 to clear the current issue.
    """
    if gclient_utils.IsEnvCog():
        print(
            'issue command is not supported. CL number is available in the '
            'primary side bar at the bottom of your editor. Setting the CL '
            'number is not supported, you can open a new workspace from the '
            'Gerrit CL directly.',
            file=sys.stderr)
        return 1

    parser.add_option('-r',
                      '--reverse',
                      action='store_true',
                      help='Lookup the branch(es) for the specified issues. If '
                      'no issues are specified, all branches with mapped '
                      'issues will be listed.')
    parser.add_option('--json',
                      help='Path to JSON output file, or "-" for stdout.')
    options, args = parser.parse_args(args)

    if options.reverse:
        branches = RunGit(['for-each-ref', 'refs/heads',
                           '--format=%(refname)']).splitlines()
        # Reverse issue lookup.
        issue_branch_map = {}

        git_config = {}
        for name, val in scm.GIT.YieldConfigRegexp(
                settings.GetRoot(), re.compile(r'branch\..*issue')):
            git_config[name] = val

        for branch in branches:
            issue = git_config.get(
                'branch.%s.%s' %
                (scm.GIT.ShortBranchName(branch), ISSUE_CONFIG_KEY))
            if issue:
                issue_branch_map.setdefault(int(issue), []).append(branch)
        if not args:
            args = sorted(issue_branch_map.keys())
        result = {}
        for issue in args:
            try:
                issue_num = int(issue)
            except ValueError:
                print('ERROR cannot parse issue number: %s' % issue,
                      file=sys.stderr)
                continue
            result[issue_num] = issue_branch_map.get(issue_num)
            print('Branch for issue number %s: %s' % (issue, ', '.join(
                issue_branch_map.get(issue_num) or ('None', ))))
        if options.json:
            write_json(options.json, result)
        return 0

    if len(args) > 0:
        issue = ParseIssueNumberArgument(args[0])
        if not issue.valid:
            DieWithError(
                'Pass a url or number to set the issue, 0 to unset it, '
                'or no argument to list it.\n'
                'Maybe you want to run git cl status?')
        cl = Changelist()
        cl.SetIssue(issue.issue)
    else:
        cl = Changelist()
    print('Issue number: %s (%s)' % (cl.GetIssue(), cl.GetIssueURL()))
    if options.json:
        write_json(
            options.json, {
                'gerrit_host': cl.GetGerritHost(),
                'gerrit_project': cl.GetGerritProject(),
                'issue_url': cl.GetIssueURL(),
                'issue': cl.GetIssue(),
            })
    return 0


def _create_commit_message(orig_message, bug=None):
    """Returns a commit message for the cherry picked CL."""
    orig_message_lines = orig_message.splitlines()
    subj_line = orig_message_lines[0]
    new_message = (f'Cherry pick "{subj_line}"\n\n'
                   "Original change's description:\n")
    for line in orig_message_lines:
        new_message += f'> {line}\n'
    new_message += '\n'
    if bug:
        new_message += f'Bug: {bug}\n'
    if change_id := git_footers.get_footer_change_id(orig_message):
        new_message += f'Change-Id: {change_id[0]}\n'
    return new_message


# TODO(b/341792235): Add metrics.
@subcommand.usage('[revisions ...]')
def CMDcherry_pick(parser, args):
    """Upload a chain of cherry picks to Gerrit.

    This should either be run inside the git repo you're trying to make changes
    to or "--host" should be provided.
    """
    parser.add_option('--branch', help='Gerrit branch, e.g. refs/heads/main')
    parser.add_option('--bug',
                      help='Bug to add to the description of each change.')
    parser.add_option('--parent-change-num',
                      type='int',
                      help='The parent change of the first cherry-pick CL, '
                      'i.e. the start of the CL chain.')
    parser.add_option('--host',
                      default=None,
                      help='Gerrit host, needed in case the command is used in '
                      'a non-git environment.')
    options, args = parser.parse_args(args)

    host = None
    if options.host:
        try:
            host = urllib.parse.urlparse(host).hostname
        except ValueError as e:
            print(f'Unable to parse host: {host}. Error: {e}', file=sys.stderr)
            return 1
    else:
        try:
            host = Changelist().GetGerritHost()
        except subprocess2.CalledProcessError:
            pass

    if not host:
        print('Unable to determine host. cherry-pick command is not supported\n'
              'in non-git environment without "--host" provided',
              file=sys.stderr)
        return 1

    if not options.branch:
        parser.error('Branch is required.')
    if not args:
        parser.error('No revisions to cherry pick.')

    change_ids_to_message = {}
    change_ids_to_commit = {}

    # Gerrit needs a change ID for each commit we cherry pick.
    for commit in args:
        # Don't use the ChangeId in the commit message since it may not be
        # unique. Gerrit will error with "Multiple changes found" if we use a
        # non-unique ID. Instead, query Gerrit with the hash and verify it
        # corresponds to a unique CL.
        changes = gerrit_util.QueryChanges(
            host=host,
            params=[('commit', commit)],
            o_params=['CURRENT_REVISION', 'CURRENT_COMMIT'],
        )
        if not changes:
            raise RuntimeError(f'No changes found for {commit}.')
        if len(changes) > 1:
            raise RuntimeError(f'Multiple changes found for {commit}.')

        change = changes[0]
        change_id = change['id']
        message = change['revisions'][commit]['commit']['message']
        change_ids_to_commit[change_id] = commit
        change_ids_to_message[change_id] = message

    print(f'Creating chain of {len(change_ids_to_message)} cherry pick(s)...')

    def print_any_remaining_commits():
        if not change_ids_to_commit:
            return
        print('Remaining commit(s) to cherry pick:')
        for commit in change_ids_to_commit.values():
            print(f'  {commit}')

    # Gerrit only supports cherry picking one commit per change, so we have
    # to cherry pick each commit individually and create a chain of CLs.
    parent_change_num = options.parent_change_num
    for change_id, orig_message in change_ids_to_message.items():
        message = _create_commit_message(orig_message, options.bug)
        orig_subj_line = orig_message.splitlines()[0]

        # Create a cherry pick first, then rebase. If we create a chained CL
        # then cherry pick, the change will lose its relation to the parent.
        try:
            new_change_info = gerrit_util.CherryPick(host,
                                                     change_id,
                                                     options.branch,
                                                     message=message)
        except gerrit_util.GerritError as e:
            print(f'Failed to create cherry pick "{orig_subj_line}": {e}. '
                  'Please resolve any merge conflicts.')
            print_any_remaining_commits()
            return 1

        change_ids_to_commit.pop(change_id)
        new_change_id = new_change_info['id']
        new_change_num = new_change_info['_number']
        new_change_url = gerrit_util.GetChangePageUrl(host, new_change_num)
        print(f'Created cherry pick of "{orig_subj_line}": {new_change_url}')

        if parent_change_num:
            try:
                gerrit_util.RebaseChange(host, new_change_id, parent_change_num)
            except gerrit_util.GerritError as e:
                parent_change_url = gerrit_util.GetChangePageUrl(
                    host, parent_change_num)
                print(f'Failed to rebase {new_change_url} on '
                      f'{parent_change_url}: {e}. Please resolve any merge '
                      'conflicts.')
                print('Once resolved, you can continue the CL chain with '
                      f'`--parent-change-num={new_change_num}` to specify '
                      'which change the chain should start with.\n')
                print_any_remaining_commits()
                return 1

        parent_change_num = new_change_num

    return 0


@metrics.collector.collect_metrics('git cl comments')
def CMDcomments(parser, args):
    """Shows or posts review comments for any changelist."""
    if gclient_utils.IsEnvCog():
        print(
            'comments command is not supported in non-git environment. '
            'Comments on the CL should show up inline in your editor. To '
            'post comments, please visit Gerrit CL page.',
            file=sys.stderr)
        return 1

    parser.add_option('-a',
                      '--add-comment',
                      dest='comment',
                      help='comment to add to an issue')
    parser.add_option('-p',
                      '--publish',
                      action='store_true',
                      help='marks CL as ready and sends comment to reviewers')
    parser.add_option('-i',
                      '--issue',
                      dest='issue',
                      help='review issue id (defaults to current issue).')
    parser.add_option('-m',
                      '--machine-readable',
                      dest='readable',
                      action='store_false',
                      default=True,
                      help='output comments in a format compatible with '
                      'editor parsing')
    parser.add_option('-j',
                      '--json-file',
                      help='File to write JSON summary to, or "-" for stdout')
    options, args = parser.parse_args(args)

    issue = None
    if options.issue:
        try:
            issue = int(options.issue)
        except ValueError:
            DieWithError('A review issue ID is expected to be a number.')

    cl = Changelist(issue=issue)

    if options.comment:
        cl.AddComment(options.comment, options.publish)
        return 0

    summary = sorted(cl.GetCommentsSummary(readable=options.readable),
                     key=lambda c: c.date)
    for comment in summary:
        if comment.disapproval:
            color = Fore.RED
        elif comment.approval:
            color = Fore.GREEN
        elif comment.sender == cl.GetIssueOwner():
            color = Fore.MAGENTA
        elif comment.autogenerated:
            color = Fore.CYAN
        else:
            color = Fore.BLUE
        print('\n%s%s   %s%s\n%s' %
              (color, comment.date.strftime('%Y-%m-%d %H:%M:%S UTC'),
               comment.sender, Fore.RESET, '\n'.join(
                   '  ' + l for l in comment.message.strip().splitlines())))

    if options.json_file:

        def pre_serialize(c):
            dct = c._asdict().copy()
            dct['date'] = dct['date'].strftime('%Y-%m-%d %H:%M:%S.%f')
            return dct

        write_json(options.json_file, [pre_serialize(x) for x in summary])
    return 0


@subcommand.usage('[codereview url or issue id]')
@metrics.collector.collect_metrics('git cl description')
def CMDdescription(parser, args):
    """Brings up the editor for the current CL's description."""
    if gclient_utils.IsEnvCog():
        print(
            'description command is not supported. Please use "Gerrit: Edit '
            'Change Information" functionality in the command palatte to edit '
            'the CL description.',
            file=sys.stderr)
        return 1

    parser.add_option(
        '-d',
        '--display',
        action='store_true',
        help='Display the description instead of opening an editor')
    parser.add_option(
        '-n',
        '--new-description',
        help='New description to set for this issue (- for stdin, '
        '+ to load from local commit HEAD)')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='Delete any unpublished Gerrit edits for this issue '
                      'without prompting')

    options, args = parser.parse_args(args)

    target_issue_arg = None
    if len(args) > 0:
        target_issue_arg = ParseIssueNumberArgument(args[0])
        if not target_issue_arg.valid:
            parser.error('Invalid issue ID or URL.')

    kwargs = {}
    if target_issue_arg:
        kwargs['issue'] = target_issue_arg.issue
        kwargs['codereview_host'] = target_issue_arg.hostname

    cl = Changelist(**kwargs)
    if not cl.GetIssue():
        DieWithError('This branch has no associated changelist.')

    if args and not args[0].isdigit():
        logging.info('canonical issue/change URL: %s\n', cl.GetIssueURL())

    description = ChangeDescription(cl.FetchDescription())

    if options.display:
        print(description.description)
        return 0

    if options.new_description:
        text = options.new_description
        if text == '-':
            text = '\n'.join(l.rstrip() for l in sys.stdin)
        elif text == '+':
            base_branch = cl.GetCommonAncestorWithUpstream()
            text = _create_description_from_log([base_branch])

        description.set_description(text)
    else:
        description.prompt()
    if cl.FetchDescription().strip() != description.description:
        cl.UpdateDescription(description.description, force=options.force)
    return 0


def FindFilesForLint(options, args):
    """Returns the base folder and a list of files to lint."""
    files = []
    cwd = os.getcwd()
    if len(args) == 1 and args[0] == '-':
        # the input is from stdin.
        files.append(args[0])
    elif len(args) > 0:
        # If file paths are given in positional args, run the lint tools
        # against them without using git commands. This allows git_cl.py
        # runnable against any files even out of git checkouts.
        for fn in args:
            if os.path.isfile(fn):
                files.append(fn)
            else:
                print('%s is not a file' % fn)
                return None, None
    elif gclient_utils.IsEnvCog():
        print('Error: missing files to lint')
        return None, None
    else:
        # If file names are omitted, use the git APIs to find the files to lint.
        include_regex = re.compile(settings.GetLintRegex())
        ignore_regex = re.compile(settings.GetLintIgnoreRegex())
        cl = Changelist()
        cwd = settings.GetRoot()
        affectedFiles = cl.GetAffectedFiles(cl.GetCommonAncestorWithUpstream())
        if not affectedFiles:
            print('Cannot lint an empty CL')
            return None, None

        for fn in affectedFiles:
            if not include_regex.match(fn):
                print('Skipping file %s' % fn)
            elif ignore_regex.match(fn):
                print('Ignoring file %s' % fn)
            else:
                files.append(fn)

    return cwd, files


@subcommand.usage('[files ...]')
@metrics.collector.collect_metrics('git cl lint')
def CMDlint(parser, args):
    """Runs cpplint on the current changelist or given files.

    positional arguments:
      files           Files to lint. If omitted, lint all the affected files.
                      If -, lint stdin.
    """
    parser.add_option(
        '--filter',
        action='append',
        metavar='-x,+y',
        help='Comma-separated list of cpplint\'s category-filters')
    options, args = parser.parse_args(args)
    root_path, files = FindFilesForLint(options, args)
    if files is None:
        return 1

    # Access to a protected member _XX of a client class
    # pylint: disable=protected-access
    try:
        import cpplint
        import cpplint_chromium

        # Process cpplint arguments, if any.
        filters = presubmit_canned_checks.GetCppLintFilters(options.filter)
        command = ['--filter=' + ','.join(filters)]
        command.extend(files)
        files_to_lint = cpplint.ParseArguments(command)
    except ImportError:
        print(
            'Your depot_tools is missing cpplint.py and/or cpplint_chromium.py.'
        )
        return 1

    # Change the current working directory before calling lint so that it
    # shows the correct base.
    previous_cwd = os.getcwd()
    try:
        os.chdir(root_path)
        extra_check_functions = [
            cpplint_chromium.CheckPointerDeclarationWhitespace
        ]
        for file in files_to_lint:
            cpplint.ProcessFile(file, cpplint._cpplint_state.verbose_level,
                                extra_check_functions)
    finally:
        os.chdir(previous_cwd)

    print('Total errors found: %d\n' % cpplint._cpplint_state.error_count)
    if cpplint._cpplint_state.error_count != 0:
        return 1
    return 0


@metrics.collector.collect_metrics('git cl presubmit')
@subcommand.usage('[base branch]')
def CMDpresubmit(parser, args):
    """Runs presubmit tests on the current changelist."""
    if gclient_utils.IsEnvCog():
        # TODO - crbug/336555565: give user more instructions on how to
        # trigger presubmit in Cog once the UX is finalized.
        print('presubmit command is not supported in non-git environment.',
              file=sys.stderr)
        return 1
    parser.add_option('-u',
                      '--upload',
                      action='store_true',
                      help='Run upload hook instead of the push hook')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='Run checks even if tree is dirty')
    parser.add_option(
        '--all',
        action='store_true',
        help='Run checks against all files, not just modified ones')
    parser.add_option('--files',
                      nargs=1,
                      help='Semicolon-separated list of files to be marked as '
                      'modified when executing presubmit or post-upload hooks. '
                      'fnmatch wildcards can also be used.')
    parser.add_option(
        '--parallel',
        action='store_true',
        help='Run all tests specified by input_api.RunTests in all '
        'PRESUBMIT files in parallel.')
    parser.add_option('--resultdb',
                      action='store_true',
                      help='Run presubmit checks in the ResultSink environment '
                      'and send results to the ResultDB database.')
    parser.add_option('--realm', help='LUCI realm if reporting to ResultDB')
    parser.add_option('-j',
                      '--json',
                      help='File to write JSON results to, or "-" for stdout')
    options, args = parser.parse_args(args)

    if not options.force and git_common.is_dirty_git_tree('presubmit'):
        print('use --force to check even if tree is dirty.')
        return 1

    cl = Changelist()
    if args:
        base_branch = args[0]
    else:
        # Default to diffing against the common ancestor of the upstream branch.
        base_branch = cl.GetCommonAncestorWithUpstream()

    start = time.time()
    try:
        if not 'PRESUBMIT_SKIP_NETWORK' in os.environ and cl.GetIssue():
            description = cl.FetchDescription()
        else:
            description = _create_description_from_log([base_branch])
    except Exception as e:
        print('Failed to fetch CL description - %s' % str(e))
        description = _create_description_from_log([base_branch])
    elapsed = time.time() - start
    if elapsed > 5:
        print('%.1f s to get CL description.' % elapsed)

    if not base_branch:
        if not options.force:
            print('use --force to check even when not on a branch.')
            return 1
        base_branch = 'HEAD'

    result = cl.RunHook(committing=not options.upload,
                        may_prompt=False,
                        verbose=options.verbose,
                        parallel=options.parallel,
                        upstream=base_branch,
                        description=description,
                        all_files=options.all,
                        files=options.files,
                        resultdb=options.resultdb,
                        realm=options.realm)
    if options.json:
        write_json(options.json, result)
    return 0


def GenerateGerritChangeId(message):
    """Returns the Change ID footer value (Ixxxxxx...xxx).

    Works the same way as
    https://gerrit-review.googlesource.com/tools/hooks/commit-msg
    but can be called on demand on all platforms.

    The basic idea is to generate git hash of a state of the tree, original
    commit message, author/committer info and timestamps.
    """
    lines = []
    tree_hash = RunGitSilent(['write-tree'])
    lines.append('tree %s' % tree_hash.strip())
    code, parent = RunGitWithCode(['rev-parse', 'HEAD~0'],
                                  suppress_stderr=False)
    if code == 0:
        lines.append('parent %s' % parent.strip())
    author = RunGitSilent(['var', 'GIT_AUTHOR_IDENT'])
    lines.append('author %s' % author.strip())
    committer = RunGitSilent(['var', 'GIT_COMMITTER_IDENT'])
    lines.append('committer %s' % committer.strip())
    lines.append('')
    # Note: Gerrit's commit-hook actually cleans message of some lines and
    # whitespace. This code is not doing this, but it clearly won't decrease
    # entropy.
    lines.append(message)
    change_hash = RunCommand(['git', 'hash-object', '-t', 'commit', '--stdin'],
                             stdin=('\n'.join(lines)).encode())
    return 'I%s' % change_hash.strip()


def GetTargetRef(remote, remote_branch, target_branch):
    """Computes the remote branch ref to use for the CL.

    Args:
        remote (str): The git remote for the CL.
        remote_branch (str): The git remote branch for the CL.
        target_branch (str): The target branch specified by the user.
    """
    if not (remote and remote_branch):
        return None

    if target_branch:
        # Canonicalize branch references to the equivalent local full symbolic
        # refs, which are then translated into the remote full symbolic refs
        # below.
        if '/' not in target_branch:
            remote_branch = 'refs/remotes/%s/%s' % (remote, target_branch)
        else:
            prefix_replacements = (
                ('^((refs/)?remotes/)?branch-heads/',
                 'refs/remotes/branch-heads/'),
                ('^((refs/)?remotes/)?%s/' % remote,
                 'refs/remotes/%s/' % remote),
                ('^(refs/)?heads/', 'refs/remotes/%s/' % remote),
            )
            match = None
            for regex, replacement in prefix_replacements:
                match = re.search(regex, target_branch)
                if match:
                    remote_branch = target_branch.replace(
                        match.group(0), replacement)
                    break
            if not match:
                # This is a branch path but not one we recognize; use as-is.
                remote_branch = target_branch
    # pylint: disable=consider-using-get
    elif remote_branch in REFS_THAT_ALIAS_TO_OTHER_REFS:
        # pylint: enable=consider-using-get
        # Handle the refs that need to land in different refs.
        remote_branch = REFS_THAT_ALIAS_TO_OTHER_REFS[remote_branch]

    # Create the true path to the remote branch.
    # Does the following translation:
    # * refs/remotes/origin/refs/diff/test -> refs/diff/test
    # * refs/remotes/origin/main -> refs/heads/main
    # * refs/remotes/branch-heads/test -> refs/branch-heads/test
    if remote_branch.startswith('refs/remotes/%s/refs/' % remote):
        remote_branch = remote_branch.replace('refs/remotes/%s/' % remote, '')
    elif remote_branch.startswith('refs/remotes/%s/' % remote):
        remote_branch = remote_branch.replace('refs/remotes/%s/' % remote,
                                              'refs/heads/')
    elif remote_branch.startswith('refs/remotes/branch-heads'):
        remote_branch = remote_branch.replace('refs/remotes/', 'refs/')

    return remote_branch


def cleanup_list(l):
    """Fixes a list so that comma separated items are put as individual items.

    So that "--reviewers joe@c,john@c --reviewers joa@c" results in
    options.reviewers == sorted(['joe@c', 'john@c', 'joa@c']).
    """
    items = sum((i.split(',') for i in l), [])
    stripped_items = (i.strip() for i in items)
    return sorted(filter(None, stripped_items))


@subcommand.usage('[flags]')
@metrics.collector.collect_metrics('git cl upload')
def CMDupload(parser, args):
    """Uploads the current changelist to codereview.

    Can skip dependency patchset uploads for a branch by running:
        git config branch.branch_name.skip-deps-uploads True
    To unset, run:
        git config --unset branch.branch_name.skip-deps-uploads
    Can also set the above globally by using the --global flag.

    If the name of the checked out branch starts with "bug-" or "fix-" followed
    by a bug number, this bug number is automatically populated in the CL
    description.

    If subject contains text in square brackets or has "<text>: " prefix, such
    text(s) is treated as Gerrit hashtags. For example, CLs with subjects:
        [git-cl] add support for hashtags
        Foo bar: implement foo
    will be hashtagged with "git-cl" and "foo-bar" respectively.
    """
    if gclient_utils.IsEnvCog():
        print(
            'upload command is not supported. Please navigate to source '
            'control view in the activity bar to upload changes to Gerrit.',
            file=sys.stderr)
        return 1

    parser.add_option('--bypass-hooks',
                      action='store_true',
                      dest='bypass_hooks',
                      help='bypass upload presubmit hook')
    parser.add_option('--bypass-watchlists',
                      action='store_true',
                      dest='bypass_watchlists',
                      help='bypass watchlists auto CC-ing reviewers')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      dest='force',
                      help="force yes to questions (don't prompt)")
    parser.add_option('--message',
                      '-m',
                      dest='message',
                      help='DEPRECATED: use --commit-description and/or '
                      '--title instead. message for patchset')
    parser.add_option('-b',
                      '--bug',
                      help='pre-populate the bug number(s) for this issue. '
                      'If several, separate with commas')
    parser.add_option('--message-file',
                      dest='message_file',
                      help='DEPRECATED: use `--commit-description -` and pipe '
                      'the file to stdin instead. File which contains message '
                      'for patchset')
    parser.add_option('--title', '-t', dest='title', help='title for patchset')
    parser.add_option('-T',
                      '--skip-title',
                      action='store_true',
                      dest='skip_title',
                      help='Use the most recent commit message as the title of '
                      'the patchset')
    parser.add_option('-r',
                      '--reviewers',
                      action='append',
                      default=[],
                      help='reviewer email addresses')
    parser.add_option('--cc',
                      action='append',
                      default=[],
                      help='cc email addresses')
    parser.add_option('--hashtag',
                      dest='hashtags',
                      action='append',
                      default=[],
                      help=('Gerrit hashtag for new CL; '
                            'can be applied multiple times'))
    parser.add_option('-s',
                      '--send-mail',
                      '--send-email',
                      dest='send_mail',
                      action='store_true',
                      help='send email to reviewer(s) and cc(s) immediately')
    parser.add_option('--target_branch',
                      '--target-branch',
                      metavar='TARGET',
                      help='Apply CL to remote ref TARGET.  ' +
                      'Default: remote branch head, or main')
    parser.add_option('--squash',
                      action='store_true',
                      help='Squash multiple commits into one')
    parser.add_option('--no-squash',
                      action='store_false',
                      dest='squash',
                      help='Don\'t squash multiple commits into one')
    parser.add_option('--topic',
                      default=None,
                      help='Topic to specify when uploading')
    parser.add_option('--r-owners',
                      dest='add_owners_to',
                      action='store_const',
                      const='R',
                      help='add a set of OWNERS to R')
    parser.add_option('-c',
                      '--use-commit-queue',
                      action='store_true',
                      default=False,
                      help='tell the CQ to commit this patchset; '
                      'implies --send-mail')
    parser.add_option('-d',
                      '--dry-run',
                      '--cq-dry-run',
                      action='store_true',
                      dest='cq_dry_run',
                      default=False,
                      help='Send the patchset to do a CQ dry run right after '
                      'upload.')
    parser.add_option('--set-bot-commit',
                      action='store_true',
                      help=optparse.SUPPRESS_HELP)
    parser.add_option('--preserve-tryjobs',
                      action='store_true',
                      help='instruct the CQ to let tryjobs running even after '
                      'new patchsets are uploaded instead of canceling '
                      'prior patchset\' tryjobs')
    parser.add_option(
        '--dependencies',
        action='store_true',
        help='Uploads CLs of all the local branches that depend on '
        'the current branch')
    parser.add_option(
        '-a',
        '--auto-submit',
        '--enable-auto-submit',
        action='store_true',
        dest='enable_auto_submit',
        help='Sends your change to the CQ after an approval. Only '
        'works on repos that have the Auto-Submit label '
        'enabled')
    parser.add_option(
        '--parallel',
        action='store_true',
        help='Run all tests specified by input_api.RunTests in all '
        'PRESUBMIT files in parallel.')
    parser.add_option('--no-autocc',
                      action='store_true',
                      help='Disables automatic addition of CC emails')
    parser.add_option('--private',
                      action='store_true',
                      help='Set the review private. This implies --no-autocc.')
    parser.add_option('-R',
                      '--retry-failed',
                      action='store_true',
                      help='Retry failed tryjobs from old patchset immediately '
                      'after uploading new patchset. Cannot be used with '
                      '--use-commit-queue or --cq-dry-run.')
    parser.add_option('--fixed',
                      '-x',
                      help='List of bugs that will be commented on and marked '
                      'fixed (pre-populates "Fixed:" tag). Same format as '
                      '-b option / "Bug:" tag. If fixing several issues, '
                      'separate with commas.')
    parser.add_option('--edit-description',
                      action='store_true',
                      default=False,
                      help='Modify description before upload. Cannot be used '
                      'with --force or --commit-description. It is a noop when '
                      '--no-squash is set or a new commit is created.')
    parser.add_option('--commit-description',
                      dest='commit_description',
                      help='Description to set for this issue/CL (- for stdin'
                      ', + to load from local commit HEAD). Can not be used '
                      'with --edit-description, --message or --message-file.'
                      'if `-` is provided, force will be implicitly enabled.')
    parser.add_option('--git-completion-helper',
                      action="store_true",
                      help=optparse.SUPPRESS_HELP)
    parser.add_option('-o',
                      '--push-options',
                      action='append',
                      default=[],
                      help='Transmit the given string to the server when '
                      'performing git push (pass-through). See git-push '
                      'documentation for more details.')
    parser.add_option('--no-add-changeid',
                      action='store_true',
                      dest='no_add_changeid',
                      help='Do not add change-ids to messages.')
    parser.add_option('--cherry-pick-stacked',
                      '--cp',
                      dest='cherry_pick_stacked',
                      action='store_true',
                      help='If parent branch has un-uploaded updates, '
                      'automatically skip parent branches and just upload '
                      'the current branch cherry-pick on its parent\'s last '
                      'uploaded commit. Allows users to skip the potential '
                      'interactive confirmation step.')
    # TODO(b/265929888): Add --wip option of --cl-status option.

    orig_args = args
    (options, args) = parser.parse_args(args)

    if options.git_completion_helper:
        print(' '.join(opt.get_opt_string() for opt in parser.option_list
                       if opt.help != optparse.SUPPRESS_HELP))
        return

    cl = Changelist(branchref=options.target_branch)

    # Do a quick RPC to Gerrit to ensure that our authentication is all working
    # properly. Otherwise `git cl upload` will:
    #   * run `git status` (slow for large repos)
    #   * run presubmit tests (likely slow)
    #   * ask the user to edit the CL description (requires thinking)
    #
    # And then attempt to push the change up to Gerrit, which can fail if
    # authentication is not working properly.
    gerrit_util.GetAccountDetails(cl.GetGerritHost())

    # Check whether git should be updated.
    recommendation = git_common.check_git_version()
    if recommendation:
        print(colorama.Fore.RED)
        print(f'WARNING: {recommendation}')
        print(colorama.Style.RESET_ALL)

    # TODO(crbug.com/1475405): Warn users if the project uses submodules and
    # they have fsmonitor enabled.
    if os.path.isfile('.gitmodules'):
        git_common.warn_submodule()

    if git_common.is_dirty_git_tree('upload'):
        return 1

    options.reviewers = cleanup_list(options.reviewers)
    options.cc = cleanup_list(options.cc)

    if options.edit_description and options.force:
        parser.error('Only one of --force and --edit-description allowed')

    if options.edit_description and options.commit_description:
        parser.error('Only one of --commit-description and --edit-description '
                     'allowed')

    if options.message and options.commit_description:
        parser.error('Only one of --commit-description and --message allowed')

    if options.commit_description and options.commit_description == '-':
        options.force = True

    if options.message_file:
        if options.message:
            parser.error('Only one of --message and --message-file allowed.')
        if options.commit_description:
            parser.error('Only one of --commit-description and --message-file '
                         'allowed.')
        options.message = gclient_utils.FileRead(options.message_file)

    if ([options.cq_dry_run, options.use_commit_queue, options.retry_failed
         ].count(True) > 1):
        parser.error('Only one of --use-commit-queue, --cq-dry-run or '
                     '--retry-failed is allowed.')

    if options.skip_title and options.title:
        parser.error('Only one of --title and --skip-title allowed.')

    if options.use_commit_queue:
        options.send_mail = True

    if options.squash is None:
        # Load default for user, repo, squash=true, in this order.
        options.squash = settings.GetSquashGerritUploads()

    # Warm change details cache now to avoid RPCs later, reducing latency for
    # developers.
    if cl.GetIssue():
        cl._GetChangeDetail([
            'DETAILED_ACCOUNTS', 'CURRENT_REVISION', 'CURRENT_COMMIT', 'LABELS'
        ])

    if options.retry_failed and not cl.GetIssue():
        print('No previous patchsets, so --retry-failed has no effect.')
        options.retry_failed = False

    if options.squash:
        if options.cherry_pick_stacked:
            try:
                orig_args.remove('--cherry-pick-stacked')
            except ValueError:
                orig_args.remove('--cp')
        UploadAllSquashed(options, orig_args)
        if options.dependencies:
            orig_args.remove('--dependencies')
            if not cl.GetIssue():
                # UploadAllSquashed stored an issue number for cl, but it
                # doesn't have access to cl object. By unsetting lookedup_issue,
                # we force code path that will read issue from the config.
                cl.lookedup_issue = False
            return upload_branch_deps(cl, orig_args, options.force)
        return 0
    if options.cherry_pick_stacked:
        parser.error(
            '--cherry-pick-stacked is not available without squash=true,')

    # cl.GetMostRecentPatchset uses cached information, and can return the last
    # patchset before upload. Calling it here makes it clear that it's the
    # last patchset before upload. Note that GetMostRecentPatchset will fail
    # if no CL has been uploaded yet.
    if options.retry_failed:
        patchset = cl.GetMostRecentPatchset()

    ret = cl.CMDUpload(options, args, orig_args)

    if options.retry_failed:
        if ret != 0:
            print('Upload failed, so --retry-failed has no effect.')
            return ret
        builds, _ = _fetch_latest_builds(cl,
                                         DEFAULT_BUILDBUCKET_HOST,
                                         latest_patchset=patchset)
        jobs = _filter_failed_for_retry(builds)
        if len(jobs) == 0:
            print('No failed tryjobs, so --retry-failed has no effect.')
            return ret
        _trigger_tryjobs(cl, jobs, options, patchset + 1)

    return ret


def UploadAllSquashed(options: optparse.Values,
                      orig_args: Sequence[str]) -> int:
    """Uploads the current and upstream branches (if necessary)."""
    cls, cherry_pick_current = _UploadAllPrecheck(options, orig_args)

    # Create commits.
    uploads_by_cl: List[Tuple[Changelist, _NewUpload]] = []
    if cherry_pick_current:
        parent = cls[1]._GitGetBranchConfigValue(GERRIT_SQUASH_HASH_CONFIG_KEY)
        new_upload = cls[0].PrepareCherryPickSquashedCommit(options, parent)
        uploads_by_cl.append((cls[0], new_upload))
    else:
        ordered_cls = list(reversed(cls))

        cl = ordered_cls[0]
        # We can only support external changes when we're only uploading one
        # branch.
        parent: Optional[str] = cl._UpdateWithExternalChanges() if len(
            ordered_cls) == 1 else None
        orig_parent: Optional[str] = None
        if parent is None:
            origin = '.'
            branch = cl.GetBranch()

            while origin == '.':
                # Search for cl's closest ancestor with a gerrit hash.
                origin, upstream_branch_ref = Changelist.FetchUpstreamTuple(
                    branch)
                if origin == '.':
                    upstream_branch = scm.GIT.ShortBranchName(
                        upstream_branch_ref)

                    # Support the `git merge` and `git pull` workflow.
                    if upstream_branch in ['master', 'main']:
                        parent = cl.GetCommonAncestorWithUpstream()
                    else:
                        orig_parent = scm.GIT.GetBranchConfig(
                            settings.GetRoot(), upstream_branch,
                            LAST_UPLOAD_HASH_CONFIG_KEY)
                        parent = scm.GIT.GetBranchConfig(
                            settings.GetRoot(), upstream_branch,
                            GERRIT_SQUASH_HASH_CONFIG_KEY)
                    if parent:
                        break
                    branch = upstream_branch
            else:
                # Either the root of the tree is the cl's direct parent and the
                # while loop above only found empty branches between cl and the
                # root of the tree.
                parent = cl.GetCommonAncestorWithUpstream()

        if orig_parent is None:
            orig_parent = parent
        for i, cl in enumerate(ordered_cls):
            # If we're in the middle of the stack, set end_commit to
            # downstream's direct ancestor.
            if i + 1 < len(ordered_cls):
                child_base_commit = ordered_cls[
                    i + 1].GetCommonAncestorWithUpstream()
            else:
                child_base_commit = None
            new_upload = cl.PrepareSquashedCommit(options,
                                                  parent,
                                                  orig_parent,
                                                  end_commit=child_base_commit)
            uploads_by_cl.append((cl, new_upload))
            parent = new_upload.commit_to_push
            orig_parent = child_base_commit

    # Create refspec options
    cl, new_upload = uploads_by_cl[-1]
    refspec_opts = cl._GetRefSpecOptions(
        options,
        new_upload.change_desc,
        multi_change_upload=len(uploads_by_cl) > 1,
        dogfood_path=True)
    refspec_suffix = ''
    if refspec_opts:
        refspec_suffix = '%' + ','.join(refspec_opts)
        assert ' ' not in refspec_suffix, (
            'spaces not allowed in refspec: "%s"' % refspec_suffix)

    remote, remote_branch = cl.GetRemoteBranch()
    branch = GetTargetRef(remote, remote_branch, options.target_branch)
    refspec = '%s:refs/for/%s%s' % (new_upload.commit_to_push, branch,
                                    refspec_suffix)

    # Git push
    git_push_metadata = {
        'gerrit_host':
        cl.GetGerritHost(),
        'title':
        options.title or '<untitled>',
        'change_id':
        git_footers.get_footer_change_id(new_upload.change_desc.description),
        'description':
        new_upload.change_desc.description,
    }
    push_stdout = cl._RunGitPushWithTraces(refspec, refspec_opts,
                                           git_push_metadata,
                                           options.push_options)

    # Post push updates
    regex = re.compile(r'remote:\s+https?://[\w\-\.\+\/#]*/(\d+)\s.*')
    change_numbers = [
        m.group(1) for m in map(regex.match, push_stdout.splitlines()) if m
    ]

    for i, (cl, new_upload) in enumerate(uploads_by_cl):
        cl.PostUploadUpdates(options, new_upload, change_numbers[i])

    return 0


def _UploadAllPrecheck(
        options: optparse.Values,
        orig_args: Sequence[str]) -> Tuple[Sequence[Changelist], bool]:
    """Checks the state of the tree and gives the user uploading options

    Returns: A tuple of the ordered list of changes that have new commits
        since their last upload and a boolean of whether the user wants to
        cherry-pick and upload the current branch instead of uploading all cls.
    """
    cl = Changelist()
    if cl.GetBranch() is None:
        DieWithError(_NO_BRANCH_ERROR)

    branch_ref = None
    cls: List[Changelist] = []
    must_upload_upstream = False
    first_pass = True

    Changelist._GerritCommitMsgHookCheck(offer_removal=not options.force)

    while True:
        if len(cls) > _MAX_STACKED_BRANCHES_UPLOAD:
            DieWithError(
                'More than %s branches in the stack have not been uploaded.\n'
                'Are your branches in a misconfigured state?\n'
                'If not, please upload some upstream changes first.' %
                (_MAX_STACKED_BRANCHES_UPLOAD))

        cl = Changelist(branchref=branch_ref)

        # Only add CL if it has anything to commit.
        base_commit = cl.GetCommonAncestorWithUpstream()
        end_commit = RunGit(['rev-parse', cl.GetBranchRef()]).strip()

        commit_summary = _GetCommitCountSummary(base_commit, end_commit)
        if commit_summary:
            cls.append(cl)
            if (not first_pass and
                    cl._GitGetBranchConfigValue(GERRIT_SQUASH_HASH_CONFIG_KEY)
                    is None):
                # We are mid-stack and the user must upload their upstream
                # branches.
                must_upload_upstream = True
            print(f'Found change with {commit_summary}...')
        elif first_pass:  # The current branch has nothing to commit. Exit.
            DieWithError('Branch %s has nothing to commit' % cl.GetBranch())
        # Else: A mid-stack branch has nothing to commit. We do not add it to
        # cls.
        first_pass = False

        # Cases below determine if we should continue to traverse up the tree.
        origin, upstream_branch_ref = Changelist.FetchUpstreamTuple(
            cl.GetBranch())
        branch_ref = upstream_branch_ref  # set branch for next run.

        upstream_branch = scm.GIT.ShortBranchName(upstream_branch_ref)
        upstream_last_upload = scm.GIT.GetBranchConfig(
            settings.GetRoot(), upstream_branch, LAST_UPLOAD_HASH_CONFIG_KEY)

        # Case 1: We've reached the beginning of the tree.
        if origin != '.':
            break

        # Case 2: If any upstream branches have never been uploaded,
        # the user MUST upload them unless they are empty. Continue to
        # next loop to add upstream if it is not empty.
        if not upstream_last_upload:
            continue

        # Case 3: If upstream's last_upload == cl.base_commit we do
        # not need to upload any more upstreams from this point on.
        # (Even if there may be diverged branches higher up the tree)
        if base_commit == upstream_last_upload:
            break

        # Case 4: If upstream's last_upload < cl.base_commit we are
        # uploading cl and upstream_cl.
        # Continue up the tree to check other branch relations.
        if scm.GIT.IsAncestor(upstream_last_upload, base_commit):
            continue

        # Case 5: If cl.base_commit < upstream's last_upload the user
        # must rebase before uploading.
        if scm.GIT.IsAncestor(base_commit, upstream_last_upload):
            DieWithError(
                'At least one branch in the stack has diverged from its upstream '
                'branch and does not contain its upstream\'s last upload.\n'
                'Please rebase the stack with `git rebase-update` before uploading.'
            )

        # The tree went through a rebase. LAST_UPLOAD_HASH_CONFIG_KEY no longer
        # has any relation to commits in the tree. Continue up the tree until we
        # hit the root.

    # We assume all cls in the stack have the same auth requirements and only
    # check this once.
    cls[0].EnsureAuthenticated(force=options.force)

    cherry_pick = False
    if len(cls) > 1:
        opt_message = ''
        branches = ', '.join([cl.branch for cl in cls])
        if len(orig_args):
            opt_message = ('options %s will be used for all uploads.\n' %
                           orig_args)
        if must_upload_upstream:
            msg = ('At least one parent branch in `%s` has never been uploaded '
                   'and must be uploaded before/with `%s`.\n' %
                   (branches, cls[1].branch))
            if options.cherry_pick_stacked:
                DieWithError(msg)
            if not options.force:
                confirm_or_exit('\n' + opt_message + msg)
        else:
            if options.cherry_pick_stacked:
                print('cherry-picking `%s` on %s\'s last upload' %
                      (cls[0].branch, cls[1].branch))
                cherry_pick = True
            elif not options.force:
                answer = gclient_utils.AskForData(
                    '\n' + opt_message +
                    'Press enter to update branches %s.\nOr type `n` to upload only '
                    '`%s` cherry-picked on %s\'s last upload:' %
                    (branches, cls[0].branch, cls[1].branch))
                if answer.lower() == 'n':
                    cherry_pick = True
    return cls, cherry_pick


@subcommand.usage('--description=<description file>')
@metrics.collector.collect_metrics('git cl split')
def CMDsplit(parser, args):
    """Splits a branch into smaller branches and uploads CLs.

    Creates a branch and uploads a CL for each group of files modified in the
    current branch that share a common OWNERS file. In the CL description and
    comment, the string '$directory', is replaced with the directory containing
    the shared OWNERS file.
    """
    if gclient_utils.IsEnvCog():
        print('split command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    parser.add_option('-d',
                      '--description',
                      dest='description_file',
                      help='A text file containing a CL description in which '
                      '$directory will be replaced by each CL\'s directory. '
                      'Mandatory except in dry runs.')
    parser.add_option('-c',
                      '--comment',
                      dest='comment_file',
                      help='A text file containing a CL comment.')
    parser.add_option(
        '-n',
        '--dry-run',
        dest='dry_run',
        action='store_true',
        default=False,
        help='List the files and reviewers for each CL that would '
        'be created, but don\'t create branches or CLs.')
    parser.add_option('--cq-dry-run',
                      action='store_true',
                      default=False,
                      help='If set, will do a cq dry run for each uploaded CL. '
                      'Please be careful when doing this; more than ~10 CLs '
                      'has the potential to overload our build '
                      'infrastructure. Try to upload these not during high '
                      'load times (usually 11-3 Mountain View time). Email '
                      'infra-dev@chromium.org with any questions.')
    parser.add_option(
        '-a',
        '--auto-submit',
        '--enable-auto-submit',
        action='store_true',
        dest='enable_auto_submit',
        default=True,
        help='Sends your change to the CQ after an approval. Only '
        'works on repos that have the Auto-Submit label '
        'enabled')
    parser.add_option(
        '--disable-auto-submit',
        action='store_false',
        dest='enable_auto_submit',
        help='Disables automatic sending of the changes to the CQ '
        'after approval. Note that auto-submit only works for '
        'repos that have the Auto-Submit label enabled.')
    parser.add_option(
        '--max-depth',
        type='int',
        default=0,
        help='The max depth to look for OWNERS files. '
        'Controls the granularity of CL splitting by limiting the depth at '
        'which the script searches for OWNERS files. '
        'e.g. let\'s consider file a/b/c/d/e.cc. Without this flag, the '
        'OWNERS file in a/b/c/d/ would be used. '
        'With --max-depth=2, the OWNERS file in a/b/ would be used. '
        'If the OWNERS file is missing in this folder, the first OWNERS file '
        'found in parents is used. '
        'Lower values of --max-depth result in fewer, larger CLs. Higher values '
        'produce more, smaller CLs.')
    parser.add_option('--topic',
                      default=None,
                      help='Topic to specify when uploading')
    options, _ = parser.parse_args(args)

    if not options.description_file and not options.dry_run:
        parser.error('No --description flag specified.')

    def WrappedCMDupload(args):
        return CMDupload(OptionParser(), args)

    return split_cl.SplitCl(options.description_file, options.comment_file,
                            Changelist, WrappedCMDupload, options.dry_run,
                            options.cq_dry_run, options.enable_auto_submit,
                            options.max_depth, options.topic,
                            settings.GetRoot())


@subcommand.usage('DEPRECATED')
@metrics.collector.collect_metrics('git cl commit')
def CMDdcommit(parser, args):
    """DEPRECATED: Used to commit the current changelist via git-svn."""
    message = ('git-cl no longer supports committing to SVN repositories via '
               'git-svn. You probably want to use `git cl land` instead.')
    print(message)
    return 1


@subcommand.usage('[upstream branch to apply against]')
@metrics.collector.collect_metrics('git cl land')
def CMDland(parser, args):
    """Commits the current changelist via git.

    In case of Gerrit, uses Gerrit REST api to "submit" the issue, which pushes
    upstream and closes the issue automatically and atomically.
    """
    if gclient_utils.IsEnvCog():
        print(
            'land command is not supported. Please go to the Gerrit CL '
            'directly to submit the CL',
            file=sys.stderr)
        return 1

    parser.add_option('--bypass-hooks',
                      action='store_true',
                      dest='bypass_hooks',
                      help='bypass upload presubmit hook')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      dest='force',
                      help="force yes to questions (don't prompt)")
    parser.add_option(
        '--parallel',
        action='store_true',
        help='Run all tests specified by input_api.RunTests in all '
        'PRESUBMIT files in parallel.')
    parser.add_option('--resultdb',
                      action='store_true',
                      help='Run presubmit checks in the ResultSink environment '
                      'and send results to the ResultDB database.')
    parser.add_option('--realm', help='LUCI realm if reporting to ResultDB')
    (options, args) = parser.parse_args(args)

    cl = Changelist()

    if not cl.GetIssue():
        DieWithError('You must upload the change first to Gerrit.\n'
                     '  If you would rather have `git cl land` upload '
                     'automatically for you, see http://crbug.com/642759')
    return cl.CMDLand(options.force, options.bypass_hooks, options.verbose,
                      options.parallel, options.resultdb, options.realm)


@subcommand.usage('<patch url or issue id or issue url>')
@metrics.collector.collect_metrics('git cl patch')
def CMDpatch(parser, args):
    """Applies (cherry-picks) a Gerrit changelist locally."""
    if gclient_utils.IsEnvCog():
        print(
            'patch command is not supported. Please create a new workspace '
            'from the Gerrit CL directly',
            file=sys.stderr)
        return 1

    parser.add_option('-b',
                      '--branch',
                      dest='newbranch',
                      help='create a new branch off trunk for the patch')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='overwrite state on the current or chosen branch')
    parser.add_option('-n',
                      '--no-commit',
                      action='store_true',
                      dest='nocommit',
                      help='don\'t commit after patch applies.')

    group = optparse.OptionGroup(
        parser,
        'Options for continuing work on the current issue uploaded from a '
        'different clone (e.g. different machine). Must be used independently '
        'from the other options. No issue number should be specified, and the '
        'branch must have an issue number associated with it')
    group.add_option('--reapply',
                     action='store_true',
                     dest='reapply',
                     help='Reset the branch and reapply the issue.\n'
                     'CAUTION: This will undo any local changes in this '
                     'branch')

    group.add_option('--pull',
                     action='store_true',
                     dest='pull',
                     help='Performs a pull before reapplying.')
    parser.add_option_group(group)

    (options, args) = parser.parse_args(args)

    if options.reapply:
        if options.newbranch:
            parser.error('--reapply works on the current branch only.')
        if len(args) > 0:
            parser.error('--reapply implies no additional arguments.')

        cl = Changelist()
        if not cl.GetIssue():
            parser.error('Current branch must have an associated issue.')

        upstream = cl.GetUpstreamBranch()
        if upstream is None:
            parser.error('No upstream branch specified. Cannot reset branch.')

        RunGit(['reset', '--hard', upstream])
        if options.pull:
            RunGit(['pull'])

        target_issue_arg = ParseIssueNumberArgument(cl.GetIssue())
        return cl.CMDPatchWithParsedIssue(target_issue_arg, options.nocommit,
                                          options.force)

    if len(args) != 1 or not args[0]:
        parser.error('Must specify issue number or URL.')

    target_issue_arg = ParseIssueNumberArgument(args[0])
    if not target_issue_arg.valid:
        parser.error('Invalid issue ID or URL.')

    # We don't want uncommitted changes mixed up with the patch.
    if git_common.is_dirty_git_tree('patch'):
        return 1

    if options.newbranch:
        if options.force:
            RunGit(['branch', '-D', options.newbranch],
                   stderr=subprocess2.PIPE,
                   error_ok=True)
        git_new_branch.create_new_branch(options.newbranch)

    cl = Changelist(codereview_host=target_issue_arg.hostname,
                    issue=target_issue_arg.issue)

    if not args[0].isdigit():
        print('canonical issue/change URL: %s\n' % cl.GetIssueURL())

    return cl.CMDPatchWithParsedIssue(target_issue_arg, options.nocommit,
                                      options.force)


def GetTreeStatus(url=None):
    """Fetches the tree status and returns either 'open', 'closed',
    'unknown' or 'unset'."""
    url = url or settings.GetTreeStatusUrl(error_ok=True)
    if url:
        status = str(urllib.request.urlopen(url).read().lower())
        if status.find('closed') != -1 or status == '0':
            return 'closed'

        if status.find('open') != -1 or status == '1':
            return 'open'

        return 'unknown'
    return 'unset'


def GetTreeStatusReason():
    """Fetches the tree status from a json url and returns the message
    with the reason for the tree to be opened or closed."""
    url = settings.GetTreeStatusUrl()
    json_url = urllib.parse.urljoin(url, '/current?format=json')
    connection = urllib.request.urlopen(json_url)
    status = json.loads(connection.read())
    connection.close()
    return status['message']


@metrics.collector.collect_metrics('git cl tree')
def CMDtree(parser, args):
    """Shows the status of the tree."""
    if gclient_utils.IsEnvCog():
        print(
            'tree command is not supported. Please go to the tree '
            'status UI to check the status of the tree directly.',
            file=sys.stderr)
        return 1

    _, args = parser.parse_args(args)
    status = GetTreeStatus()
    if 'unset' == status:
        print(
            'You must configure your tree status URL by running "git cl config".'
        )
        return 2

    print('The tree is %s' % status)
    print()
    print(GetTreeStatusReason())
    if status != 'open':
        return 1
    return 0


@metrics.collector.collect_metrics('git cl try')
def CMDtry(parser, args):
    """Triggers tryjobs using either Buildbucket or CQ dry run."""
    if gclient_utils.IsEnvCog():
        print(
            'try command is not supported. Please go to the Gerrit CL '
            'page to trigger individual builds or CQ dry run.',
            file=sys.stderr)
        return 1

    group = optparse.OptionGroup(parser, 'Tryjob options')
    group.add_option(
        '-b',
        '--bot',
        action='append',
        help=('IMPORTANT: specify ONE builder per --bot flag. Use it multiple '
              'times to specify multiple builders. ex: '
              '"-b win_rel -b win_layout". See '
              'the try server waterfall for the builders name and the tests '
              'available.'))
    group.add_option(
        '-B',
        '--bucket',
        default='',
        help=('Buildbucket bucket to send the try requests. Format: '
              '"luci.$LUCI_PROJECT.$LUCI_BUCKET". eg: "luci.chromium.try"'))
    group.add_option(
        '-r',
        '--revision',
        help='Revision to use for the tryjob; default: the revision will '
        'be determined by the try recipe that builder runs, which usually '
        'defaults to HEAD of origin/master or origin/main')
    group.add_option('--category',
                     default='git_cl_try',
                     help='Specify custom build category.')
    group.add_option(
        '--project',
        help='Override which project to use. Projects are defined '
        'in recipe to determine to which repository or directory to '
        'apply the patch')
    group.add_option(
        '-p',
        '--property',
        dest='properties',
        action='append',
        default=[],
        help='Specify generic properties in the form -p key1=value1 -p '
        'key2=value2 etc. The value will be treated as '
        'json if decodable, or as string otherwise. '
        'NOTE: using this may make your tryjob not usable for CQ, '
        'which will then schedule another tryjob with default properties')
    group.add_option('--buildbucket-host',
                     default='cr-buildbucket.appspot.com',
                     help='Host of buildbucket. The default host is %default.')
    parser.add_option_group(group)
    parser.add_option('-R',
                      '--retry-failed',
                      action='store_true',
                      default=False,
                      help='Retry failed jobs from the latest set of tryjobs. '
                      'Not allowed with --bucket and --bot options.')
    parser.add_option(
        '-i',
        '--issue',
        type=int,
        help='Operate on this issue instead of the current branch\'s implicit '
        'issue.')
    options, args = parser.parse_args(args)

    # Make sure that all properties are prop=value pairs.
    bad_params = [x for x in options.properties if '=' not in x]
    if bad_params:
        parser.error('Got properties with missing "=": %s' % bad_params)

    if args:
        parser.error('Unknown arguments: %s' % args)

    cl = Changelist(issue=options.issue)
    if not cl.GetIssue():
        parser.error('Need to upload first.')

    # HACK: warm up Gerrit change detail cache to save on RPCs.
    cl._GetChangeDetail(['DETAILED_ACCOUNTS', 'ALL_REVISIONS'])

    error_message = cl.CannotTriggerTryJobReason()
    if error_message:
        parser.error('Can\'t trigger tryjobs: %s' % error_message)

    if options.bot:
        if options.retry_failed:
            parser.error('--bot is not compatible with --retry-failed.')
        if not options.bucket:
            parser.error('A bucket (e.g. "chromium/try") is required.')

        triggered = [b for b in options.bot if 'triggered' in b]
        if triggered:
            parser.error(
                'Cannot schedule builds on triggered bots: %s.\n'
                'This type of bot requires an initial job from a parent (usually a '
                'builder). Schedule a job on the parent instead.\n' % triggered)

        if options.bucket.startswith('.master'):
            parser.error('Buildbot masters are not supported.')

        project, bucket = _parse_bucket(options.bucket)
        if project is None or bucket is None:
            parser.error('Invalid bucket: %s.' % options.bucket)
        jobs = sorted((project, bucket, bot) for bot in options.bot)
    elif options.retry_failed:
        print('Searching for failed tryjobs...')
        builds, patchset = _fetch_latest_builds(cl, DEFAULT_BUILDBUCKET_HOST)
        if options.verbose:
            print('Got %d builds in patchset #%d' % (len(builds), patchset))
        jobs = _filter_failed_for_retry(builds)
        if not jobs:
            print('There are no failed jobs in the latest set of jobs '
                  '(patchset #%d), doing nothing.' % patchset)
            return 0
        num_builders = len(jobs)
        if num_builders > 10:
            confirm_or_exit('There are %d builders with failed builds.' %
                            num_builders,
                            action='continue')
    else:
        if options.verbose:
            print('git cl try with no bots now defaults to CQ dry run.')
        print('Scheduling CQ dry run on: %s' % cl.GetIssueURL())
        return cl.SetCQState(_CQState.DRY_RUN)

    patchset = cl.GetMostRecentPatchset()
    try:
        _trigger_tryjobs(cl, jobs, options, patchset)
    except BuildbucketResponseException as ex:
        print('ERROR: %s' % ex)
        return 1
    return 0


@metrics.collector.collect_metrics('git cl try-results')
def CMDtry_results(parser, args):
    """Prints info about results for tryjobs associated with the current CL."""
    if gclient_utils.IsEnvCog():
        print(
            'try-results command is not supported. Please go to the Gerrit '
            'CL page to see the tryjob results instead.',
            file=sys.stderr)
        return 1

    group = optparse.OptionGroup(parser, 'Tryjob results options')
    group.add_option('-p',
                     '--patchset',
                     type=int,
                     help='patchset number if not current.')
    group.add_option('--print-master',
                     action='store_true',
                     help='print master name as well.')
    group.add_option('--color',
                     action='store_true',
                     default=setup_color.IS_TTY,
                     help='force color output, useful when piping output.')
    group.add_option('--buildbucket-host',
                     default='cr-buildbucket.appspot.com',
                     help='Host of buildbucket. The default host is %default.')
    group.add_option(
        '--json',
        help=('Path of JSON output file to write tryjob results to,'
              'or "-" for stdout.'))
    parser.add_option_group(group)
    parser.add_option(
        '-i',
        '--issue',
        type=int,
        help='Operate on this issue instead of the current branch\'s implicit '
        'issue.')
    options, args = parser.parse_args(args)
    if args:
        parser.error('Unrecognized args: %s' % ' '.join(args))

    cl = Changelist(issue=options.issue)
    if not cl.GetIssue():
        parser.error('Need to upload first.')

    patchset = options.patchset
    if not patchset:
        patchset = cl.GetMostRecentDryRunPatchset()
        if not patchset:
            parser.error('Code review host doesn\'t know about issue %s. '
                         'No access to issue or wrong issue number?\n'
                         'Either upload first, or pass --patchset explicitly.' %
                         cl.GetIssue())

    try:
        jobs = _fetch_tryjobs(cl, DEFAULT_BUILDBUCKET_HOST, patchset)
    except BuildbucketResponseException as ex:
        print('Buildbucket error: %s' % ex)
        return 1
    if options.json:
        write_json(options.json, jobs)
    else:
        _print_tryjobs(options, jobs)
    return 0


@subcommand.usage('[new upstream branch]')
@metrics.collector.collect_metrics('git cl upstream')
def CMDupstream(parser, args):
    """Prints or sets the name of the upstream branch, if any."""
    if gclient_utils.IsEnvCog():
        print('upstream command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    _, args = parser.parse_args(args)
    if len(args) > 1:
        parser.error('Unrecognized args: %s' % ' '.join(args))

    cl = Changelist()
    if args:
        # One arg means set upstream branch.
        branch = cl.GetBranch()
        RunGit(['branch', '--set-upstream-to', args[0], branch])
        # Clear configured merge-base, if there is one.
        git_common.remove_merge_base(branch)
        cl = Changelist()
        print('Upstream branch set to %s' % (cl.GetUpstreamBranch(), ))
    else:
        print(cl.GetUpstreamBranch())
    return 0


@metrics.collector.collect_metrics('git cl web')
def CMDweb(parser, args):
    """Opens the current CL in the web browser."""
    if gclient_utils.IsEnvCog():
        print(
            'web command is not supported. Please use "Gerrit: Open Changes '
            'in Gerrit" functionality in the command palette in the Editor '
            'instead.',
            file=sys.stderr)
        return 1

    parser.add_option('-p',
                      '--print-only',
                      action='store_true',
                      dest='print_only',
                      help='Only print the Gerrit URL, don\'t open it in the '
                      'browser.')
    (options, args) = parser.parse_args(args)
    if args:
        parser.error('Unrecognized args: %s' % ' '.join(args))

    issue_url = Changelist().GetIssueURL()
    if not issue_url:
        print('ERROR No issue to open', file=sys.stderr)
        return 1

    if options.print_only:
        print(issue_url)
        return 0

    # Redirect I/O before invoking browser to hide its output. For example, this
    # allows us to hide the "Created new window in existing browser session."
    # message from Chrome. Based on https://stackoverflow.com/a/2323563.
    saved_stdout = os.dup(1)
    saved_stderr = os.dup(2)
    os.close(1)
    os.close(2)
    os.open(os.devnull, os.O_RDWR)
    try:
        webbrowser.open(issue_url)
    finally:
        os.dup2(saved_stdout, 1)
        os.dup2(saved_stderr, 2)
    return 0


@metrics.collector.collect_metrics('git cl set-commit')
def CMDset_commit(parser, args):
    """Sets the commit bit to trigger the CQ."""
    if gclient_utils.IsEnvCog():
        print(
            'set-commit command is not supported. Please go to the Gerrit CL '
            'page directly to submit the CL.',
            file=sys.stderr)
        return 1

    parser.add_option('-d',
                      '--dry-run',
                      action='store_true',
                      help='trigger in dry run mode')
    parser.add_option('-c',
                      '--clear',
                      action='store_true',
                      help='stop CQ run, if any')
    parser.add_option(
        '-i',
        '--issue',
        type=int,
        help='Operate on this issue instead of the current branch\'s implicit '
        'issue.')
    options, args = parser.parse_args(args)
    if args:
        parser.error('Unrecognized args: %s' % ' '.join(args))
    if [options.dry_run, options.clear].count(True) > 1:
        parser.error('Only one of --dry-run, and --clear are allowed.')

    cl = Changelist(issue=options.issue)
    if not cl.GetIssue():
        parser.error('Must upload the issue first.')

    if options.clear:
        state = _CQState.NONE
    elif options.dry_run:
        state = _CQState.DRY_RUN
    else:
        state = _CQState.COMMIT
    cl.SetCQState(state)
    return 0


@metrics.collector.collect_metrics('git cl set-close')
def CMDset_close(parser, args):
    """Closes the issue."""
    if gclient_utils.IsEnvCog():
        print(
            'set-close command is not supported. Please go to the Gerrit CL '
            'page to abandon the CL instead.',
            file=sys.stderr)
        return 1

    parser.add_option(
        '-i',
        '--issue',
        type=int,
        help='Operate on this issue instead of the current branch\'s implicit '
        'issue.')
    options, args = parser.parse_args(args)
    if args:
        parser.error('Unrecognized args: %s' % ' '.join(args))
    cl = Changelist(issue=options.issue)
    # Ensure there actually is an issue to close.
    if not cl.GetIssue():
        DieWithError('ERROR: No issue to close.')
    cl.CloseIssue()
    return 0


@metrics.collector.collect_metrics('git cl diff')
def CMDdiff(parser, args):
    """Shows differences between local tree and last upload."""
    if gclient_utils.IsEnvCog():
        print(
            'diff command is not supported. Please navigate to source '
            'control view in the activity bar to check the diff.',
            file=sys.stderr)
        return 1

    parser.add_option('--stat',
                      action='store_true',
                      dest='stat',
                      help='Generate a diffstat')
    options, args = parser.parse_args(args)
    if args:
        parser.error('Unrecognized args: %s' % ' '.join(args))

    cl = Changelist()
    issue = cl.GetIssue()
    branch = cl.GetBranch()
    if not issue:
        DieWithError('No issue found for current branch (%s)' % branch)

    base = cl._GitGetBranchConfigValue(LAST_UPLOAD_HASH_CONFIG_KEY)
    if not base:
        base = cl._GitGetBranchConfigValue(GERRIT_SQUASH_HASH_CONFIG_KEY)
    if not base:
        detail = cl._GetChangeDetail(['CURRENT_REVISION', 'CURRENT_COMMIT'])
        revision_info = detail['revisions'][detail['current_revision']]
        fetch_info = revision_info['fetch']['http']
        RunGit(['fetch', fetch_info['url'], fetch_info['ref']])
        base = 'FETCH_HEAD'

    cmd = ['git', 'diff']
    if options.stat:
        cmd.append('--stat')
    cmd.append(base)
    subprocess2.check_call(cmd)

    return 0


@metrics.collector.collect_metrics('git cl owners')
def CMDowners(parser, args):
    """Finds potential owners for reviewing."""
    if gclient_utils.IsEnvCog():
        print('owners command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    parser.add_option(
        '--ignore-current',
        action='store_true',
        help='Ignore the CL\'s current reviewers and start from scratch.')
    parser.add_option('--ignore-self',
                      action='store_true',
                      help='Do not consider CL\'s author as an owners.')
    parser.add_option('--no-color',
                      action='store_true',
                      help='Use this option to disable color output')
    parser.add_option('--batch',
                      action='store_true',
                      help='Do not run interactively, just suggest some')
    # TODO: Consider moving this to another command, since other
    #       git-cl owners commands deal with owners for a given CL.
    parser.add_option('--show-all',
                      action='store_true',
                      help='Show all owners for a particular file')
    options, args = parser.parse_args(args)

    cl = Changelist()
    author = cl.GetAuthor()

    if options.show_all:
        if len(args) == 0:
            print('No files specified for --show-all. Nothing to do.')
            return 0
        owners_by_path = cl.owners_client.BatchListOwners(args)
        for path in args:
            print('Owners for %s:' % path)
            print('\n'.join(
                ' - %s' % owner
                for owner in owners_by_path.get(path, ['No owners found'])))
        return 0

    if args:
        if len(args) > 1:
            parser.error('Unknown args.')
        base_branch = args[0]
    else:
        # Default to diffing against the common ancestor of the upstream branch.
        base_branch = cl.GetCommonAncestorWithUpstream()

    affected_files = cl.GetAffectedFiles(base_branch)

    if options.batch:
        owners = cl.owners_client.SuggestOwners(affected_files,
                                                exclude=[author])
        print('\n'.join(owners))
        return 0

    return owners_finder.OwnersFinder(
        affected_files,
        author, [] if options.ignore_current else cl.GetReviewers(),
        cl.owners_client,
        disable_color=options.no_color,
        ignore_author=options.ignore_self).run()


def _SplitArgsByCmdLineLimit(args):
    """Splits a list of arguments into shorter lists that fit within the command
    line limit."""
    # The maximum command line length is 32768 characters on Windows and 2097152
    # characters on other platforms. Use a lower limit to be safe.
    command_line_limit = 30000 if sys.platform.startswith('win32') else 2000000

    batch_args = []
    batch_length = 0
    for arg in args:
        # Add 1 to account for the space between arguments.
        arg_length = len(arg) + 1
        # If the current argument is too long to fit in a single command line,
        # split it into smaller parts.
        if batch_length + arg_length > command_line_limit and batch_args:
            yield batch_args
            batch_args = []
            batch_length = 0

        batch_args.append(arg)
        batch_length += arg_length

    if batch_args:
        yield batch_args


def RunGitDiffCmd(diff_type,
                  upstream_commit,
                  files,
                  allow_prefix=False,
                  **kwargs):
    """Generates and runs diff command."""
    # Generate diff for the current branch's changes.
    diff_cmd = ['-c', 'core.quotePath=false', 'diff', '--no-ext-diff']

    if allow_prefix:
        # explicitly setting --src-prefix and --dst-prefix is necessary in the
        # case that diff.noprefix is set in the user's git config.
        diff_cmd += ['--src-prefix=a/', '--dst-prefix=b/']
    else:
        diff_cmd += ['--no-prefix']

    diff_cmd += diff_type
    diff_cmd += [upstream_commit, '--']

    if not files:
        return RunGit(diff_cmd, **kwargs)

    for file in files:
        if file != '-' and not os.path.isdir(file) and not os.path.isfile(file):
            DieWithError('Argument "%s" is not a file or a directory' % file)

    output = ''
    for files_batch in _SplitArgsByCmdLineLimit(files):
        output += RunGit(diff_cmd + files_batch, **kwargs)

    return output


def _RunClangFormatDiff(opts, clang_diff_files, top_dir, upstream_commit):
    """Runs clang-format-diff and sets a return value if necessary."""
    # Set to 2 to signal to CheckPatchFormatted() that this patch isn't
    # formatted. This is used to block during the presubmit.
    return_value = 0

    # Locate the clang-format binary in the checkout
    try:
        clang_format_tool = clang_format.FindClangFormatToolInChromiumTree()
    except clang_format.NotFoundError as e:
        DieWithError(e)

    if opts.full or settings.GetFormatFullByDefault():
        cmd = [clang_format_tool]
        if not opts.dry_run and not opts.diff:
            cmd.append('-i')
        if opts.dry_run:
            for diff_file in clang_diff_files:
                with open(diff_file, 'r') as myfile:
                    code = myfile.read().replace('\r\n', '\n')
                    stdout = RunCommand(cmd + [diff_file], cwd=top_dir)
                    stdout = stdout.replace('\r\n', '\n')
                    if opts.diff:
                        sys.stdout.write(stdout)
                    if code != stdout:
                        return_value = 2
        else:
            stdout = RunCommand(cmd + clang_diff_files, cwd=top_dir)
            if opts.diff:
                sys.stdout.write(stdout)
    else:
        try:
            script = clang_format.FindClangFormatScriptInChromiumTree(
                'clang-format-diff.py')
        except clang_format.NotFoundError as e:
            DieWithError(e)

        cmd = ['vpython3', script, '-p0']
        if not opts.dry_run and not opts.diff:
            cmd.append('-i')

        diff_output = RunGitDiffCmd(['-U0'], upstream_commit, clang_diff_files)

        env = os.environ.copy()
        env['PATH'] = (str(os.path.dirname(clang_format_tool)) + os.pathsep +
                       env['PATH'])
        # If `clang-format-diff.py` is run without `-i` and the diff is
        # non-empty, it returns an error code of 1. This will cause `RunCommand`
        # to die with an error if `error_ok` is not set.
        stdout = RunCommand(cmd,
                            error_ok=True,
                            stdin=diff_output.encode(),
                            cwd=top_dir,
                            env=env,
                            shell=sys.platform.startswith('win32'))
        if opts.diff:
            sys.stdout.write(stdout)
        if opts.dry_run and len(stdout) > 0:
            return_value = 2

    return return_value


def _RunGoogleJavaFormat(opts, paths, top_dir, upstream_commit):
    """Runs google-java-format and sets a return value if necessary."""
    tool = google_java_format.FindGoogleJavaFormat()
    if tool is None:
        # Fail silently. It could be we are on an old chromium revision, or that
        # it is a non-chromium project. https://crbug.com/1491627
        print('google-java-format not found, skipping java formatting.')
        return 0

    base_cmd = [tool, '--aosp']
    if not opts.diff:
        if opts.dry_run:
            base_cmd += ['--dry-run']
        else:
            base_cmd += ['--replace']

    changed_lines_only = not (opts.full or settings.GetFormatFullByDefault())
    if changed_lines_only:
        # Format two lines around each changed line so that the correct amount
        # of blank lines will be added between symbols.
        line_diffs = _ComputeFormatDiffLineRanges(paths,
                                                  upstream_commit,
                                                  expand=2)

    def RunFormat(cmd, path, range_args, **kwds):
        stdout = RunCommand(cmd + range_args + [path], **kwds)

        if changed_lines_only:
            # google-java-format will not remove unused imports because they
            # do not fall within the changed lines. Run the command again to
            # remove them.
            if opts.diff:
                stdout = RunCommand(cmd + ['--fix-imports-only', '-'],
                                    stdin=stdout.encode(),
                                    **kwds)
            else:
                stdout += RunCommand(cmd + ['--fix-imports-only', path], **kwds)

        # If --diff is passed, google-java-format will output formatted content.
        # Diff it with the existing file in the checkout and output the result.
        if opts.diff:
            stdout = RunGitDiffCmd(['-U3'],
                                   '--no-index', [path, '-'],
                                   stdin=stdout.encode(),
                                   **kwds)
        return stdout

    results = []
    kwds = {'error_ok': True, 'cwd': top_dir}
    with multiprocessing.pool.ThreadPool() as pool:
        for path in paths:
            cmd = base_cmd.copy()
            range_args = []
            if changed_lines_only:
                ranges = line_diffs.get(path)
                if not ranges:
                    # E.g. There were only deleted lines.
                    continue
                range_args = ['--lines={}:{}'.format(a, b) for a, b in ranges]

            results.append(
                pool.apply_async(RunFormat,
                                 args=[cmd, path, range_args],
                                 kwds=kwds))

        return_value = 0
        for result in results:
            stdout = result.get()
            if stdout:
                if opts.diff:
                    sys.stdout.write('Requires formatting: ' + stdout)
                else:
                    return_value = 2

        return return_value


def _RunRustFmt(opts, rust_diff_files, top_dir, upstream_commit):
    """Runs rustfmt.  Just like _RunClangFormatDiff returns 2 to indicate that
    presubmit checks have failed (and returns 0 otherwise)."""
    # Locate the rustfmt binary.
    try:
        rustfmt_tool = rustfmt.FindRustfmtToolInChromiumTree()
    except rustfmt.NotFoundError as e:
        DieWithError(e)

    chromium_src_path = gclient_paths.GetPrimarySolutionPath()
    rustfmt_toml_path = os.path.join(chromium_src_path, '.rustfmt.toml')

    # TODO(crbug.com/1440869): Support formatting only the changed lines
    # if `opts.full or settings.GetFormatFullByDefault()` is False.
    cmd = [rustfmt_tool, f'--config-path={rustfmt_toml_path}']
    if opts.dry_run:
        cmd.append('--check')
    cmd += rust_diff_files
    rustfmt_exitcode = subprocess2.call(cmd)

    if opts.presubmit and rustfmt_exitcode != 0:
        return 2

    return 0


def _RunSwiftFormat(opts, swift_diff_files, top_dir, upstream_commit):
    """Runs swift-format.  Just like _RunClangFormatDiff returns 2 to indicate
    that presubmit checks have failed (and returns 0 otherwise)."""
    if sys.platform != 'darwin':
        DieWithError('swift-format is only supported on macOS.')
    # Locate the swift-format binary.
    try:
        swift_format_tool = swift_format.FindSwiftFormatToolInChromiumTree()
    except swift_format.NotFoundError as e:
        DieWithError(e)

    cmd = [swift_format_tool]
    if opts.dry_run:
        cmd += ['lint', '-s']
    else:
        cmd += ['format', '-i']
    cmd += swift_diff_files
    swift_format_exitcode = subprocess2.call(cmd)

    if opts.presubmit and swift_format_exitcode != 0:
        return 2

    return 0


def _RunYapf(opts, paths, top_dir, upstream_commit):
    depot_tools_path = os.path.dirname(os.path.abspath(__file__))
    yapf_tool = os.path.join(depot_tools_path, 'yapf')

    # Used for caching.
    yapf_configs = {}
    for p in paths:
        # Find the yapf style config for the current file, defaults to depot
        # tools default.
        _FindYapfConfigFile(p, yapf_configs, top_dir)

    # Turn on python formatting by default if a yapf config is specified.
    # This breaks in the case of this repo though since the specified
    # style file is also the global default.
    if opts.python is None:
        paths = [
            p for p in paths
            if _FindYapfConfigFile(p, yapf_configs, top_dir) is not None
        ]

    # Note: yapf still seems to fix indentation of the entire file
    # even if line ranges are specified.
    # See https://github.com/google/yapf/issues/499
    if not opts.full and paths:
        line_diffs = _ComputeFormatDiffLineRanges(paths, upstream_commit)

    yapfignore_patterns = _GetYapfIgnorePatterns(top_dir)
    paths = _FilterYapfIgnoredFiles(paths, yapfignore_patterns)

    return_value = 0
    for path in paths:
        yapf_style = _FindYapfConfigFile(path, yapf_configs, top_dir)
        # Default to pep8 if not .style.yapf is found.
        if not yapf_style:
            yapf_style = 'pep8'

        cmd = ['vpython3', yapf_tool, '--style', yapf_style, path]

        if not opts.full:
            ranges = line_diffs.get(path)
            if not ranges:
                continue
            # Only run yapf over changed line ranges.
            for diff_start, diff_end in ranges:
                cmd += ['-l', '{}-{}'.format(diff_start, diff_end)]

        if opts.diff or opts.dry_run:
            cmd += ['--diff']
            # Will return non-zero exit code if non-empty diff.
            stdout = RunCommand(cmd,
                                error_ok=True,
                                stderr=subprocess2.PIPE,
                                cwd=top_dir,
                                shell=sys.platform.startswith('win32'))
            if opts.diff:
                sys.stdout.write(stdout)
            elif len(stdout) > 0:
                return_value = 2
        else:
            cmd += ['-i']
            RunCommand(cmd, cwd=top_dir, shell=sys.platform.startswith('win32'))
    return return_value


def _RunGnFormat(opts, paths, top_dir, upstream_commit):
    cmd = ['gn', 'format']
    if opts.dry_run or opts.diff:
        cmd.append('--dry-run')
    return_value = 0
    for path in paths:
        gn_ret = subprocess2.call(cmd + [path],
                                  shell=sys.platform.startswith('win'),
                                  cwd=top_dir)
        if opts.dry_run and gn_ret == 2:
            return_value = 2  # Not formatted.
        elif opts.diff and gn_ret == 2:
            # TODO this should compute and print the actual diff.
            print('This change has GN build file diff for ' + path)
        elif gn_ret != 0:
            # For non-dry run cases (and non-2 return values for dry-run), a
            # nonzero error code indicates a failure, probably because the
            # file doesn't parse.
            DieWithError('gn format failed on ' + path +
                         '\nTry running `gn format` on this file manually.')
    return return_value


def _RunMojomFormat(opts, paths, top_dir, upstream_commit):
    primary_solution_path = gclient_paths.GetPrimarySolutionPath()
    if not primary_solution_path:
        DieWithError('Could not find the primary solution path (e.g. '
                     'the chromium checkout)')
    mojom_format_path = os.path.join(primary_solution_path, 'mojo', 'public',
                                     'tools', 'mojom', 'mojom_format.py')
    if not os.path.exists(mojom_format_path):
        DieWithError('Could not find mojom formater at '
                     f'"{mojom_format_path}"')

    cmd = ['vpython3', mojom_format_path]
    if opts.dry_run:
        cmd.append('--dry-run')
    cmd.extend(paths)

    ret = subprocess2.call(cmd)
    if opts.dry_run and ret != 0:
        return 2

    return ret


def _RunMetricsXMLFormat(opts, paths, top_dir, upstream_commit):
    # Skip the metrics formatting from the global presubmit hook. These files
    # have a separate presubmit hook that issues an error if the files need
    # formatting, whereas the top-level presubmit script merely issues a
    # warning. Formatting these files is somewhat slow, so it's important not to
    # duplicate the work.
    if opts.presubmit:
        return 0

    return_value = 0
    for path in paths:
        pretty_print_tool = metrics_xml_format.FindMetricsXMLFormatterTool(path)
        if not pretty_print_tool:
            continue

        cmd = [shutil.which('vpython3'), pretty_print_tool, '--non-interactive']
        # If the XML file is histograms.xml or enums.xml, add the xml path
        # to the command as histograms/pretty_print.py now needs a relative
        # path argument after splitting the histograms into multiple
        # directories. For example, in tools/metrics/ukm, pretty-print could
        # be run using: $ python pretty_print.py But in
        # tools/metrics/histogrmas, pretty-print should be run with an
        # additional relative path argument, like: $ python pretty_print.py
        # metadata/UMA/histograms.xml $ python pretty_print.py enums.xml
        metricsDir = metrics_xml_format.GetMetricsDir(top_dir, path)
        histogramsDir = os.path.join(top_dir, 'tools', 'metrics', 'histograms')
        if metricsDir == histogramsDir:
            cmd.append(path)
        if opts.dry_run or opts.diff:
            cmd.append('--diff')

        stdout = RunCommand(cmd, cwd=top_dir)
        if opts.diff:
            sys.stdout.write(stdout)
        if opts.dry_run and stdout:
            return_value = 2  # Not formatted.
    return return_value


def MatchingFileType(file_name, extensions):
    """Returns True if the file name ends with one of the given extensions."""
    return bool([ext for ext in extensions if file_name.lower().endswith(ext)])


@subcommand.usage('[files or directories to diff]')
@metrics.collector.collect_metrics('git cl format')
def CMDformat(parser, args):
    """Runs auto-formatting tools (clang-format etc.) on the diff."""
    if gclient_utils.IsEnvCog():
        print(
            'format command is not supported. Please use the "Format '
            'Document" functionality in command palette in the Editor '
            'instead.',
            file=sys.stderr)
        return 1
    clang_exts = ['.cc', '.cpp', '.h', '.m', '.mm', '.proto']
    GN_EXTS = ['.gn', '.gni', '.typemap']
    parser.add_option('--full',
                      action='store_true',
                      help='Reformat the full content of all touched files')
    parser.add_option('--upstream', help='Branch to check against')
    parser.add_option('--dry-run',
                      action='store_true',
                      help='Don\'t modify any file on disk.')
    parser.add_option(
        '--no-clang-format',
        dest='clang_format',
        action='store_false',
        default=True,
        help='Disables formatting of various file types using clang-format.')
    parser.add_option('--python',
                      action='store_true',
                      help='Enables python formatting on all python files.')
    parser.add_option(
        '--no-python',
        action='store_false',
        dest='python',
        help='Disables python formatting on all python files. '
        'If neither --python or --no-python are set, python files that have a '
        '.style.yapf file in an ancestor directory will be formatted. '
        'It is an error to set both.')
    parser.add_option('--js',
                      action='store_true',
                      help='Format javascript code with clang-format. '
                      'Has no effect if --no-clang-format is set.')
    parser.add_option('--diff',
                      action='store_true',
                      help='Print diff to stdout rather than modifying files.')
    parser.add_option('--presubmit',
                      action='store_true',
                      help='Used when running the script from a presubmit.')

    parser.add_option(
        '--rust-fmt',
        dest='use_rust_fmt',
        action='store_true',
        default=rustfmt.IsRustfmtSupported(),
        help='Enables formatting of Rust file types using rustfmt.')
    parser.add_option(
        '--no-rust-fmt',
        dest='use_rust_fmt',
        action='store_false',
        help='Disables formatting of Rust file types using rustfmt.')

    parser.add_option(
        '--swift-format',
        dest='use_swift_format',
        action='store_true',
        default=swift_format.IsSwiftFormatSupported(),
        help='Enables formatting of Swift file types using swift-format '
        '(macOS host only).')
    parser.add_option(
        '--no-swift-format',
        dest='use_swift_format',
        action='store_false',
        help='Disables formatting of Swift file types using swift-format.')

    parser.add_option('--mojom',
                      action='store_true',
                      help='Enables formatting of .mojom files.')

    parser.add_option('--no-java',
                      action='store_true',
                      help='Disable auto-formatting of .java')

    opts, files = parser.parse_args(args)

    # Normalize files against the current path, so paths relative to the
    # current directory are still resolved as expected.
    files = [os.path.join(os.getcwd(), file) for file in files]

    # git diff generates paths against the root of the repository.  Change
    # to that directory so clang-format can find files even within subdirs.
    rel_base_path = settings.GetRelativeRoot()
    if rel_base_path:
        os.chdir(rel_base_path)

    # Grab the merge-base commit, i.e. the upstream commit of the current
    # branch when it was created or the last time it was rebased. This is
    # to cover the case where the user may have called "git fetch origin",
    # moving the origin branch to a newer commit, but hasn't rebased yet.
    upstream_commit = None
    upstream_branch = opts.upstream
    if not upstream_branch:
        cl = Changelist()
        upstream_branch = cl.GetUpstreamBranch()
    if upstream_branch:
        upstream_commit = RunGit(['merge-base', 'HEAD', upstream_branch])
        upstream_commit = upstream_commit.strip()

    if not upstream_commit:
        DieWithError('Could not find base commit for this branch. '
                     'Are you in detached state?')

    # Filter out deleted files
    diff_output = RunGitDiffCmd(['--name-only', '--diff-filter=d'],
                                upstream_commit, files)
    diff_files = diff_output.splitlines()

    if opts.js:
        clang_exts.extend(['.js', '.ts'])

    formatters = [
        (GN_EXTS, _RunGnFormat),
        (['.xml'], _RunMetricsXMLFormat),
    ]
    if not opts.no_java:
        formatters += [(['.java'], _RunGoogleJavaFormat)]
    if opts.clang_format:
        formatters += [(clang_exts, _RunClangFormatDiff)]
    if opts.use_rust_fmt:
        formatters += [(['.rs'], _RunRustFmt)]
    if opts.use_swift_format:
        formatters += [(['.swift'], _RunSwiftFormat)]
    if opts.python is not False:
        formatters += [(['.py'], _RunYapf)]
    if opts.mojom:
        formatters += [(['.mojom'], _RunMojomFormat)]

    top_dir = settings.GetRoot()
    return_value = 0
    for file_types, format_func in formatters:
        paths = [p for p in diff_files if MatchingFileType(p, file_types)]
        if not paths:
            continue
        ret = format_func(opts, paths, top_dir, upstream_commit)
        return_value = return_value or ret

    return return_value


@subcommand.usage('<codereview url or issue id>')
@metrics.collector.collect_metrics('git cl checkout')
def CMDcheckout(parser, args):
    """Checks out a branch associated with a given Gerrit issue."""
    if gclient_utils.IsEnvCog():
        print('checkout command is not supported in non-git environment.',
              file=sys.stderr)
        return 1

    _, args = parser.parse_args(args)

    if len(args) != 1:
        parser.print_help()
        return 1

    issue_arg = ParseIssueNumberArgument(args[0])
    if not issue_arg.valid:
        parser.error('Invalid issue ID or URL.')

    target_issue = str(issue_arg.issue)

    output = scm.GIT.YieldConfigRegexp(settings.GetRoot(),
                                       r'branch\..*\.' + ISSUE_CONFIG_KEY)

    branches = []
    for key, issue in output:
        if issue == target_issue:
            branches.append(
                re.sub(r'branch\.(.*)\.' + ISSUE_CONFIG_KEY, r'\1', key))

    if len(branches) == 0:
        print('No branch found for issue %s.' % target_issue)
        return 1
    if len(branches) == 1:
        RunGit(['checkout', branches[0]])
    else:
        print('Multiple branches match issue %s:' % target_issue)
        for i in range(len(branches)):
            print('%d: %s' % (i, branches[i]))
        which = gclient_utils.AskForData('Choose by index: ')
        try:
            RunGit(['checkout', branches[int(which)]])
        except (IndexError, ValueError):
            print('Invalid selection, not checking out any branch.')
            return 1

    return 0


def CMDlol(parser, args):
    # This command is intentionally undocumented.
    print(
        zlib.decompress(
            base64.b64decode(
                'eNptkLEOwyAMRHe+wupCIqW57v0Vq84WqWtXyrcXnCBsmgMJ+/SSAxMZgRB6NzE'
                'E2ObgCKJooYdu4uAQVffUEoE1sRQLxAcqzd7uK2gmStrll1ucV3uZyaY5sXyDd9'
                'JAnN+lAXsOMJ90GANAi43mq5/VeeacylKVgi8o6F1SC63FxnagHfJUTfUYdCR/W'
                'Ofe+0dHL7PicpytKP750Fh1q2qnLVof4w8OZWNY')).decode('utf-8'))
    return 0


def CMDversion(parser, args):
    import utils
    print(utils.depot_tools_version())


class OptionParser(optparse.OptionParser):
    """Creates the option parse and add --verbose support."""
    def __init__(self, *args, **kwargs):
        optparse.OptionParser.__init__(self,
                                       *args,
                                       prog='git cl',
                                       version=__version__,
                                       **kwargs)
        self.add_option('-v',
                        '--verbose',
                        action='count',
                        default=0,
                        help='Use 2 times for more debugging info')

    @typing.overload
    def parse_args(
        self,
        args: None = None,
        values: Optional[optparse.Values] = None
    ) -> tuple[optparse.Values, list[str]]:
        ...

    @typing.overload
    def parse_args(  # pylint: disable=signature-differs
        self,
        args: Sequence[AnyStr],
        values: Optional[optparse.Values] = None
    ) -> tuple[optparse.Values, list[AnyStr]]:
        ...

    def parse_args(self, args: Sequence[AnyStr] | None = None, values=None):
        try:
            return self._parse_args(args)
        finally:
            if metrics.collector.config.should_collect_metrics:
                try:
                    # GetViewVCUrl ultimately calls logging method.
                    project_url = settings.GetViewVCUrl().strip('/+')
                    if project_url in metrics_utils.KNOWN_PROJECT_URLS:
                        metrics.collector.add('project_urls', [project_url])
                except subprocess2.CalledProcessError:
                    # Occurs when command is not executed in a git repository
                    # We should not fail here. If the command needs to be
                    # executed in a repo, it will be raised later.
                    pass

    @typing.overload
    def _parse_args(self, args: None) -> tuple[optparse.Values, list[str]]:
        ...

    @typing.overload
    def _parse_args(
            self,
            args: Sequence[AnyStr]) -> tuple[optparse.Values, list[AnyStr]]:
        ...

    def _parse_args(self, args: Sequence[AnyStr] | None):
        # Create an optparse.Values object that will store only the actual
        # passed options, without the defaults.
        actual_options = optparse.Values()
        _, argslist = optparse.OptionParser.parse_args(self, args,
                                                       actual_options)
        # Create an optparse.Values object with the default options.
        options = optparse.Values(self.get_default_values().__dict__)
        # Update it with the options passed by the user.
        options._update_careful(actual_options.__dict__)
        # Store the options passed by the user in an _actual_options attribute.
        # We store only the keys, and not the values, since the values can
        # contain arbitrary information, which might be PII.
        metrics.collector.add('arguments', list(actual_options.__dict__.keys()))

        levels = [logging.WARNING, logging.INFO, logging.DEBUG]
        logging.basicConfig(
            level=levels[min(options.verbose,
                             len(levels) - 1)],
            format='[%(levelname).1s%(asctime)s %(process)d %(thread)d '
            '%(filename)s] %(message)s')

        return options, argslist


def main(argv):
    colorize_CMDstatus_doc()
    dispatcher = subcommand.CommandDispatcher(__name__)
    try:
        return dispatcher.execute(OptionParser(), argv)
    except gerrit_util.GerritError as e:
        DieWithError(str(e))
    except auth.LoginRequiredError as e:
        DieWithError(str(e))
    except urllib.error.HTTPError as e:
        if e.code != 500:
            raise
        DieWithError((
            'App Engine is misbehaving and returned HTTP %d, again. Keep faith '
            'and retry or visit go/isgaeup.\n%s') % (e.code, str(e)))
    return 0


if __name__ == '__main__':
    # These affect sys.stdout, so do it outside of main() to simplify mocks in
    # the unit tests.
    setup_color.init()
    with metrics.collector.print_notice_and_exit():
        sys.exit(main(sys.argv[1:]))
