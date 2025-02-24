# -*- coding: utf-8 -*-
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DEPS = [
  'git_cl',
  'recipe_engine/path',
  'recipe_engine/raw_io',
  'recipe_engine/step',
]


def RunSteps(api):
  api.git_cl.upload("Do the thing foobar\nNow with emoji: 😄")
  api.git_cl.issue()
  result = api.git_cl.get_description(
      patch_url='https://code.review/123',
      suffix='build')
  api.git_cl.set_description(
      'bammmm', patch_url='https://code.review/123')
  api.step('echo', ['echo', result.stdout.decode('utf-8')])

  api.git_cl.set_default_repo_location(api.path.mkdtemp('fakerepo'))

  api.step(
      'echo', ['echo', api.git_cl.get_description().stdout.decode('utf-8')])

  api.git_cl.set_description('new description woo')

  api.step(
      'echo', ['echo', api.git_cl.get_description().stdout.decode('utf-8')])

def GenTests(api):
  yield (
      api.test('basic') +
      api.override_step_data(
          'git_cl description (build)', stdout=api.raw_io.output('hi')) +
      api.override_step_data(
          'git_cl description', stdout=api.raw_io.output('hey')) +
      api.override_step_data(
          'git_cl description (2)', stdout=api.raw_io.output(
              'new description woo'))
  )
