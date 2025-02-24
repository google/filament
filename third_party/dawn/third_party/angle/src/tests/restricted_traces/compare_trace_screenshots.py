#! /usr/bin/env python3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
'''
compare_trace_screenshots.py

This script will cycle through screenshots from traces and compare them in useful ways.

It can run in multiple ways.

* `versus_native`

  This mode expects to be run in a directory full of two sets of screenshots

    angle_trace_tests --run-to-key-frame --screenshot-dir /tmp/screenshots
    angle_trace_tests --run-to-key-frame --screenshot-dir /tmp/screenshots --use-gl=native
    python3 compare_trace_screenshots.py versus_native --screenshot-dir /tmp/screenshots --trace-list-path ~/angle/src/tests/restricted_traces/

* `versus_upgrade`

  This mode expects to be pointed to two directories of identical images (same names and pixel contents)

    python3 compare_trace_screenshots.py versus_upgrade --before /my/trace/before --after /my/trace/after --out /my/trace/compare

Prerequisites
sudo apt-get install imagemagick
'''

import argparse
import json
import logging
import os
import subprocess
import sys

DEFAULT_LOG_LEVEL = 'info'

EXIT_SUCCESS = 0
EXIT_FAILURE = 1


def versus_native(args):

    # Get a list of all PNG files in the directory
    png_files = os.listdir(args.screenshot_dir)

    # Build a set of unique trace names
    traces = set()

    def get_traces_from_images():
        # Iterate through the PNG files
        for png_file in sorted(png_files):
            if png_file.startswith("angle_native") or png_file.startswith("angle_vulkan"):
                # Strip the prefix and the PNG extension from the file name
                trace_name = png_file.replace("angle_vulkan_",
                                              "").replace("swiftshader_",
                                                          "").replace("angle_native_",
                                                                      "").replace(".png", "")
                traces.add(trace_name)

    def get_traces_from_file(restricted_traces_path):
        with open(os.path.join(restricted_traces_path, "restricted_traces.json")) as f:
            trace_data = json.load(f)

        # Have to split the 'trace version' thing up
        trace_and_version = trace_data['traces']
        for i in trace_and_version:
            traces.add(i.split(' ',)[0])

    def get_trace_key_frame(restricted_traces_path, trace):
        with open(os.path.join(restricted_traces_path, trace, trace + ".json")) as f:
            single_trace_data = json.load(f)

        metadata = single_trace_data['TraceMetadata']
        keyframe = ""
        if 'KeyFrames' in metadata:
            keyframe = metadata['KeyFrames'][0]
        return keyframe

    if args.trace_list_path != None:
        get_traces_from_file(args.trace_list_path)
    else:
        get_traces_from_images()

    for trace in sorted(traces):
        if args.trace_list_path != None:
            keyframe = get_trace_key_frame(args.trace_list_path, trace)
            frame = ""
            if keyframe != "":
                frame = "_frame" + str(keyframe)

        native_file = "angle_native_" + trace + frame + ".png"
        native_file = os.path.join(args.screenshot_dir, native_file)
        if not os.path.isfile(native_file):
            native_file = "MISSING_EXT.png"

        vulkan_file = "angle_vulkan_" + trace + frame + ".png"
        vulkan_file = os.path.join(args.screenshot_dir, vulkan_file)
        if not os.path.isfile(vulkan_file):
            vulkan_file = "angle_vulkan_swiftshader_" + trace + frame + ".png"
            vulkan_file = os.path.join(args.screenshot_dir, vulkan_file)
            if not os.path.isfile(vulkan_file):
                vulkan_file = "MISSING_EXT.png"

        # Compare each of the images with different fuzz factors so we can see how each is doing
        # `compare -metric AE -fuzz ${FUZZ} ${VULKAN} ${NATIVE} ${TRACE}_fuzz${FUZZ}_diff.png`
        results = []
        for fuzz in {0, 1, 2, 5, 10, 20}:
            diff_file = trace + "_fuzz" + str(fuzz) + "%_TEST_diff.png"
            diff_file = os.path.join(args.screenshot_dir, diff_file)
            command = "compare -metric AE -fuzz " + str(
                fuzz) + "% " + vulkan_file + " " + native_file + " " + diff_file
            logging.debug("Running " + command)
            diff = subprocess.run(command, shell=True, capture_output=True)
            for line in diff.stderr.splitlines():
                if "unable to open image".encode('UTF-8') in line:
                    results.append("NA".encode('UTF-8'))
                else:
                    results.append(diff.stderr)
            logging.debug(" for " + trace + " " + str(fuzz) + "%")

        print(trace, os.path.basename(vulkan_file), os.path.basename(native_file),
              results[0].decode('UTF-8'), results[1].decode('UTF-8'), results[2].decode('UTF-8'),
              results[3].decode('UTF-8'), results[4].decode('UTF-8'), results[5].decode('UTF-8'))


