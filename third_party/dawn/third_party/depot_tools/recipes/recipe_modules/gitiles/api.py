# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import base64
import urllib.parse

from recipe_engine import recipe_api


class Gitiles(recipe_api.RecipeApi):
  """Module for polling a git repository using the Gitiles web interface."""

  def _fetch(self, url, step_name, fmt, attempts=None, add_json_log=True,
             log_limit=None, log_start=None, extract_to=None, **kwargs):
    """Fetches information from Gitiles.

    Args:
      * fmt (str): one of ('text', 'json', 'archive'). Instructs the underlying
        gitiles_client tool how to process the HTTP response.
          * text - implies the response is base64 encoded
          * json - implies the response is JSON
          * archive - implies the response is a compressed tarball; requires
            `extract_to`.
      * extract_to (Path): When fmt=='archive', instructs gitiles_client to
        extract the archive to this non-existent folder.
      * log_limit: for log URLs, limit number of results. None implies 1 page,
        as returned by Gitiles.
      * log_start: for log URLs, the start cursor for paging.
      * add_json_log: if True, will spill out json into log.
    """
    assert fmt in ('json', 'text', 'archive')

    args = [
        '--json-file', self.m.json.output(add_json_log=add_json_log),
        '--url', url,
        '--format', fmt,
    ]
    if fmt == 'archive':
      assert extract_to is not None, 'archive format requires extract_to'
      args.extend(['--extract-to', extract_to])
    if attempts:
      args.extend(['--attempts', attempts])
    if log_limit is not None:
      args.extend(['--log-limit', log_limit])
    if log_start is not None:
      args.extend(['--log-start', log_start])
    accept_statuses = kwargs.pop('accept_statuses', None)
    if accept_statuses:
      args.extend([
          '--accept-statuses',
          ','.join([str(s) for s in accept_statuses])])
    cmd = ['vpython3', '-u', self.resource('gerrit_client.py')] + args
    return self.m.step(step_name, cmd, **kwargs)

  def refs(self, url, step_name='refs', attempts=None):
    """Returns a list of refs in the remote repository."""
    step_result = self._fetch(
        self.m.url.join(url, '+refs'),
        step_name,
        fmt='json',
        attempts=attempts)

    refs = sorted(str(ref) for ref in step_result.json.output)
    step_result.presentation.logs['refs'] = refs
    return refs

  def log(self, url, ref, limit=0, cursor=None,
          step_name=None, attempts=None, **kwargs):
    """Returns the most recent commits under the given ref with properties.

    Args:
      * url (str): URL of the remote repository.
      * ref (str): Name of the desired ref (see Gitiles.refs).
      * limit (int): Number of commits to limit the fetching to.
        Gitiles does not return all commits in one call; instead paging is
        used. 0 implies to return whatever first gerrit responds with.
        Otherwise, paging will be used to fetch at least this many
        commits, but all fetched commits will be returned.
      * cursor (str or None): The paging cursor used to fetch the next page.
      * step_name (str): Custom name for this step (optional).

    Returns:
      A tuple of (commits, cursor).
      Commits are a list of commits (as Gitiles dict structure) in reverse
      chronological order. The number of commits may be higher than limit
      argument.
      Cursor can be used for subsequent calls to log for paging. If None,
      signals that there are no more commits to fetch.
    """
    assert limit >= 0
    step_name = step_name or 'gitiles log: %s%s' % (
        ref, ' from %s' % cursor if cursor else '')

    step_result = self._fetch(
        self.m.url.join(url, '+log/%s' % ref),
        step_name,
        log_limit=limit,
        log_start=cursor,
        attempts=attempts,
        fmt='json',
        add_json_log=True,
        **kwargs)

    # The output is formatted as a JSON dict with a "log" key. The "log" key
    # is a list of commit dicts, which contain information about the commit.
    commits = step_result.json.output['log']
    cursor = step_result.json.output.get('next')

    step_result.presentation.step_text = (
        '<br />%d commits fetched' % len(commits))
    return commits, cursor

  def commit_log(self, url, commit, step_name=None, attempts=None):
    """Returns: (dict) the Gitiles commit log structure for a given commit.

    Args:
      * url (str): The base repository URL.
      * commit (str): The commit hash.
      * step_name (str): If not None, override the step name.
      * attempts (int): Number of times to try the request before failing.
    """
    step_name = step_name or 'commit log: %s' % commit

    commit_url = '%s/+/%s' % (url, commit)
    step_result = self._fetch(commit_url, step_name, attempts=attempts,
                              fmt='json')
    return step_result.json.output

  def download_file(self, repository_url, file_path, branch='main',
                    step_name=None, attempts=None, **kwargs):
    """Downloads raw file content from a Gitiles repository.

    Args:
      * repository_url (str): Full URL to the repository.
      * branch (str): Branch of the repository.
      * file_path (str): Relative path to the file from the repository root.
      * step_name (str): Custom name for this step (optional).
      * attempts (int): Number of times to try the request before failing.

    Returns:
      Raw file content.
    """
    fetch_url = self.m.url.join(repository_url, '+/%s/%s' % (branch, file_path))
    step_result = self._fetch(
        fetch_url,
        step_name or 'fetch %s:%s' % (branch, file_path,),
        attempts=attempts,
        fmt='text',
        add_json_log=False,
        **kwargs)
    if step_result.json.output['value'] is None:
      return None

    value = base64.b64decode(step_result.json.output['value'])
    try:
      # If the file is not utf-8 encodable, return the bytes
      value = value.decode('utf-8')
    finally:
      return value

  def download_archive(self, repository_url, destination,
                       revision='refs/heads/main'):
    """Downloads an archive of the repo and extracts it to `destination`.

    If the gitiles server attempts to provide a tarball with paths which escape
    `destination`, this function will extract all valid files and then
    raise StepFailure with an attribute `StepFailure.gitiles_skipped_files`
    containing the names of the files that were skipped.

    Args:
      * repository_url (str): Full URL to the repository
      * destination (Path): Local path to extract the archive to. Must not exist
        prior to this call.
      * revision (str): The ref or revision in the repo to download. Defaults to
        'refs/heads/main'.
    """
    step_name = 'download %s @ %s' % (repository_url, revision)
    fetch_url = self.m.url.join(repository_url, '+archive/%s.tgz' % (revision,))
    step_result = self._fetch(
      fetch_url,
      step_name,
      fmt='archive',
      add_json_log=False,
      extract_to=destination,
      step_test_data=lambda: self.m.json.test_api.output({
        'extracted': {
          'filecount': 1337,
          'bytes': 7192345,
        },
      })
    )
    self.m.path.mock_add_paths(destination)
    j = step_result.json.output
    if j['extracted']['filecount']:
      stat = j['extracted']
      step_result.presentation.step_text += (
        '<br/>extracted %s files - %.02f MB' % (
          stat['filecount'], stat['bytes'] / (1000.0**2)))
    if j.get('skipped', {}).get('filecount'):
      stat = j['skipped']
      step_result.presentation.step_text += (
        '<br/>SKIPPED %s files - %.02f MB' % (
          stat['filecount'], stat['bytes'] / (1000.0**2)))
      step_result.presentation.logs['skipped files'] = stat['names']
      step_result.presentation.status = self.m.step.FAILURE
      ex = self.m.step.StepFailure(step_name)
      ex.gitiles_skipped_files = stat['names']
      raise ex

  def parse_repo_url(self, repo_url):
    """Returns (host, project) pair.

    Returns (None, None) if repo_url is not recognized.
    """
    return parse_repo_url(repo_url)

  def unparse_repo_url(self, host, project):
    """Generates a Gitiles repo URL. See also parse_repo_url."""
    return unparse_repo_url(host, project)

  def canonicalize_repo_url(self, repo_url):
    """Returns a canonical form of repo_url. If not recognized, returns as is.
    """
    if repo_url:
      host, project = parse_repo_url(repo_url)
      if host and project:
        repo_url = unparse_repo_url(host, project)
    return repo_url


def parse_http_host_and_path(url):
  # Copied from https://chromium.googlesource.com/infra/luci/recipes-py/+/809e57935211b3fcb802f74a7844d4f36eff6b87/recipe_modules/buildbucket/util.py
  parsed = urllib.parse.urlparse(url)
  if not parsed.scheme:
    parsed = urllib.parse.urlparse('https://' + url)
  if (parsed.scheme in ('http', 'https') and
      not parsed.params and
      not parsed.query and
      not parsed.fragment):
    return parsed.netloc, parsed.path
  return None, None


def parse_repo_url(repo_url):
  """Returns (host, project) pair.

  Returns (None, None) if repo_url is not recognized.
  """
  # Adapted from https://chromium.googlesource.com/infra/luci/recipes-py/+/809e57935211b3fcb802f74a7844d4f36eff6b87/recipe_modules/buildbucket/util.py
  host, project = parse_http_host_and_path(repo_url)
  if not host or not project or '+' in project.split('/'):
    return None, None
  project = project.strip('/')
  if project.startswith('a/'):
    project = project[len('a/'):]
  if project.endswith('.git'):
    project = project[:-len('.git')]
  return host, project


def unparse_repo_url(host, project):
  return 'https://%s/%s' % (host, project)
