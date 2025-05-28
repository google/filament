# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import flask
import pathlib
import json

from utils import ArgParseImpl

from flask import Flask, request, make_response, send_from_directory

DIR = pathlib.Path(__file__).parent.absolute()
HTML_DIR = os.path.join(DIR, "viewer_html")

def _create_app(config):
  app = Flask(__name__)

  client_config = config.copy()
  base_dir = client_config['base_dir']
  comparison_dir = client_config['comparison_dir']
  diff_dir = client_config['diff_dir']

  del client_config['base_dir']
  del client_config['comparison_dir']
  del client_config['diff_dir']

  @app.route('/r/', methods=['GET'])
  def get_r():
    return json.dumps(client_config)

  @app.route('/g/<path:filepath>', methods=['GET'])
  def get_g(filepath):
    return send_from_directory(base_dir, filepath)

  @app.route('/d/<path:filepath>', methods=['GET'])
  def get_d(filepath):
    return send_from_directory(diff_dir, filepath)

  @app.route('/c/<path:filepath>', methods=['GET'])
  def get_c(filepath):
    return send_from_directory(comparison_dir, filepath)

  @app.route('/<path:filepath>')
  def get_static_file(filepath):
    return send_from_directory(HTML_DIR, filepath)

  @app.route('/')
  def get_index():
    return send_from_directory(HTML_DIR, 'index.html')

  app.url_map.strict_slashes = False
  return app

if __name__ == '__main__':
  PORT = 8901
  parser = ArgParseImpl()
  parser.add_argument('--diff', help='Diff directory', required=True)
  args, _ = parser.parse_known_args(sys.argv[1:])

  with open(os.path.join(args.diff, 'compare_results.json'), 'r') as f:
    config = json.loads(f.read())
  config['diff_dir'] = os.path.abspath(args.diff)

  app = _create_app(config)
  from waitress import serve

  print(f'Point your browser to http://localhost:{PORT} to see the diff results')
  serve(app, host="127.0.0.1", port=PORT)
