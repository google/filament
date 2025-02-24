# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gitiles',
    'recipe_engine/json',
    'recipe_engine/step',
    'recipe_engine/path',
    'recipe_engine/properties',
]


def RunSteps(api):
  url = 'https://chromium.googlesource.com/chromium/src'
  for ref in api.gitiles.refs(url):
    _, cursor = api.gitiles.log(url, ref)
    if cursor:
      api.gitiles.log(url, ref, limit=10, cursor=cursor)
  api.gitiles.commit_log(url, api.properties['commit_log_hash'])

  data = api.gitiles.download_file(url, 'OWNERS', attempts=5)
  assert data == 'foobar'

  data = api.gitiles.download_file(url, 'BYTES', attempts=5)
  assert data == b'\xab'

  data = api.gitiles.download_file(url, 'NONEXISTENT', attempts=1,
                                   accept_statuses=[404])

  api.gitiles.download_archive(url, api.path.start_dir / 'archive')

  try:
    api.gitiles.download_archive(url, api.path.start_dir / 'archive2')
    assert False  # pragma: no cover
  except api.step.StepFailure as ex:
    assert '/root' in ex.gitiles_skipped_files


def GenTests(api):
  yield (
      api.test('basic')
      + api.properties(
          commit_log_hash=api.gitiles.make_hash('commit'),
      )
      + api.step_data('refs', api.gitiles.make_refs_test_data(
          'HEAD',
          'refs/heads/A',
          'refs/tags/B',
      ))
      + api.step_data(
          'gitiles log: HEAD',
          api.gitiles.make_log_test_data('HEAD', cursor='deadbeaf'),
      )
      + api.step_data(
          'gitiles log: HEAD from deadbeaf',
          api.gitiles.make_log_test_data('HEAD'),
      )
      + api.step_data(
          'gitiles log: refs/heads/A',
          api.gitiles.make_log_test_data('A'),
      )
      + api.step_data(
          'gitiles log: refs/tags/B',
          api.gitiles.make_log_test_data('B')
      )
      + api.step_data(
          'commit log: %s' % (api.gitiles.make_hash('commit')),
          api.gitiles.make_commit_test_data('commit', 'C', new_files=[
              'foo/bar',
              'baz/qux',
          ])
      )
      + api.step_data(
          'fetch main:OWNERS',
          api.gitiles.make_encoded_file('foobar')
      )
      + api.step_data(
          'fetch main:BYTES',
          api.gitiles.make_encoded_file_from_bytes(b'\xab')
      )
      + api.step_data(
          'fetch main:NONEXISTENT',
          api.json.output({'value': None})
      )
      + api.step_data(
          ('download https://chromium.googlesource.com/chromium/src @ '
           'refs/heads/main (2)'),
        api.json.output({
          'extracted': {
            'filecount': 10,
            'bytes': 14925,
          },
          'skipped': {
            'filecount': 4,
            'bytes': 7192345,
            'names': ['/root', '../relative', 'sneaky/../../relative'],
          },
        })
      )
  )
