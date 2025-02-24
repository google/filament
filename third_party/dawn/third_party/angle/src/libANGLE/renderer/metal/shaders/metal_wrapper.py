#!/usr/bin/python3
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os
import subprocess
import sys


def main(args):
    return subprocess.run(args, stdout=subprocess.PIPE, text=True).returncode


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
