#!/usr/bin/env python3

# Copyright 2025 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
""" Script for generating the various Tint fuzzer corpora.

The basic functionality is to collect all .wgsl files under a given directory
and copy them to a given directory, flattening their file names by replacing
path separators with underscores. If the output directory already exists, it
will be deleted and re-created. Files ending with ".expected.wgsl" are
skipped.

Additional flags can be used for providing tooling for converting this corpus
of .wgsl  files into Tint IR fuzzer protobufs (.tirb) or for minimizing the
corpus.

usage: generate_tint_corpus.py [-h] [-v] [--debug] [--wgsl_fuzzer WGSL_FUZZER | --ir_as IR_AS | --ir_fuzzer IR_FUZZER] input_dir output_dir

Generates Tint fuzzer corpus from provided test files, using the provided tool.

positional arguments:
  input_dir             Directory containing source files to be turned into a
                        corpus, what format these are expected to be is
                        determined by the type of corpus being generated (which
                        is determined by the tool flag provided). If no tool is
                        provided the non-minimized WGSL corpus will be
                        generated, and the inputs are expected to contain .wgsl
                        WGSL shader files, non-WGSL and '*.expected.wgsl` files
                        will be ignored
  output_dir            Output directory that the results directory should be
                        placed in. The base WGSL fuzzer corpus is created, in
                        '<output_dir>/wgsl_corpus'. Other corpus locations will
                        be specified in their tool specific flags. If a
                        directory already exists, it will be overwritten.

options:
  -h, --help            show this help message and exit
  -v, --verbose         Enables verbose logging
  --debug               Enables developer debug logging

tool flags:
  Flags for tool to use for generating the corpus. Which tool that is provided
  determines which type of corpus is generated. If no tool is
  provided the non-minimized WGSL fuzzer corpus will be generated. Only one
  corpus will be generated per invocation, so these flags are mutually
  exclusive.

  --wgsl_fuzzer WGSL_FUZZER
                        Instance of tint_wgsl_fuzzer to use for minimization, if
                         provided a minimized WGSL corpus will be generated in
                        '<output_dir>/wgsl_min_corpus'. (This can take over an
                        hour to run). <input_dir> is expected to only contain
                        .wgsl WGSL shader files, as would be generated for the
                        non-minimized WGSL corpus.
  --ir_as IR_AS         Instance of ir_fuzz_as to use for assembling IR binary
                        test cases, if provided a non-minimized IR corpus will
                        be generated in '<output_dir>/ir_corpus'. <input_dir> is
                        expected to only contain .wgsl WGSL shader files, as
                        would be generated for the non-minimized WGSL corpus.
  --ir_fuzzer IR_FUZZER
                        Instance of tint_ir_fuzzer to use for minimization, if
                        provided a minimized corpus will be generated in
                        '<output_dir>/ir_min_corpus'. (This can take over an
                        hour to run). <input_dir> is expected to only contain
                        .tirb IR binary test case files, as would be generated
                        for the non-minimized IR corpus.
"""

import argparse
from enum import Enum
import logging
import os
import pathlib
import shutil
import subprocess
import sys


class Mode(Enum):
    """Different corpus output modes that the script can run in"""
    WGSL = 1
    WGSL_MIN = 2
    IR = 3
    IR_MIN = 4


# Note: When Mac upgrades to a modern version Python, this can be replaced with a TypedDict and type annotations can be used through out
class Options:
    """Container of all the control options parsed from the command line args"""

    def __init__(self, mode, wgsl_fuzzer_bin, ir_as_bin, ir_fuzzer_bin,
                 input_dir, output_dir, wgsl_corpus_dir, wgsl_min_corpus_dir,
                 ir_corpus_dir, ir_min_corpus_dir):
        self.mode = mode
        self.wgsl_fuzzer_bin = wgsl_fuzzer_bin
        self.ir_as_bin = ir_as_bin
        self.ir_fuzzer_bin = ir_fuzzer_bin
        self.input_dir = input_dir
        self.output_dir = output_dir
        self.wgsl_corpus_dir = wgsl_corpus_dir
        self.wgsl_min_corpus_dir = wgsl_min_corpus_dir
        self.ir_corpus_dir = ir_corpus_dir
        self.ir_min_corpus_dir = ir_min_corpus_dir


logger = logging.getLogger(__name__)


