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
    self.models = []

    def _check(models):
      assert _is_list_of_strings(models)
      assert all(m in existing_models for m in models)

    models = data.get('models')
    if models:
      _check(models)
      self.models = models
    model_list_file = data.get('model_list_file')
    if model_list_file and os.path.exists(model_list_file):
      with open(model_list_file, 'r') as f:
        models = list(filter(lambda a: len(a) > 0, map(lambda a: a.strip(), f.read().split('\n'))))
        _check(models)
        self.models += models

    # Parse tolerance configuration from preset
    tolerance = data.get('tolerance')
    if tolerance:
      assert _is_dict(tolerance)
      self._validate_tolerance(tolerance)
      self.tolerance = tolerance
    else:
      self.tolerance = None

  def _validate_tolerance(self, tolerance):
    """
    Validate tolerance configuration structure.

    Tolerance can be:
    1. Single criteria: {"max_pixel_diff": 5, "allowed_diff_pixels": 1.0}
    2. Nested criteria: {"operator": "OR", "criteria": [...]}
    """
    if 'criteria' in tolerance:
      # Nested structure with operator
      operator = tolerance.get('operator', 'AND')
      assert operator.upper() in ['AND', 'OR'], f"Invalid operator: {operator}"

      criteria_list = tolerance['criteria']
      assert isinstance(criteria_list, list), "criteria must be a list"
      assert len(criteria_list) > 0, "criteria list cannot be empty"

      # Recursively validate each criteria
      for criteria in criteria_list:
        self._validate_tolerance(criteria)
    else:
      # Leaf criteria - validate individual parameters
      valid_keys = {'max_pixel_diff', 'max_pixel_diff_percent', 'allowed_diff_pixels'}
      tolerance_keys = set(tolerance.keys())
      invalid_keys = tolerance_keys - valid_keys
      assert len(invalid_keys) == 0, f"Invalid tolerance keys: {invalid_keys}"

      if 'max_pixel_diff' in tolerance:
        assert isinstance(tolerance['max_pixel_diff'], (int, float)), "max_pixel_diff must be numeric"
        assert 0 <= tolerance['max_pixel_diff'] <= 255, "max_pixel_diff must be 0-255"

      if 'max_pixel_diff_percent' in tolerance:
        assert isinstance(tolerance['max_pixel_diff_percent'], (int, float)), "max_pixel_diff_percent must be numeric"
        assert 0 <= tolerance['max_pixel_diff_percent'] <= 100, "max_pixel_diff_percent must be 0-100%"

      if 'allowed_diff_pixels' in tolerance:
        assert isinstance(tolerance['allowed_diff_pixels'], (int, float)), "allowed_diff_pixels must be numeric"
        assert 0 <= tolerance['allowed_diff_pixels'] <= 100, "allowed_diff_pixels must be 0-100%"

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
    preset_tolerance = None
    if apply_presets:
      given_presets = {p.name: p for p in presets}
      assert all((name in given_presets) for name in apply_presets),\
        f'used preset {name} which is not in {given_presets}'

      # Note that this needs to applied in order.  Models will be overwritten.
      # Properties will be "added" in order.
      # Tolerance is inherited from the LAST preset that has one defined
      for preset in apply_presets:
        rendering.update(given_presets[preset].rendering)
        preset_models = given_presets[preset].models
        if given_presets[preset].tolerance:
          preset_tolerance = given_presets[preset].tolerance

    assert 'rendering' in data
    rendering.update(data['rendering'])
    self.rendering = rendering

    models = data.get('models')
    self.models = preset_models
    if models:
      assert _is_list_of_strings(models)
      assert all(m in existing_models for m in models)
      self.models = set(models + self.models)

    # Parse tolerance configuration - test-level tolerance overrides preset tolerance
    tolerance = data.get('tolerance')
    if tolerance:
      assert _is_dict(tolerance)
      self._validate_tolerance(tolerance)
      self.tolerance = tolerance
    else:
      # Use tolerance inherited from presets
      self.tolerance = preset_tolerance

  def _validate_tolerance(self, tolerance):
    """Use the same validation logic as PresetConfig."""
    # Create a temporary PresetConfig instance to reuse validation logic
    temp_preset = PresetConfig({'name': 'temp', 'rendering': {}}, {})
    return temp_preset._validate_tolerance(tolerance)

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
        chain(*(glob.glob(f'{d}/**/*.glb', recursive=True) for d in model_search_paths))) + \
        list(
          chain(*(glob.glob(f'{d}/**/*.gltf', recursive=True) for d in model_search_paths)))
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

def parse_from_path(config_path):
  with open(config_path, 'r') as f:
    json_txt = json.loads(_remove_comments_from_json_txt(f.read()))
    return RenderTestConfig(json_txt)


if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = parse_test_config_from_path(args.test)
