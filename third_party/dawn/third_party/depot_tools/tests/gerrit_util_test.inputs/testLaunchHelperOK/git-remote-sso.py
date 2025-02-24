#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import sys
import tempfile

from pathlib import Path

THIS_DIR = Path(__file__).parent.absolute()

with tempfile.TemporaryDirectory() as tempdir:
    tempdir = Path(tempdir)

    target_config = tempdir / "gitconfig"
    target_cookies = tempdir / "cookiefile.txt"

    shutil.copyfile(THIS_DIR / "gitconfig", target_config)
    shutil.copyfile(THIS_DIR / "cookiefile.txt", target_cookies)

    print('http.proxy=localhost:12345')
    print(f'include.path={target_config}')
    print(f'http.cookiefile={target_cookies}')
    sys.stdout.flush()
    # need to fully close file descriptor, sys.stdout.close() doesn't seem to cut
    # it.
    os.close(1)

    print("OK", file=sys.stderr)

    # block until stdin closes, then clean everything via TemporaryDirectory().
    #
    # This emulates the behavior of the real git-remote-sso helper which just
    # prints temporary configuration for a daemon running elsewhere.
    sys.stdin.read()
