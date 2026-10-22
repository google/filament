#!/usr/bin/env python3
# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import sys

def main(argv):
    parser = argparse.ArgumentParser(description='Update Loader.rc for official builds')
    parser.add_argument('src')
    parser.add_argument('dst')
    parser.add_argument('--is_official', action='store_true')
    parser.set_defaults(is_official=False)
    args = parser.parse_args(argv)
    with open(args.src, 'r') as src:
        with open(args.dst, 'w') as dst:
            for line in src:
                if args.is_official:
                    if line.startswith('#define VER_FILE_DESCRIPTION_STR'):
                        dst.write(line.replace('Dev Build', '0'))
                    elif line.startswith('#define VER_FILE_VERSION_STR'):
                        dst.write(line.replace(' - Dev Build', ''))
                    else:
                        dst.write(line)
                else:
                    dst.write(line)
if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))