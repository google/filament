#!/usr/bin/env python3

import os, sys

build_path = sys.argv[1]
if os.path.exists(build_path):
  for (path, dir, files) in os.walk(build_path):
    for cur_file in files:
      if cur_file.endswith('index.lock'):
        path_to_file = os.path.join(path, cur_file)
        print('deleting %s' % path_to_file)
        os.remove(path_to_file)

