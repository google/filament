#! /usr/bin/env python3
"""ExtractIRForPassTest.py - extract IR just before selected pass would be run.

This script automates some operations to make it easier to write IR tests:
  1. Gets the pass list for an HLSL compilation using -Odump
  2. Compiles HLSL with -fcgl and outputs to intermediate IR
  3. Collects list of passes before the desired pass and adds
     -hlsl-passes-pause to write correct metadata
  4. Invokes dxopt to run passes on -fcgl output and write bitcode result
  5. Disassembles bitcode to .ll file for use as a test
  6. Inserts RUN line with -hlsl-passes-resume and desired pass

Examples:
  ExtractIRForPassTest.py -p scalarrepl-param-hlsl -o my_test.ll my_test.hlsl -- -T cs_6_0 -Od
    - stop before 'scalarrepl-param-hlsl' pass; output to my_test.ll
  ExtractIRForPassTest.py -p simplifycfg -n 2 -o my_test.ll my_test.hlsl -- -T cs_6_0
    - stop before the second invocation of 'simplifycfg' pass

Use dxc with -Odump to dump the pass sequence for reference.
"""

import os
import sys
import subprocess
import tempfile
import argparse


def ParseArgs():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter, description=__doc__
    )
    parser.add_argument(
        "-p",
        dest="desired_pass",
        metavar="<desired-pass>",
        required=True,
        help="stop before this module pass (per-function prepass not supported).",
    )
    parser.add_argument(
        "-n",
        dest="invocation",
        metavar="<invocation>",
        type=int,
        default=1,
        help="pass invocation number on which to stop (default=1)",
    )
    parser.add_argument(
        "hlsl_file", metavar="<hlsl-file>", help="input HLSL file path to compile"
    )
    parser.add_argument(
        "-o",
        dest="output_file",
        metavar="<output-file>",
        required=True,
        help="output file name",
    )
    parser.add_argument(
        "compilation_options",
        nargs="*",
        metavar="-- <DXC options>",
        help="set of compilation options needed to compile the HLSL file with DXC",
    )
    return parser.parse_args()


def SplitAtPass(passes, pass_name, invocation=1):
    pass_name = "-" + pass_name
    before = []
    fn_passes = True
    count = 0
    after = None
    for line in passes:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line == "-opt-mod-passes":
            fn_passes = False
        if after:
            after.append(line)
            continue
        if not fn_passes:
            if line == pass_name:
                count += 1
                if count >= invocation:
                    after = [line]
                    continue
        before.append(line)
    if count == 0:
        raise ValueError(
            "Pass '{}' not found in pass list.  Check spelling and that it is a module pass.  Pass list: {}".format(
                pass_name, passes
            )
        )
    elif count < invocation:
        raise ValueError(
            "Pass '{}' found {} times, but {} invocations requested.  Pass list: {}".format(
                pass_name, count, invocation, passes
            )
        )

    return before, after


def GetTempFilename(*args, **kwargs):
    "Get temp filename and close the file handle for use by others"
    fd, name = tempfile.mkstemp(*args, **kwargs)
    os.close(fd)
    return name


def main(args):
    try:
        # 1. Gets the pass list for an HLSL compilation using -Odump
        cmd = ["dxc", "/Odump", args.hlsl_file] + args.compilation_options
        # print(cmd)
        all_passes = subprocess.check_output(cmd, text=True)
        all_passes = all_passes.splitlines()

        # 2. Compiles HLSL with -fcgl and outputs to intermediate IR
        fcgl_file = GetTempFilename(".ll")
        cmd = [
            "dxc",
            "-fcgl",
            "-Fc",
            fcgl_file,
            args.hlsl_file,
        ] + args.compilation_options
        # print(cmd)
        subprocess.check_call(cmd)

        # 3. Collects list of passes before the desired pass and adds
        #    -hlsl-passes-pause to write correct metadata
        passes_before, passes_after = SplitAtPass(
            all_passes, args.desired_pass, args.invocation
        )
        print(
            "\nPasses before: {}\n\nRemaining passes: {}".format(
                " ".join(passes_before), " ".join(passes_after)
            )
        )
        passes_before.append("-hlsl-passes-pause")

        # 4. Invokes dxopt to run passes on -fcgl output and write bitcode result
        bitcode_file = GetTempFilename(".bc")
        cmd = ["dxopt", "-o=" + bitcode_file, fcgl_file] + passes_before
        # print(cmd)
        subprocess.check_call(cmd)

        # 5. Disassembles bitcode to .ll file for use as a test
        temp_out = GetTempFilename(".ll")
        cmd = ["dxc", "/dumpbin", "-Fc", temp_out, bitcode_file]
        # print(cmd)
        subprocess.check_call(cmd)

        # 6. Inserts RUN line with -hlsl-passes-resume and desired pass
        with open(args.output_file, "wt") as f:
            f.write(
                "; RUN: %dxopt %s -hlsl-passes-resume -{} -S | FileCheck %s\n\n".format(
                    args.desired_pass
                )
            )
            with open(temp_out, "rt") as f_in:
                f.write(f_in.read())

        # Clean up temp files
        os.unlink(fcgl_file)
        os.unlink(bitcode_file)
        os.unlink(temp_out)
    except:
        print(f"\nSomething went wrong!\nMost recent command and arguments: {cmd}\n")
        raise


if __name__ == "__main__":
    args = ParseArgs()
    main(args)
    print("\nSuccess!  See output file:\n{}".format(args.output_file))
