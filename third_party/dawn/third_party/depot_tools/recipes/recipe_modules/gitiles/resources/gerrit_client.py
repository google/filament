#!/usr/bin/env vpython3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Simple client for the Gerrit REST API.

Example usage:
  ./gerrit_client.py -j /tmp/out.json -f json \
    -u https://chromium.googlesource.com/chromium/src/+log
"""

import argparse
import json
import logging
import os
import sys
import tarfile
import time
import urllib.parse

DEPOT_TOOLS = os.path.abspath(
    os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, os.pardir,
                 os.pardir))
sys.path.insert(0, DEPOT_TOOLS)

from gerrit_util import CreateHttpConn, ReadHttpResponse, ReadHttpJsonResponse


def reparse_url(parsed_url, query_params):
  return urllib.parse.ParseResult(
      scheme=parsed_url.scheme,
      netloc=parsed_url.netloc,
      path=parsed_url.path,
      params=parsed_url.params,
      fragment=parsed_url.fragment,
      query=urllib.parse.urlencode(query_params, doseq=True))


def gitiles_get(parsed_url, handler, attempts):
  # This insanity is due to CreateHttpConn interface :(
  host = parsed_url.netloc
  path = parsed_url.path
  if parsed_url.query:
    path += '?%s' % (parsed_url.query, )

  retry_delay_seconds = 1
  attempt = 1
  while True:
    try:
      return handler(CreateHttpConn(host, path))
    except Exception as e:
      if attempt >= attempts:
        raise
      logging.exception('Failed to perform Gitiles operation: %s', e)

    # Retry from previous loop.
    logging.error('Sleeping %d seconds before retry (%d/%d)...',
                  retry_delay_seconds, attempt, attempts)
    time.sleep(retry_delay_seconds)
    retry_delay_seconds *= 2
    attempt += 1


def fetch_log_with_paging(query_params, limit, fetch):
  """Fetches log, possibly requesting multiple pages to do so.

  Args:
    query_params (dict): Parameters to use in the request.
    limit (int): Page size.
    fetch (function): Function to use to make the requests.

  Returns:
    Dict with key "log", whose value is a list of commits.
  """
  # Log api returns {'log': [list of commits], 'next': hash}.
  last_result = fetch(query_params)
  commits = last_result['log']
  while last_result.get('next') and len(commits) < limit:
    query_params['s'] = last_result.get('next')
    last_result = fetch(query_params)
    # The first commit in `last_result` is not necessarily the parent of the
    # last commit in result so far!  This is because log command can be done on
    # one file object, for example:
    #   https://gerrit.googlesource.com/gitiles/+log/1c21279f337da8130/COPYING
    # Even when getting log for the whole repository, there could be merge
    # commits.
    commits.extend(last_result['log'])
  # Use 'next' field (if any) from `last_result`, but commits aggregated
  # from all the results. This essentially imitates paging with at least
  # `limit` page size.
  last_result['log'] = commits
  logging.debug(
      'fetched %d commits, next: %s.', len(commits),
      last_result.get('next'))
  return last_result


def main(arguments):
  parser = create_argparser()
  args = parser.parse_args(arguments)

  if args.extract_to and args.format != "archive":
    parser.error('--extract-to requires --format=archive')
  if not args.extract_to and args.format == "archive":
    parser.error('--format=archive requires --extract-to')

  if args.extract_to:
    # make sure it is absolute and ends with '/'
    args.extract_to = os.path.join(os.path.abspath(args.extract_to), '')
    os.makedirs(args.extract_to)

  parsed_url = urllib.parse.urlparse(args.url)
  if not parsed_url.scheme.startswith('http'):
    parser.error('Invalid URI scheme (expected http or https): %s' % args.url)

  query_params = {}
  if parsed_url.query:
    query_params.update(urllib.parse.parse_qs(parsed_url.query))
  # Force the format specified on command-line.
  if query_params.get('format'):
    parser.error('URL must not contain format; use --format command line flag '
                 'instead.')
  query_params['format'] = args.format

  kwargs = {}
  accept_statuses = frozenset([int(s) for s in args.accept_statuses.split(',')])
  if accept_statuses:
    kwargs['accept_statuses'] = accept_statuses

  # Choose handler.
  if args.format == 'json':
    def handler(conn):
      return ReadHttpJsonResponse(conn, **kwargs)
  elif args.format == 'text':
    # Text fetching will pack the text into structured JSON.
    def handler(conn):
      # Wrap in a structured JSON for export to recipe module.
      return {
        'value': ReadHttpResponse(conn, **kwargs).read() or None,
      }
  elif args.format == 'archive':
    # Archive fetching hooks result to tarfile extraction. This implementation
    # is able to do a streaming extraction operation without having to buffer
    # the entire tarfile.
    def handler(conn):
      ret = {
        'extracted': {
          'filecount': 0,
          'bytes': 0,
        },
        'skipped': {
          'filecount': 0,
          'bytes': 0,
          'names': [],
        }
      }
      fileobj = ReadHttpResponse(conn, **kwargs)
      with tarfile.open(mode='r|*', fileobj=fileobj) as tf:
        # monkeypatch the TarFile object to allow printing messages and
        # collecting stats for each extracted file. extractall makes a single
        # linear pass over the tarfile, which is compatible with
        # ReadHttpResponse; other naive implementations (such as `getmembers`)
        # do random access over the file and would require buffering the whole
        # thing (!!).
        em = tf._extract_member
        def _extract_member(tarinfo, targetpath):
          if not os.path.abspath(targetpath).startswith(args.extract_to):
            print('Skipping %s' % (tarinfo.name,))
            ret['skipped']['filecount'] += 1
            ret['skipped']['bytes'] += tarinfo.size
            ret['skipped']['names'].append(tarinfo.name)
            return
          print('Extracting %s' % (tarinfo.name,))
          ret['extracted']['filecount'] += 1
          ret['extracted']['bytes'] += tarinfo.size
          return em(tarinfo, targetpath)
        tf._extract_member = _extract_member
        tf.extractall(args.extract_to)
      return ret

  if args.log_start:
    query_params['s'] = args.log_start

  def fetch(query_params):
    parsed_url_with_query = reparse_url(parsed_url, query_params)
    result = gitiles_get(parsed_url_with_query, handler, args.attempts)
    if not args.quiet:
      logging.info('Read from %s: %s', parsed_url_with_query.geturl(), result)
    return result

  if args.log_limit:
    if args.format != 'json':
      parser.error('--log-limit works with json format only')
    result = fetch_log_with_paging(query_params, args.log_limit, fetch)
  else:
    # Either not a log request, or don't care about paging.
    # So, just return whatever is fetched the first time.
    result = fetch(query_params)

  with open(args.json_file, 'w') as json_file:
    json.dump(result, json_file)
  return 0


def create_argparser():
  parser = argparse.ArgumentParser()
  parser.add_argument(
      '-j', '--json-file',
      help='Path to json file for output.')
  parser.add_argument(
      '--extract-to',
      help='Local path to extract archive url. Must not exist.')
  parser.add_argument(
      '-f', '--format', required=True, choices=('json', 'text', 'archive'))
  parser.add_argument(
      '-u', '--url', required=True,
      help='Url of gitiles. For example, '
           'https://chromium.googlesource.com/chromium/src/+refs. '
           'Insert a/ after domain for authenticated access.')
  parser.add_argument(
      '-a', '--attempts', type=int, default=1,
      help='The number of attempts to make (with exponential backoff) before '
           'failing. If several requests are to be made, applies per each '
           'request separately.')
  parser.add_argument(
      '-q', '--quiet', action='store_true',
      help='Suppress file contents logging output.')
  parser.add_argument(
      '--log-limit', type=int, default=None,
      help='Follow gitiles pages to fetch at least this many commits. By '
           'default, first page with unspecified number of commits is fetched. '
           'Only for https://<hostname>/<repo>/+log/... gitiles request.')
  parser.add_argument(
      '--log-start',
      help='If given, continue fetching log by paging from this commit hash. '
           'This value can be typically be taken from json result of previous '
           'call to log, which returns next page start commit as "next" key. '
           'Only for https://<hostname>/<repo>/+log/... gitiles request.')
  parser.add_argument(
      '--accept-statuses', type=str, default='200',
      help='Comma-separated list of Status codes to accept as "successful" '
           'HTTP responses.')
  return parser


if __name__ == '__main__':
  logging.basicConfig()
  logging.getLogger().setLevel(logging.INFO)
  sys.exit(main(sys.argv[1:]))
