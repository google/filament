# Copyright (C) 2024 The Android Open Source Project
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

from utils import execute, ArgParseImpl

import glob
from itertools import chain
import json
import sys
import os
from os import path

def _is_list_of_strings(field):
  return isinstance(field, list) and\
      all(isinstance(item, str) for item in field)

def _is_string(s):
    return isinstance(s, str)

def _is_dict(s):
    return isinstance(s, dict)

class RenderingConfig():
  def __init__(self, data):
    assert 'name' in data
    assert _is_string(data['name'])
    self.name = data['name']

    assert 'rendering' in data
    assert _is_dict(data['rendering'])
    self.rendering = data['rendering']

class PresetConfig(RenderingConfig):
  def __init__(self, data, existing_models):
    RenderingConfig.__init__(self, data)
    models = data.get('models')
    if models:
      assert _is_list_of_strings(models)
      assert all(m in existing_models for m in models)
      self.models = models

class TestConfig(RenderingConfig):
  def __init__(self, data, existing_models, presets):
    RenderingConfig.__init__(self, data)
    description = data.get('description')
    if description:
      assert _is_string(description)
      self.description = description

    apply_presets = data.get('apply_presets')
    rendering = {}
    preset_models = []
    if apply_presets:
      given_presets = {p.name: p for p in presets}
      assert all((name in given_presets) for name in apply_presets)
      for preset in apply_presets:
        rendering.update(given_presets[preset].rendering)
        preset_models += given_presets[preset].models

    assert 'rendering' in data
    rendering.update(data['rendering'])
    self.rendering = rendering

    models = data.get('models')
    self.models = preset_models
    if models:
      assert _is_list_of_strings(models)
      assert all(m in existing_models for m in models)
      self.models = set(models + self.models)

  def to_filament_format(self):
    json_out = {
        'name': self.name,
        'base': self.rendering
    }
    return json.dumps(json_out)

class RenderTestConfig():
  def __init__(self, data):
    assert 'name' in data
    name = data['name']
    assert _is_string(name)
    self.name = name

    assert 'backends' in data
    backends = data['backends']
    assert _is_list_of_strings(backends)
    self.backends = backends

    assert 'model_search_paths' in data
    model_search_paths = data.get('model_search_paths')
    assert _is_list_of_strings(model_search_paths)
    assert all(path.isdir(p) for p in model_search_paths)

    model_paths = list(
        chain(*(glob.glob(f'{d}/**/*.glb', recursive=True) for d in model_search_paths)))
    # This flatten the output for glob.glob
    self.models = {path.splitext(path.basename(model))[0]: model for model in model_paths}

    preset_data = data.get('presets')
    presets = []
    if preset_data:
      presets = [PresetConfig(p, self.models) for p in preset_data]

    assert 'tests' in data
    self.tests = [TestConfig(t, self.models, presets) for t in data['tests']]
    test_names = list([t.name for t in self.tests])

    # We cannot have duplicate test names
    assert len(test_names) == len(set(test_names))

def _remove_comments_from_json_txt(json_txt):
  res = []
  for line in json_txt.split('\n'):
    if '//' in line:
      line = line.split('//')[0]
    res.append(line)
  return '\n'.join(res)

def parse_test_config_from_path(config_path):
  with open(config_path, 'r') as f:
    json_txt = json.loads(_remove_comments_from_json_txt(f.read()))
    return RenderTestConfig(json_txt)


if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = parse_test_config_from_path(args.test)
