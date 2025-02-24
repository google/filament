# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import codecs
import argparse
import os
import sys

import py_vulcanize

import tracing_project


def Main(args):
  parser = argparse.ArgumentParser(usage='%(prog)s --outdir=<directory>')
  parser.add_argument('--outdir', dest='out_dir',
                      help='Where to place generated content')
  parser.add_argument('--no-min', default=False, action='store_true',
                      help='Skip minification')
  args = parser.parse_args(args)

  if not args.out_dir:
    sys.stderr.write('ERROR: Must specify --outdir=<directory>')
    parser.print_help()
    return 1

  names = ['tracing.ui.extras.about_tracing.about_tracing']
  project = tracing_project.TracingProject()

  vulcanizer = project.CreateVulcanizer()
  load_sequence = vulcanizer.CalcLoadSequenceForModuleNames(names)

  olddir = os.getcwd()
  try:
    if not os.path.exists(args.out_dir):
      os.makedirs(args.out_dir)
    o = codecs.open(os.path.join(args.out_dir, 'about_tracing.html'), 'w',
                    encoding='utf-8')
    try:
      py_vulcanize.GenerateStandaloneHTMLToFile(
          o,
          load_sequence,
          title='chrome://tracing',
          flattened_js_url='tracing.js',
          minify=not args.no_min)
    except py_vulcanize.module.DepsException as ex:
      sys.stderr.write('Error: %s\n\n' % str(ex))
      return 255
    o.close()

    o = codecs.open(os.path.join(args.out_dir, 'about_tracing.js'), 'w',
                    encoding='utf-8')
    assert o.encoding == 'utf-8'
    py_vulcanize.GenerateJSToFile(
        o,
        load_sequence,
        use_include_tags_for_scripts=False,
        dir_for_include_tag_root=args.out_dir,
        minify=not args.no_min)
    o.close()

  finally:
    os.chdir(olddir)

  return 0