def parse_args():
    """Parse command line arguments and produce control options structure.
    Returns:
        A populated Options structure.
    """
    parser = argparse.ArgumentParser(
        prog='generate_tint_corpus.py',
        description=
        'Generates Tint fuzzer corpus from provided test files, using the provided tool.'
    )
    parser.add_argument(
        'input_dir',
        help=
        "Directory containing source files to be turned into a corpus, what format these are expected to be is determined by the type of corpus being generated (which is determined by the tool flag provided). If no tool is provided the non-minimized WGSL corpus will be generated, and the inputs are expected to contain .wgsl WGSL shader files, non-WGSL and '*.expected.wgsl` files will be ignored",
        type=str)
    parser.add_argument(
        'output_dir',
        help=
        "Output directory that the results directory should be placed in. The base WGSL fuzzer corpus is created, in '<output_dir>/wgsl_corpus'. Other corpus locations will be specified in their tool specific flags. If a directory already exists, it will be overwritten.",
        type=str)
    parser.add_argument('-v',
                        '--verbose',
                        help="Enables verbose logging",
                        action="store_const",
                        dest="loglevel",
                        const=logging.INFO)
    parser.add_argument('--debug',
                        help="Enables developer debug logging",
                        action="store_const",
                        dest="loglevel",
                        const=logging.DEBUG)
    tool_group = parser.add_argument_group(
        'tool flags',
        'Flags for tool to use for generating the corpus. Which tool that is provided determines which type of corpus is generated. If no tool is provided the non-minimized WGSL fuzzer corpus will be generated.\nOnly one corpus will be generated per invocation, so these flags are mutually exclusive.'
    )
    tool_group = tool_group.add_mutually_exclusive_group()
    tool_group.add_argument(
        '--wgsl_fuzzer',
        help=
        "Instance of tint_wgsl_fuzzer to use for minimization, if provided a minimized WGSL corpus will be generated in '<output_dir>/wgsl_min_corpus'. (This can take over an hour to  run). <input_dir> is expected to be only contain .wgsl WGSL shader files, as would be generated for the non-minimized WGSL corpus.",
        type=str)
    tool_group.add_argument(
        '--ir_as',
        help=
        "Instance of ir_fuzz_as to use for assembling IR binary test cases, if provided a non-minimized IR corpus will be generated in '<output_dir>/ir_corpus'. <input_dir> is expected to only contain .wgsl WGSL shader files, as would be generated for the non-minimized WGSL corpus.",
        type=str)
    tool_group.add_argument(
        '--ir_fuzzer',
        help=
        "Instance of tint_ir_fuzzer to use for minimization, if provided a minimized corpus will be generated in '<output_dir>/ir_min_corpus'. (This can take over an hour to  run). <input_dir> is expected to only contain .tirb IR binary test case files, as would be generated for the non-minimized IR corpus.",
        type=str)
    args = parser.parse_args()
    logging.basicConfig(level=args.loglevel)
    logger.debug(vars(args))

    output_dir = os.path.abspath(args.output_dir)

    wgsl_fuzzer_bin = check_binary_accessible(
        args.wgsl_fuzzer, "--wgsl_fuzzer=<tint_wgsl_fuzzer>")
    ir_as_bin = check_binary_accessible(args.ir_as, "--ir_as=<ir_fuzz_as>")
    ir_fuzzer_bin = check_binary_accessible(args.ir_fuzzer,
                                            "--ir_fuzzer=<tint_ir_fuzzer>")

    # This can be replaced with match/case if Python >= 3.10 is guaranteed
    if not wgsl_fuzzer_bin and not ir_as_bin and not ir_fuzzer_bin:
        mode = Mode.WGSL
    elif wgsl_fuzzer_bin and not ir_as_bin and not ir_fuzzer_bin:
        mode = Mode.WGSL_MIN
    elif not wgsl_fuzzer_bin and ir_as_bin and not ir_fuzzer_bin:
        mode = Mode.IR
    elif not wgsl_fuzzer_bin and not ir_as_bin and ir_fuzzer_bin:
        mode = Mode.IR_MIN
    else:
        logger.critical(
            f"Some how more than one tool managed to get set after parsing args, wgsl_fuzzer_bin = '{wgsl_fuzzer_bin}, ir_as_bin  = '{ir_as_bin}', ir_fuzzer_bin = '{ir_fuzzer_bin}"
        )
        sys.exit(1)

    return Options(mode=mode,
                   wgsl_fuzzer_bin=wgsl_fuzzer_bin,
                   ir_as_bin=ir_as_bin,
                   ir_fuzzer_bin=ir_fuzzer_bin,
                   input_dir=os.path.abspath(args.input_dir.rstrip(os.sep)),
                   output_dir=output_dir,
                   wgsl_corpus_dir=os.path.join(output_dir, "wgsl_corpus"),
                   wgsl_min_corpus_dir=os.path.join(output_dir,
                                                    "wgsl_min_corpus"),
                   ir_corpus_dir=os.path.join(output_dir, "ir_corpus"),
                   ir_min_corpus_dir=os.path.join(output_dir, "ir_min_corpus"))


def list_files_with_suffix(root_search_dir, suffix, excludes):
    """Lists all the files beneath a root directory with a given suffix.

    Args:
        root_search_dir (str): The directory to search for WGSL files in.
        suffix (str): The suffix that is being looked for
        excludes (list | None): A list of suffixes that would match 'suffix', but should not be included in the result.
    Returns:
        Yields path to any file that ends in 'suffix' but does not also end with any entry from excludes
    """
    if excludes is None:
        excludes = []

    for root, folders, files in os.walk(root_search_dir):
        for filename in folders + files:
            if filename.endswith(suffix):
                if any(filename.endswith(e) for e in excludes):
                    logger.debug(f"Skipping {filename}")
                    continue
                yield os.path.join(root, filename)


