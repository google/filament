#!/usr/bin/env python3
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import os
import shutil
import subprocess
import sys

netlog_viewer_root_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(netlog_viewer_root_path)
import netlog_viewer_project


project = netlog_viewer_project.NetlogViewerProject()

src_dir = project.netlog_viewer_src_path
out_dir = os.path.join(netlog_viewer_root_path, "appengine", "static")
components_dir = os.path.join(project.catapult_third_party_path,
                              "polymer", "components")

if os.path.exists(out_dir):
  shutil.rmtree(out_dir)

os.mkdir(out_dir)

in_html = os.path.join(src_dir, 'index.html')
out_html = os.path.join(out_dir, 'vulcanized.html')

try:
  subprocess.check_call(['vulcanize', in_html,
                         '--inline-scripts', '--inline-css', '--strip-comments',
                         '--redirect', '/components|' + components_dir,
                         '--redirect', '/third_party|'
                            + project.catapult_third_party_path,
                         '--out-html', out_html])
except OSError:
  sys.stderr.write('''
ERROR: Could not execute "vulcanize".

To install vulcanize on Linux:
  sudo apt-get install npm
  sudo npm install -g vulcanize
'''[1:])
  sys.exit(1)

