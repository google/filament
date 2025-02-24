# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import os
import scm
import subprocess2
import sys
import urllib.parse

# Current version of metrics recording.
# When we add new metrics, the version number will be increased, we display the
# user what has changed, and ask the user to agree again.
CURRENT_VERSION = 3

APP_URL = 'https://cit-cli-metrics.appspot.com'

REPORT_BUILD = os.getenv('DEPOT_TOOLS_REPORT_BUILD')
COLLECT_METRICS = (os.getenv('DEPOT_TOOLS_COLLECT_METRICS') != '0'
                   and os.getenv('DEPOT_TOOLS_METRICS') != '0')

SYNC_STATUS_SUCCESS = 'SYNC_STATUS_SUCCESS'
SYNC_STATUS_FAILURE = 'SYNC_STATUS_FAILURE'


def get_notice_countdown_header(countdown):
    if countdown == 0:
        yield '     METRICS COLLECTION IS TAKING PLACE'
    else:
        yield '  METRICS COLLECTION WILL START IN %d EXECUTIONS' % countdown


def get_notice_version_change_header():
    yield '       WE ARE COLLECTING ADDITIONAL METRICS'
    yield ''
    yield ' Please review the changes and opt-in again.'


def get_notice_footer():
    yield 'To suppress this message opt in or out using:'
    yield '$ gclient metrics [--opt-in] [--opt-out]'
    yield 'For more information please see metrics.README.md'
    yield 'in your depot_tools checkout or visit'
    yield 'https://bit.ly/3MpLAYM.'


def get_change_notice(version):
    if version == 0:
        return []  # No changes for version 0

    if version == 1:
        return [
            'We want to collect the Git version.',
            'We want to collect information about the HTTP',
            'requests that depot_tools makes, and the git and',
            'cipd commands it executes.',
            '',
            'We only collect known strings to make sure we',
            'don\'t record PII.',
        ]

    if version == 2:
        return [
            'We will start collecting metrics from bots.',
            'There are no changes for developers.',
            'If the DEPOT_TOOLS_REPORT_BUILD environment variable is set,',
            'we will report information about the current build',
            '(e.g. buildbucket project, bucket, builder and build id),',
            'and authenticate to the metrics collection server.',
            'This information will only be recorded for requests',
            'authenticated as bot service accounts.',
        ]

    if version == 3:
        return [
            'We will start collecting metrics for experiment flags.',
            'These are boolean or enum values that show whether',
            'you have opted in to or out of experimental features',
        ]


KNOWN_PROJECT_URLS = {
    'https://chrome-internal.googlesource.com/chrome/ios_internal',
    'https://chrome-internal.googlesource.com/infra/infra_internal',
    'https://chromium.googlesource.com/breakpad/breakpad',
    'https://chromium.googlesource.com/chromium/src',
    'https://chromium.googlesource.com/chromium/tools/depot_tools',
    'https://chromium.googlesource.com/crashpad/crashpad',
    'https://chromium.googlesource.com/external/gyp',
    'https://chromium.googlesource.com/external/naclports',
    'https://chromium.googlesource.com/infra/infra',
    'https://chromium.googlesource.com/native_client/',
    'https://chromium.googlesource.com/syzygy',
    'https://chromium.googlesource.com/v8/v8',
    'https://dart.googlesource.com/sdk',
    'https://pdfium.googlesource.com/pdfium',
    'https://skia.googlesource.com/buildbot',
    'https://skia.googlesource.com/skia',
    'https://webrtc.googlesource.com/src',
}

KNOWN_HTTP_HOSTS = {
    'chrome-internal-review.googlesource.com',
    'chromium-review.googlesource.com',
    'dart-review.googlesource.com',
    'eu1-mirror-chromium-review.googlesource.com',
    'pdfium-review.googlesource.com',
    'skia-review.googlesource.com',
    'us1-mirror-chromium-review.googlesource.com',
    'us2-mirror-chromium-review.googlesource.com',
    'us3-mirror-chromium-review.googlesource.com',
    'webrtc-review.googlesource.com',
}

KNOWN_HTTP_METHODS = {
    'DELETE',
    'GET',
    'PATCH',
    'POST',
    'PUT',
}

KNOWN_HTTP_PATHS = {
    'accounts': re.compile(r'(/a)?/accounts/.*'),
    'changes': re.compile(r'(/a)?/changes/([^/]+)?$'),
    'changes/abandon': re.compile(r'(/a)?/changes/.*/abandon'),
    'changes/comments': re.compile(r'(/a)?/changes/.*/comments'),
    'changes/detail': re.compile(r'(/a)?/changes/.*/detail'),
    'changes/edit': re.compile(r'(/a)?/changes/.*/edit'),
    'changes/message': re.compile(r'(/a)?/changes/.*/message'),
    'changes/restore': re.compile(r'(/a)?/changes/.*/restore'),
    'changes/reviewers': re.compile(r'(/a)?/changes/.*/reviewers/.*'),
    'changes/revisions/commit':
    re.compile(r'(/a)?/changes/.*/revisions/.*/commit'),
    'changes/revisions/review':
    re.compile(r'(/a)?/changes/.*/revisions/.*/review'),
    'changes/submit': re.compile(r'(/a)?/changes/.*/submit'),
    'projects/branches': re.compile(r'(/a)?/projects/.*/branches/.*'),
}

