# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'osx_sdk',
    'recipe_engine/platform',
    'recipe_engine/properties',
    'recipe_engine/raw_io',
    'recipe_engine/step',
]


def RunSteps(api):
  with api.osx_sdk('mac'):
    api.step('gn', ['gn', 'gen', 'out/Release'])
    api.step('ninja', ['ninja', '-C', 'out/Release'])


def GenTests(api):
  for platform in ('linux', 'mac', 'win'):
    yield (api.test(platform) +
           api.platform.name(platform))

  yield api.test(
      'explicit_version',
      api.platform.name('mac'),
      api.osx_sdk.pick_sdk_version('deadbeef'),
  )

  yield api.test(
      'automatic_version',
      api.platform.name('mac'),
      api.osx_sdk.macos_version('10.15.6'),
  )

  yield api.test(
      'ancient_version',
      api.platform.name('mac'),
      api.osx_sdk.macos_version('10.1'),
  )

  bad_versions = [
      'meep.morp',
      '1.2.3.4',
      '10',
  ]
  for version in bad_versions:
    try:
      api.osx_sdk.macos_version(version)
      assert False, f'macos_version regex failure, allowed {version=}'  # pragma: no cover
    except ValueError:
      pass
