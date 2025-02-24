# Copyright 2018 The LUCI Authors. All rights reserved.
# Use of this source code is governed under the Apache License, Version 2.0
# that can be found in the LICENSE file.

from recipe_engine.post_process import DropExpectation, StatusSuccess

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'gitiles',
  'recipe_engine/properties',
  'recipe_engine/step',
]


def RunSteps(api):
  valid_urls = [
    'https://host/path/to/project',
    'http://host/path/to/project',
    'https://host/a/path/to/project',
    'https://host/path/to/project.git',
    'http://host/a/path/to/project',
    'host/a/path/to/project',
  ]
  for repo_url in valid_urls:
    host, project = api.gitiles.parse_repo_url(repo_url)
    assert host == 'host', host
    assert project == 'path/to/project', project

  invalid_urls = [
    'https://host/a/path/to/project?a=b',
    'https://host/path/to/project/+/main',
  ]
  for repo_url in invalid_urls:
    host, project = api.gitiles.parse_repo_url(repo_url)
    assert host is None
    assert project is None

  actual = api.gitiles.unparse_repo_url('host', 'path/to/project')
  expected = 'https://host/path/to/project'
  assert actual == expected

  actual = api.gitiles.canonicalize_repo_url('http://host/path/to/project')
  expected = 'https://host/path/to/project'
  assert actual == expected

  actual = api.gitiles.canonicalize_repo_url('http://unrecognized')
  expected = 'http://unrecognized'
  assert actual == expected


def GenTests(api):
  yield api.test(
      'basic',
      api.post_process(StatusSuccess),
      api.post_process(DropExpectation),
  )
