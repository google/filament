#!/usr/bin/python3

# Copyright 2015 Google Inc.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generate .gclient file for Angle.

Because gclient won't accept "--name ." use a different name then edit.
"""

import subprocess
import sys


def main():
    gclient_cmd = ('gclient config --name change2dot --unmanaged '
                   'https://chromium.googlesource.com/angle/angle.git')
    try:
        rc = subprocess.call(gclient_cmd, shell=True)
    except OSError:
        print('could not run "%s" via shell' % gclient_cmd)
        sys.exit(1)

    if rc:
        print('failed command: "%s"' % gclient_cmd)
        sys.exit(1)

    with open('.gclient') as gclient_file:
        content = gclient_file.read()

    content = content.replace('change2dot', '.')
    if sys.platform.startswith('linux'):
        content += 'target_os = [ \'android\' ]\n'

    with open('.gclient', 'w') as gclient_file:
        gclient_file.write(content)

    print('created .gclient')


if __name__ == '__main__':
    main()
