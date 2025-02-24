#!/usr/bin/env python
#  Copyright 2018 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate library file with compressed symbols per Android build
# process.
# https://www.ece.villanova.edu/VECR/doc/gdb/MiniDebugInfo.html

import argparse
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '--objcopy', required=True, help='The objcopy binary to run', metavar='PATH')
    parser.add_argument('--nm', required=True, help='The nm binary to run', metavar='PATH')
    parser.add_argument('--strip', required=True, help='The strip binary to run', metavar='PATH')
    parser.add_argument(
        '--output', required=True, help='Final output shared object file', metavar='FILE')
    parser.add_argument(
        '--unstrippedsofile',
        required=True,
        help='Unstripped shared object file produced by linking command',
        metavar='FILE')
    args = parser.parse_args()

    copy_cmd = ["cp", args.unstrippedsofile, args.output]
    result = subprocess.call(copy_cmd)

    nm_cmd = subprocess.Popen([args.nm, '-D', args.output, '--format=posix', '--defined-only'],
                              stdout=subprocess.PIPE)

    awk_cmd = subprocess.Popen(['awk', '{ print $1}'], stdin=nm_cmd.stdout, stdout=subprocess.PIPE)

    dynsym_out = open(args.output + '.dynsyms', 'w')
    sort_cmd = subprocess.Popen(['sort'], stdin=awk_cmd.stdout, stdout=dynsym_out)
    sort_cmd.wait()
    dynsym_out.close()

    funcsyms_out = open(args.output + '.funcsyms', 'w')
    nm_cmd = subprocess.Popen([args.nm, args.output, '--format=posix', '--defined-only'],
                              stdout=subprocess.PIPE)

    awk_cmd = subprocess.Popen(['awk', '{ if ($2 == "T" || $2 == "t" || $2 == "D") print $1 }'],
                               stdin=nm_cmd.stdout,
                               stdout=subprocess.PIPE)

    sort_cmd = subprocess.Popen(['sort'], stdin=awk_cmd.stdout, stdout=funcsyms_out)
    sort_cmd.wait()
    funcsyms_out.close()

    keep_symbols = open(args.output + '.keep_symbols', 'w')
    comm_cmd = subprocess.Popen(
        ['comm', '-13', args.output + '.dynsyms', args.output + '.funcsyms'], stdout=keep_symbols)
    comm_cmd.wait()

    # Ensure that the keep_symbols file is not empty.
    keep_symbols.write("\n")
    keep_symbols.close()

    objcopy_cmd = [args.objcopy, '--only-keep-debug', args.output, args.output + '.debug']
    subprocess.check_call(objcopy_cmd)

    objcopy_cmd = [
        args.objcopy, '-S', '--remove-section', '.gdb_index', '--remove-section', '.comment',
        '--keep-symbols', args.output + '.keep_symbols', args.output + '.debug',
        args.output + '.mini_debuginfo'
    ]
    subprocess.check_call(objcopy_cmd)

    strip_cmd = [args.strip, '--strip-all', '-R', '.comment', args.output]
    subprocess.check_call(strip_cmd)

    xz_cmd = ['xz', '-f', args.output + '.mini_debuginfo']
    subprocess.check_call(xz_cmd)

    objcopy_cmd = [
        args.objcopy, '--add-section', '.gnu_debugdata=' + args.output + '.mini_debuginfo.xz',
        args.output
    ]
    subprocess.check_call(objcopy_cmd)

    # Clean out scratch files
    rm_cmd = [
        'rm', '-f', args.output + '.dynsyms', args.output + '.funcsyms',
        args.output + '.keep_symbols', args.output + '.debug', args.output + '.mini_debuginfo',
        args.output + '.mini_debuginfo.xz'
    ]
    result = subprocess.call(rm_cmd)

    return result


if __name__ == "__main__":
    sys.exit(main())
