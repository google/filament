#!/usr/bin/env vpython3
# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Redirects to the version of the metrics XML formatter in the Chromium tree.
"""
import gclient_paths
import os
import shutil
import subprocess
import sys


def GetMetricsDir(top_dir, path):
    metrics_xml_dirs = [
        os.path.join(top_dir, 'tools', 'metrics', 'actions'),
        os.path.join(top_dir, 'tools', 'metrics', 'histograms'),
        os.path.join(top_dir, 'tools', 'metrics', 'structured'),
        os.path.join(top_dir, 'tools', 'metrics', 'ukm'),
    ]
    abs_dirname = os.path.dirname(os.path.realpath(path))
    for xml_dir in metrics_xml_dirs:
        if abs_dirname.startswith(xml_dir):
            return xml_dir
    return None


def IsSupportedHistogramsXML(path):
    supported_xmls = set([
        'histograms.xml',
        'enums.xml',
        'histogram_suffixes_list.xml',
    ])
    return os.path.basename(path) in supported_xmls


def log(msg, verbose):
    if verbose:
        print(msg)


def FindMetricsXMLFormatterTool(path, verbose=False):
    """Returns a path to the metrics XML formatter executable."""
    top_dir = gclient_paths.GetPrimarySolutionPath()
    if not top_dir:
        log('Not executed in a Chromium checkout; skip formatting', verbose)
        return ''
    xml_dir = GetMetricsDir(top_dir, path)
    if not xml_dir:
        log(f'{path} is not a metric XML; skip formatting', verbose)
        return ''
    # Just to ensure that the given file is located in the current checkout
    # folder. If not, skip the formatting.
    if not os.path.realpath(path).startswith(os.path.realpath(top_dir)):
        log(
            f'{path} is not located in the current Chromium checkout; '
            'skip formatting', verbose)
        return ''

    histograms_base_dir = os.path.join(top_dir, 'tools', 'metrics',
                                       'histograms')
    if xml_dir.startswith(histograms_base_dir):
        # Skips the formatting, if the XML file is not one of the known types.
        if not IsSupportedHistogramsXML(path):
            log(f'{path} is not a supported histogram XML; skip formatting',
                verbose)
            return ''

    return os.path.join(xml_dir, 'pretty_print.py')


usage_text = """Usage: %s [option] filepath

Format a given metrics XML file with the metrics XML formatters in the Chromium
checkout. Noop, if executed out of a Chromium checkout.

Note that not all the options are understood by all the formatters.
Find the formatter binaries for all the options supported by each formatter.

positional arguments:
  filepath           if the path is not under tools/metrics,
                     no formatter will be run.

options:,
  -h, --help          show this help message and exit'
  --presubmit
  --diff"""


def _print_help():
    print(usage_text % sys.argv[0])


def main(args):
    path = next((arg for arg in args if not arg.startswith('-')), None)
    if not path:
        _print_help()
        return 0
    if not os.path.exists(path):
        raise FileNotFoundError(f'{path} does not exist.')

    tool = FindMetricsXMLFormatterTool(path, verbose=True)
    if not tool:
        # Fail (almost) silently.
        return 0

    subprocess.run([
        shutil.which('vpython3'),
        tool,
    ] + args)
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main(sys.argv[1:]))
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