def versus_upgrade(args):

    # Get a list of all the files in before
    before_files = sorted(os.listdir(args.before))

    # Get a list of all the files in after
    after_files = sorted(os.listdir(args.after))

    # If either list is missing files, this is a fail!
    if before_files != after_files:
        before_minus_after = list(sorted(set(before_files) - set(after_files)))
        after_minus_before = list(sorted(set(after_files) - set(before_files)))
        print("File lists don't match!")
        if before_minus_after is not []:
            print("Extra before files: %s" % before_minus_after)
        if after_minus_before is not []:
            print("Extra after files: %s" % after_minus_before)
        exit(1)

    # Walk through the before list and compare it with after
    for before_image, after_image in zip(sorted(before_files), sorted(after_files)):

        # Compare each of the images using root mean squared, no fuzz factor
        # `compare -metric RMSE ${BEFORE} ${AFTER} ${TRACE}_RMSE_diff.png;`

        results = []
        diff_file = args.outdir + "/" + before_image + "_TEST_diff.png"
        command = "compare -metric RMSE " + os.path.join(
            args.before, before_image) + " " + os.path.join(args.after,
                                                            after_image) + " " + diff_file
        diff = subprocess.run(command, shell=True, capture_output=True)
        for line in diff.stderr.splitlines():
            if "unable to open image".encode('UTF-8') in line:
                results.append("NA".encode('UTF-8'))
            else:
                # If the last element of the diff isn't zero, there was a pixel diff
                if line.split()[-1] != b'(0)':
                    print(before_image, diff.stderr.decode('UTF-8'))
                    print("Pixel diff detected!")
                    exit(1)
                else:
                    results.append(diff.stderr)

        print(before_image, results[0].decode('UTF-8'))

    print("Test completed successfully, no diffs detected")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--log', help='Logging level.', default=DEFAULT_LOG_LEVEL)

    # Create commands for different modes of using this script
    subparsers = parser.add_subparsers(dest='command', required=True, help='Command to run.')

    # This mode will compare images of two runs, vulkan vs. native, and give you fuzzy comparison results
    versus_native_parser = subparsers.add_parser(
        'versus_native', help='Compares vulkan vs. native images.')
    versus_native_parser.add_argument(
        '--screenshot-dir', help='Directory containing two sets of screenshots', required=True)
    versus_native_parser.add_argument(
        '--trace-list-path', help='Path to dir containing restricted_traces.json')

    # This mode will compare before and after images when upgrading a trace
    versus_upgrade_parser = subparsers.add_parser(
        'versus_upgrade', help='Compare images before and after an upgrade')
    versus_upgrade_parser.add_argument(
        '--before', help='Full path to dir containing *before* screenshots', required=True)
    versus_upgrade_parser.add_argument(
        '--after', help='Full path to dir containing *after* screenshots', required=True)
    versus_upgrade_parser.add_argument('--outdir', help='Where to write output files', default='.')

    args = parser.parse_args()

    try:
        if args.command == 'versus_native':
            return versus_native(args)
        elif args.command == 'versus_upgrade':
            return versus_upgrade(args)
        else:
            logging.fatal('Unknown command: %s' % args.command)
            return EXIT_FAILURE
    except subprocess.CalledProcessError as e:
        logging.exception('There was an exception: %s', e.output.decode())
        return EXIT_FAILURE


if __name__ == '__main__':
    sys.exit(main())
