# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details on the presubmit API built into depot_tools.
"""

import os

USE_PYTHON3 = True


def _CommonChecks(input_api, output_api):
    d = os.path.dirname
    angle_root = d(d(input_api.PresubmitLocalPath()))
    gen_script = os.path.join(angle_root, 'testing', 'buildbot', 'generate_buildbot_json.py')

    # Validate the format of the mb_config.pyl file.
    mb_path = os.path.join(angle_root, 'tools', 'mb', 'mb.py')
    config_path = os.path.join(input_api.PresubmitLocalPath(), 'angle_mb_config.pyl')

    commands = [
        input_api.Command(
            name='generate_buildbot_json',
            cmd=[
                input_api.python3_executable, gen_script, '--check', '--verbose',
                '--pyl-files-dir',
                input_api.PresubmitLocalPath()
            ],
            kwargs={},
            message=output_api.PresubmitError),
        input_api.Command(
            name='mb_validate',
            cmd=[
                input_api.python3_executable,
                mb_path,
                'validate',
                '-f',
                config_path,
            ],
            kwargs={'cwd': input_api.PresubmitLocalPath()},
            message=output_api.PresubmitError),
    ]
    messages = []

    messages.extend(input_api.RunTests(commands))
    return messages


def CheckChangeOnUpload(input_api, output_api):
    return _CommonChecks(input_api, output_api)


def CheckChangeOnCommit(input_api, output_api):
    return _CommonChecks(input_api, output_api)
