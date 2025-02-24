#!/usr/bin/env vpython3

# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Runs simulation tests and lint on the recipes."""

import os
import subprocess

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def recipes_py(*args):
    recipes_cfg_path = os.path.join(ROOT_DIR, 'infra', 'config', 'recipes.cfg')
    subprocess.check_call([os.path.join(ROOT_DIR, 'recipes', 'recipes.py'),
                           '--package', recipes_cfg_path] + list(args))


recipes_py('test', 'run')

recipes_py('lint')
