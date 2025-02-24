# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

from PB.go.chromium.org.luci.swarming.proto.api_v2 import (
    swarming as swarming_pb)
from PB.recipe_modules.recipe_engine.led import properties as led_properties_pb

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'tryserver',
    'recipe_engine/buildbucket',
    'recipe_engine/properties',
]


def RunSteps(api):
  api.tryserver.require_is_tryserver()


def GenTests(api):
  yield api.test(
      'tryjob',
      api.buildbucket.try_build(),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'not-a-tryjob',
      api.post_check(post_process.StatusException),
      api.post_check(post_process.StepException, 'not a tryjob'),
      api.post_process(post_process.DropExpectation),
      status="INFRA_FAILURE",
  )

  yield api.test(
      'not-a-tryjob-led',
      api.properties(
          **{
              '$recipe_engine/led':
              led_properties_pb.InputProperties(
                  led_run_id='fake-run-id',
                  rbe_cas_input=swarming_pb.CASReference(
                      cas_instance=(
                          'projects/example/instances/default_instance'),
                      digest=swarming_pb.Digest(
                          hash='examplehash',
                          size_bytes=71,
                      ),
                  ),
              ),
          }),
      api.post_check(post_process.StatusFailure),
      api.post_check(post_process.StepFailure, 'not a tryjob'),
      api.post_process(post_process.DropExpectation),
      status="FAILURE")
