#!/usr/bin/env python

#  Copyright 2018 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# This tool will create a json description of the GN build environment that
# can then be used by gen_angle_android_bp.py to build an Android.bp file for
# the Android Soong build system.
# The input to this tool is a list of GN labels for which to capture the build
# information in json:
#
# Generating angle.json needs to be done from within a Chromium build:
#   cd <chromium>/src
#   gen_angle_gn_info_json.py //third_party/angle:libGLESv2 //third_party/angle:libEGL
#
# This will output an angle.json that can be copied to the angle directory
# within Android.
#
# Optional arguments:
#  --gn_out <file>  GN output config to use (e.g., out/Default or out/Debug.)
#  --output <file>  json file to create, default is angle.json
#

import argparse
import json
import logging
import subprocess
import sys


def get_json_description(gn_out, target_name):
    try:
        text_desc = subprocess.check_output(['gn', 'desc', '--format=json', gn_out, target_name])
    except subprocess.CalledProcessError as e:
        logging.error("e.retcode = %s" % e.returncode)
        logging.error("e.cmd = %s" % e.cmd)
        logging.error("e.output = %s" % e.output)
    try:
        json_out = json.loads(text_desc)
    except ValueError:
        raise ValueError("Unable to decode JSON\ncmd: %s\noutput:\n%s" % (subprocess.list2cmdline(
            ['gn', 'desc', '--format=json', gn_out, target_name]), text_desc))

    return json_out


def load_json_deps(desc, gn_out, target_name, all_desc, indent="  "):
    """Extracts dependencies from the given target json description
       and recursively extracts json descriptions.

       desc: json description for target_name that includes dependencies
       gn_out: GN output file with configuration info
       target_name: name of target in desc to lookup deps
       all_desc: dependent descriptions added here
       indent: Print with indent to show recursion depth
    """
    target = desc[target_name]
    text_descriptions = []
    for dep in target.get('deps', []):
        if dep not in all_desc:
            logging.debug("dep: %s%s" % (indent, dep))
            new_desc = get_json_description(gn_out, dep)
            all_desc[dep] = new_desc[dep]
            load_json_deps(new_desc, gn_out, dep, all_desc, indent + "  ")
        else:
            logging.debug("dup: %s%s" % (indent, dep))


def create_build_description(gn_out, targets):
    """Creates the JSON build description by running GN."""

    logging.debug("targets = %s" % targets)
    json_descriptions = {}
    for target in targets:
        logging.debug("target: %s" % (target))
        target_desc = get_json_description(gn_out, target)
        if (target in target_desc and target not in json_descriptions):
            json_descriptions[target] = target_desc[target]
            load_json_deps(target_desc, gn_out, target, json_descriptions)
        else:
            logging.debug("Invalid target: %s" % target)
    return json_descriptions


def main():
    logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
    parser = argparse.ArgumentParser(
        description='Generate json build information from a GN description.')
    parser.add_argument(
        '--gn_out',
        help='GN output config to use (e.g., out/Default or out/Debug.)',
        default='out/Default',
    )
    parser.add_argument(
        '--output',
        help='json file to create',
        default='angle.json',
    )
    parser.add_argument(
        'targets',
        nargs=argparse.REMAINDER,
        help='Targets to include in the json (e.g., "//libEGL")')
    args = parser.parse_args()

    desc = create_build_description(args.gn_out, args.targets)
    fh = open(args.output, "w")
    fh.write(json.dumps(desc, indent=4, sort_keys=True))
    fh.close()

    print("Output written to: %s" % args.output)


if __name__ == '__main__':
    sys.exit(main())