KNOWN_HTTP_ARGS = {
    'ALL_REVISIONS',
    'CURRENT_COMMIT',
    'CURRENT_REVISION',
    'DETAILED_ACCOUNTS',
    'LABELS',
}

GIT_VERSION_RE = re.compile(r'git version (\d)\.(\d{0,2})\.(\d{0,2})')

KNOWN_SUBCOMMAND_ARGS = {
    'cc', 'hashtag', 'l=Auto-Submit+1', 'l=Code-Review+1', 'l=Code-Review+2',
    'l=Commit-Queue+1', 'l=Commit-Queue+2', 'label', 'm', 'notify=ALL',
    'notify=NONE', 'private', 'r', 'ready', 'topic', 'wip'
}


def get_python_version():
    """Return the python version in the major.minor.micro format."""
    return '{v.major}.{v.minor}.{v.micro}'.format(v=sys.version_info)


def get_git_version():
    """Return the Git version in the major.minor.micro format."""
    p = subprocess2.Popen(['git', '--version'],
                          stdout=subprocess2.PIPE,
                          stderr=subprocess2.PIPE)
    stdout, _ = p.communicate()
    match = GIT_VERSION_RE.match(stdout.decode('utf-8'))
    if not match:
        return None
    return '%s.%s.%s' % match.groups()


def get_bot_metrics():
    try:
        project, bucket, builder, build = REPORT_BUILD.split('/')
        return {
            'build_id': int(build),
            'builder': {
                'project': project,
                'bucket': bucket,
                'builder': builder,
            },
        }
    except (AttributeError, ValueError):
        return None


def return_code_from_exception(exception):
    """Returns the exit code that would result of raising the exception."""
    if exception is None:
        return 0
    e = exception[1]
    if isinstance(e, KeyboardInterrupt):
        return 130
    if isinstance(e, SystemExit):
        return e.code
    return 1


def extract_known_subcommand_args(args):
    """Extract the known arguments from the passed list of args."""
    known_args = []
    for arg in args:
        if arg in KNOWN_SUBCOMMAND_ARGS:
            known_args.append(arg)
        else:
            arg = arg.split('=')[0]
            if arg in KNOWN_SUBCOMMAND_ARGS:
                known_args.append(arg)
    return sorted(known_args)


def extract_http_metrics(request_uri, method, status, response_time):
    """Extract metrics from the request URI.

    Extracts the host, path, and arguments from the request URI, and returns
    them along with the method, status and response time.

    The host, method, path and arguments must be in the KNOWN_HTTP_* constants
    defined above.

    Arguments are the values of the o= url parameter. In Gerrit, additional
    fields can be obtained by adding o parameters, each option requires more
    database lookups and slows down the query response time to the client, so
    we make an effort to collect them.

    The regex defined in KNOWN_HTTP_PATH_RES are checked against the path, and
    those that match will be returned.
    """
    http_metrics = {
        'status': status,
        'response_time': response_time,
    }

    if method in KNOWN_HTTP_METHODS:
        http_metrics['method'] = method

    parsed_url = urllib.parse.urlparse(request_uri)

    if parsed_url.netloc in KNOWN_HTTP_HOSTS:
        http_metrics['host'] = parsed_url.netloc

    for name, path_re in KNOWN_HTTP_PATHS.items():
        if path_re.match(parsed_url.path):
            http_metrics['path'] = name
            break

    parsed_query = urllib.parse.parse_qs(parsed_url.query)

    # Collect o-parameters from the request.
    args = [arg for arg in parsed_query.get('o', []) if arg in KNOWN_HTTP_ARGS]
    if args:
        http_metrics['arguments'] = args

    return http_metrics


def get_repo_timestamp(path_to_repo):
    """Get an approximate timestamp for the upstream of |path_to_repo|.

    Returns the top two bits of the timestamp of the HEAD for the upstream of
    the branch path_to_repo is checked out at.
    """
    # Get the upstream for the current branch. If we're not in a branch,
    # fallback to HEAD.
    try:
        upstream = scm.GIT.GetUpstreamBranch(path_to_repo) or 'HEAD'
    except subprocess2.CalledProcessError:
        upstream = 'HEAD'

    # Get the timestamp of the HEAD for the upstream of the current branch.
    p = subprocess2.Popen(
        ['git', '-C', path_to_repo, 'log', '-n1', upstream, '--format=%at'],
        stdout=subprocess2.PIPE,
        stderr=subprocess2.PIPE)
    stdout, _ = p.communicate()

    # If there was an error, give up.
    if p.returncode != 0:
        return None

    return stdout.strip()


def print_boxed_text(out, min_width, lines):
    [EW, NS, SE, SW, NE, NW] = list('=|++++')
    width = max(min_width, max(len(line) for line in lines))
    out(SE + EW * (width + 2) + SW + '\n')
    for line in lines:
        out('%s %-*s %s\n' % (NS, width, line, NS))
    out(NE + EW * (width + 2) + NW + '\n')


def print_notice(countdown):
    """Print a notice to let the user know the status of metrics collection."""
    lines = list(get_notice_countdown_header(countdown))
    lines.append('')
    lines += list(get_notice_footer())
    print_boxed_text(sys.stderr.write, 49, lines)


def print_version_change(config_version):
    """Print a notice to let the user know we are collecting more metrics."""
    lines = list(get_notice_version_change_header())
    for version in range(config_version + 1, CURRENT_VERSION + 1):
        lines.append('')
        lines += get_change_notice(version)
    print_boxed_text(sys.stderr.write, 49, lines)