def check_binary_accessible(bin_filename, log_text):
    """Check if a binary file exists and accessible.

    Args:
        bin_filename (str|None): The filename of the binary file to check.
        log_text (str): String describing the related flag for error messages.

    Returns:
        bin_filename if it is executable & accessible, or None if it is None. Causes a fatal error if bin_filename is not None, but not executable & accessible.
    """
    if not bin_filename:
        return None

    which_bin = shutil.which(bin_filename, mode=os.F_OK | os.X_OK, path='.')
    if not which_bin:
        if not os.path.exists(os.path.join('.', bin_filename)):
            logger.critical(
                f"Unable to run {log_text} cmd: '{bin_filename}' does not exist in path"
            )
        elif not os.access(os.path.join('.', bin_filename), os.X_OK):
            logger.critical(
                f"Unable to run {log_text} cmd: '{bin_filename}' is not executable"
            )
        else:
            logger.critical(
                f"Unable to run {log_text} cmd: '{bin_filename}' for unknown reason"
            )
        sys.exit(1)
    return which_bin


def create_clean_dir(dirname):
    """Makes sure there is an empty directory at location.

    Will remove the directory if it already exists and recreate it.

    Args:
        dirname (str): The directory to create.
    """
    if os.path.exists(dirname):
        shutil.rmtree(dirname)
    os.makedirs(dirname)


def touch_stamp_file(output_dir, task_name):
    """Touches a stamp file to record when a task completed.

    Args:
        output_dir (str): The directory to touch the stamp file in.
        task_name (str): The name of the task being stamped for.
    """
    stamp_file = os.path.join(output_dir, f"{task_name}.stamp")
    logger.debug(f"Touching {task_name}.stamp")
    pathlib.Path(stamp_file).touch(mode=0o644, exist_ok=True)


def generate_wgsl_corpus(options):
    """Generate non-minimized WGSL corpus

    Args:
        options (Options): Control options parsed from the command line.
    """
    logger.info(f"Generating WGSL corpus to \'{options.wgsl_corpus_dir}\' ...")
    create_clean_dir(options.wgsl_corpus_dir)
    for in_file in list_files_with_suffix(options.input_dir, ".wgsl",
                                          [".expected.wgsl"]):
        out_file = in_file[len(options.input_dir) + 1:].replace(os.sep, '_')
        logger.debug("Copying " + in_file + " to " + out_file)
        shutil.copy(in_file, os.path.join(options.wgsl_corpus_dir, out_file))
    touch_stamp_file(options.output_dir, "wgsl")
    logger.info("Finished generating WGSL corpus")


def generate_wgsl_minimized_corpus(options):
    """Generate minimized WGSL corpus

    Args:
        options (Options): Control options parsed from the command line.
    """
    logger.info(
        f"Minimizing WGSL corpus to \'{options.wgsl_min_corpus_dir}\' (this will take a while) ..."
    )
    create_clean_dir(options.wgsl_min_corpus_dir)

    # libFuzzer uses TO FROM args for merging/minimization
    min_cmd = [
        options.wgsl_fuzzer_bin, '-merge=1', options.wgsl_min_corpus_dir,
        options.input_dir
    ]
    logger.info(f"Invoking \'{' '.join(min_cmd)}\'")
    subprocess.run(min_cmd)

    touch_stamp_file(options.output_dir, "wgsl_min")
    logger.info("Finished minimizing WGSL corpus")


def generate_ir_corpus(options):
    """Generate non-minimized IR corpus

    Args:
        options (Options): Control options parsed from the command line.
    """
    logger.info(f"Generating IR corpus to \'{options.ir_corpus_dir}\' ...")
    create_clean_dir(options.ir_corpus_dir)

    gen_cmd = [options.ir_as_bin, options.input_dir, options.ir_corpus_dir]
    logger.info(f"Invoking \'{' '.join(gen_cmd)}\'")
    subprocess.run(gen_cmd)

    touch_stamp_file(options.output_dir, "ir")
    logger.info("Finished generating IR corpus")


def generated_ir_minimized_corpus(options):
    """Generate minimized IR corpus

    Args:
        options (Options): Control options parsed from the command line.
    """
    logger.info(
        f"Minimizing IR corpus to \'{options.ir_min_corpus_dir}\' (this will take a while) ..."
    )
    create_clean_dir(options.ir_min_corpus_dir)

    # libFuzzer uses TO FROM args for merging/minimization
    min_cmd = [
        options.ir_fuzzer_bin, '-merge=1', options.ir_min_corpus_dir,
        options.input_dir
    ]
    logger.info(f"Invoking \'{' '.join(min_cmd)}\'")
    subprocess.run(min_cmd)

    touch_stamp_file(options.output_dir, "ir_min")
    logger.info("Finished minimizing IR corpus")


# Builder function map (can be replaced with match/case if Python >= 3.10 is guaranteed)
builders = {
    Mode.WGSL: generate_wgsl_corpus,
    Mode.WGSL_MIN: generate_wgsl_minimized_corpus,
    Mode.IR: generate_ir_corpus,
    Mode.IR_MIN: generated_ir_minimized_corpus,
}


def main():
    options = parse_args()
    builders[options.mode](options)


if __name__ == "__main__":
    sys.exit(main())
