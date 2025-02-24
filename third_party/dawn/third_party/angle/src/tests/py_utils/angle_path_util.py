# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import pathlib
import posixpath
import sys

ANGLE_ROOT_DIR = str(pathlib.Path(__file__).resolve().parents[3])


def AddDepsDirToPath(posixpath_from_root):
    relative_path = os.path.join(*posixpath.split(posixpath_from_root))
    full_path = os.path.join(ANGLE_ROOT_DIR, relative_path)
    if not os.path.exists(full_path):
        # Assume Chromium checkout
        chromium_root_dir = os.path.abspath(os.path.join(ANGLE_ROOT_DIR, '..', '..'))
        full_path = os.path.join(chromium_root_dir, relative_path)
        assert os.path.exists(full_path)

    if full_path not in sys.path:
        sys.path.insert(0, full_path)
