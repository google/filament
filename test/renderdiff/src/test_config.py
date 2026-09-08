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
from typing import Optional

def _is_list_of_strings(field):
  return isinstance(field, list) and all(isinstance(item, str) for item in field)

def _is_string(s):
  return isinstance(s, str)

def _is_dict(s):
  return isinstance(s, dict)

def validate_tolerance(tolerance):
  """
  Validate tolerance configuration structure.

  Tolerance follows the imagediff/diffimg schema:
  {
    "mode": "LEAF" | "AND" | "OR",
    "maxAbsDiff": float,           // For LEAF mode
    "maxFailingPixelsFraction": float, // Global check
    "children": [...]              // For AND/OR mode
  }
  """
  valid_keys = {'mode', 'maxAbsDiff', 'maxFailingPixelsFraction', 'children', 'swizzle', 'channelMask', 'shiftRadius', 'blurRadius'}
  tolerance_keys = set(tolerance.keys())
  invalid_keys = tolerance_keys - valid_keys
  assert len(invalid_keys) == 0, f"Invalid tolerance keys: {invalid_keys}"

  if 'children' in tolerance:
    mode = tolerance.get('mode', 'AND')
    assert mode in ['AND', 'OR'], f"Invalid mode for node with children: {mode}"

    children = tolerance['children']
    assert isinstance(children, list), "children must be a list"
    assert len(children) > 0, "children list cannot be empty"

    for child in children:
      validate_tolerance(child)
  else:
    if 'maxAbsDiff' in tolerance:
      assert isinstance(tolerance['maxAbsDiff'], (int, float)), "maxAbsDiff must be numeric"
      assert tolerance['maxAbsDiff'] >= 0, "maxAbsDiff must be non-negative"

    if 'maxFailingPixelsFraction' in tolerance:
      assert isinstance(tolerance['maxFailingPixelsFraction'], (int, float)), "maxFailingPixelsFraction must be numeric"
      assert 0 <= tolerance['maxFailingPixelsFraction'] <= 1.0, "maxFailingPixelsFraction must be 0.0-1.0"


_MODEL_SCAN_CACHE = {}

def scan_models(search_paths, base_dir=None):
  """
  Recursively scan for .glb and .gltf files across search_paths.
  Results are cached by the set of resolved search paths.
  """
  resolved_paths = []
  for p in search_paths:
    if not path.isabs(p):
      if path.isdir(p):
        resolved_paths.append(path.abspath(p))
      elif base_dir and path.isdir(path.join(base_dir, p)):
        resolved_paths.append(path.abspath(path.join(base_dir, p)))
      else:
        resolved_paths.append(path.abspath(p))
    else:
      resolved_paths.append(path.abspath(p))

  cache_key = tuple(sorted(set(resolved_paths)))
  if cache_key in _MODEL_SCAN_CACHE:
    return _MODEL_SCAN_CACHE[cache_key]

  models = {}
  for d in resolved_paths:
    if not path.isdir(d):
      continue
    glb_files = glob.glob(f'{d}/**/*.glb', recursive=True)
    gltf_files = glob.glob(f'{d}/**/*.gltf', recursive=True)
    for model_file in chain(glb_files, gltf_files):
      name = path.splitext(path.basename(model_file))[0]
      models[name] = path.abspath(model_file)

  _MODEL_SCAN_CACHE[cache_key] = models
  return models


class GltfTestConfig:
  def __init__(self, data, inherited_search_paths=None, inherited_models=None, inherited_rendering=None, base_dir=None):
    assert _is_dict(data), "gltf_test must be a dictionary"

    # 1. Resolve model search paths
    search_paths = list(inherited_search_paths or [])
    if 'model_search_paths' in data:
      test_search_paths = data['model_search_paths']
      assert _is_list_of_strings(test_search_paths), "gltf_test.model_search_paths must be a list of strings"
      for sp in test_search_paths:
        if sp not in search_paths:
          search_paths.append(sp)
    self.model_search_paths = search_paths

    # 2. Discover available models
    self.models_map = scan_models(self.model_search_paths, base_dir=base_dir)

    # 3. Resolve models list
    models = list(inherited_models or [])
    if 'models' in data:
      test_models = data['models']
      assert _is_list_of_strings(test_models), "gltf_test.models must be a list of strings"
      models = test_models
    self.models = models

    # Check that each model exists if search paths are provided
    if self.model_search_paths and self.models:
      for m in self.models:
        assert m in self.models_map, f"Model '{m}' not found in model_search_paths: {self.model_search_paths}"

    # 4. Resolve rendering automation dictionary
    rendering = dict(inherited_rendering or {})
    if 'rendering' in data:
      assert _is_dict(data['rendering']), "gltf_test.rendering must be a dictionary"
      rendering.update(data['rendering'])
    self.rendering = rendering


class SampleTestConfig:
  def __init__(self, data, inherited_warmup_frames=None, inherited_fixed_timestep=None, inherited_args=None):
    assert _is_dict(data), "sample_test must be a dictionary"

    assert 'executable' in data, "sample_test must specify 'executable'"
    assert _is_string(data['executable']), "sample_test.executable must be a string"
    self.executable = data['executable']

    self.target = data.get('target', self.executable)
    assert _is_string(self.target), "sample_test.target must be a string"

    warmup_default = inherited_warmup_frames if inherited_warmup_frames is not None else 10
    self.warmup_frames = data.get('warmup_frames', warmup_default)
    assert isinstance(self.warmup_frames, int) and self.warmup_frames >= 0, "sample_test.warmup_frames must be a non-negative integer"

    timestep_default = inherited_fixed_timestep if inherited_fixed_timestep is not None else 0.0166667
    self.fixed_timestep = float(data.get('fixed_timestep', timestep_default))

    args = list(inherited_args or [])
    if 'args' in data:
      assert _is_list_of_strings(data['args']), "sample_test.args must be a list of strings"
      args += data['args']
    self.args = args


class PresetConfig:
  def __init__(self, data, base_dir=None):
    assert 'name' in data, "Preset must have a 'name'"
    assert _is_string(data['name']), "Preset 'name' must be a string"
    self.name = data['name']

    # Parse tolerance configuration from preset
    tolerance = data.get('tolerance')
    if tolerance:
      assert _is_dict(tolerance), "tolerance must be a dictionary"
      validate_tolerance(tolerance)
      self.tolerance = tolerance
    else:
      self.tolerance = None

    # Strict schema: reject legacy flat fields
    assert 'rendering' not in data, f"Preset '{self.name}' defines legacy root-level 'rendering'; use 'gltf_test'"
    assert 'models' not in data, f"Preset '{self.name}' defines legacy root-level 'models'; use 'gltf_test'"
    assert 'model_search_paths' not in data, f"Preset '{self.name}' defines legacy root-level 'model_search_paths'; use 'gltf_test'"
    assert 'model_list_file' not in data, f"Preset '{self.name}' defines legacy 'model_list_file' which is no longer supported"

    assert not ('gltf_test' in data and 'sample_test' in data), \
      f"Preset '{self.name}' cannot define both 'gltf_test' and 'sample_test'"

    self.gltf_data = data.get('gltf_test')
    self.sample_data = data.get('sample_test')


class TestConfig:
  def __init__(self, data, presets, default_renderers, base_dir=None):
    assert 'name' in data, "Test must have a 'name'"
    assert _is_string(data['name']), "Test 'name' must be a string"
    self.name = data['name']

    description = data.get('description')
    if description:
      assert _is_string(description), "description must be a string"
    self.description = description

    self.renderers = data.get('renderers', default_renderers)
    assert _is_list_of_strings(self.renderers), f"Test '{self.name}' renderers must be a list of strings"

    self.apply_presets = data.get('apply_presets', [])
    assert _is_list_of_strings(self.apply_presets), f"Test '{self.name}' apply_presets must be a list of strings"

    # Strict schema: reject legacy flat fields
    assert 'rendering' not in data, f"Test '{self.name}' defines legacy root-level 'rendering'; use 'gltf_test'"
    assert 'models' not in data, f"Test '{self.name}' defines legacy root-level 'models'; use 'gltf_test'"
    assert 'model_search_paths' not in data, f"Test '{self.name}' defines legacy root-level 'model_search_paths'; use 'gltf_test'"
    assert 'model_list_file' not in data, f"Test '{self.name}' defines legacy 'model_list_file' which is no longer supported"

    assert ('gltf_test' in data) ^ ('sample_test' in data), \
      f"Test '{self.name}' must define exactly one of 'gltf_test' or 'sample_test'"

    self.is_gltf_test = 'gltf_test' in data
    self.is_sample_test = 'sample_test' in data

    # 1. Process presets in order
    preset_dict = {p.name: p for p in presets}
    preset_tolerance = None

    # gltf inheritance accumulators
    inherited_search_paths = []
    inherited_models = []
    inherited_rendering = {}

    # sample inheritance accumulators
    inherited_warmup_frames = None
    inherited_fixed_timestep = None
    inherited_args = []

    # Note that this needs to applied in order.  Models will be overwritten.
    # Properties will be "added" in order.
    # Tolerance is inherited from the LAST preset that has one defined
    for preset_name in self.apply_presets:
      assert preset_name in preset_dict, f"Used preset '{preset_name}' which is not defined in presets"
      preset = preset_dict[preset_name]

      if preset.tolerance:
        preset_tolerance = preset.tolerance

      if self.is_gltf_test:
        assert preset.sample_data is None, \
          f"Test '{self.name}' is a gltf_test but applied preset '{preset_name}' defines sample_test"
        if preset.gltf_data:
          p_gltf = preset.gltf_data
          if 'model_search_paths' in p_gltf:
            for sp in p_gltf['model_search_paths']:
              if sp not in inherited_search_paths:
                inherited_search_paths.append(sp)
          if 'models' in p_gltf:
            inherited_models = p_gltf['models']
          if 'rendering' in p_gltf:
            inherited_rendering.update(p_gltf['rendering'])
      elif self.is_sample_test:
        assert preset.gltf_data is None, \
          f"Test '{self.name}' is a sample_test but applied preset '{preset_name}' defines gltf_test"
        if preset.sample_data:
          p_sample = preset.sample_data
          if 'warmup_frames' in p_sample:
            inherited_warmup_frames = p_sample['warmup_frames']
          if 'fixed_timestep' in p_sample:
            inherited_fixed_timestep = p_sample['fixed_timestep']
          if 'args' in p_sample:
            inherited_args += p_sample['args']

    # 2. Resolve tolerance (test tolerance overrides preset tolerance)
    tolerance = data.get('tolerance')
    if tolerance:
      assert _is_dict(tolerance), f"Test '{self.name}' tolerance must be a dictionary"
      validate_tolerance(tolerance)
      self.tolerance = tolerance
    else:
      self.tolerance = preset_tolerance

    # 3. Instantiate specific test config
    if self.is_gltf_test:
      self.gltf_test = GltfTestConfig(
        data['gltf_test'],
        inherited_search_paths=inherited_search_paths,
        inherited_models=inherited_models,
        inherited_rendering=inherited_rendering,
        base_dir=base_dir
      )
      self.sample_test = None
    else:
      self.sample_test = SampleTestConfig(
        data['sample_test'],
        inherited_warmup_frames=inherited_warmup_frames,
        inherited_fixed_timestep=inherited_fixed_timestep,
        inherited_args=inherited_args
      )
      self.gltf_test = None

  def to_filament_format(self):
    json_out = {
      'name': self.name,
      'base': self.gltf_test.rendering if self.gltf_test else {}
    }
    return json.dumps(json_out)


class RenderTestConfig:
  def __init__(self, data, base_dir=None):
    assert 'name' in data, "Root config must specify 'name'"
    assert _is_string(data['name']), "Root config 'name' must be a string"
    self.name = data['name']

    # Support 'renderers' (desktop) or 'backends' (android)
    renderers = data.get('renderers', data.get('backends'))
    assert renderers is not None, "Root config must specify 'renderers'"
    assert _is_list_of_strings(renderers), "Root config 'renderers' must be a list of strings"
    self.renderers = renderers

    # Strict schema: reject legacy root fields
    assert 'model_search_paths' not in data, "Root config specifies legacy 'model_search_paths'; move under presets or tests 'gltf_test'"
    assert 'model_list_file' not in data, "Root config specifies legacy 'model_list_file' which is removed"
    assert 'rendering' not in data, "Root config specifies legacy 'rendering'; move under presets or tests 'gltf_test'"

    preset_data = data.get('presets', [])
    self.presets = [
      PresetConfig(p, base_dir=base_dir)
      for p in preset_data
    ]

    assert 'tests' in data, "Root config must specify 'tests'"
    self.tests = [
      TestConfig(
        t,
        presets=self.presets,
        default_renderers=self.renderers,
        base_dir=base_dir
      )
      for t in data['tests']
    ]

    # We cannot have duplicate test names
    test_names = [t.name for t in self.tests]
    assert len(test_names) == len(set(test_names)), f"Duplicate test names detected: {test_names}"


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
    base_dir = path.dirname(path.abspath(config_path))
    return RenderTestConfig(json_txt, base_dir=base_dir)


if __name__ == "__main__":
  parser = ArgParseImpl()
  parser.add_argument('--test', help='Configuration of the test', required=True)

  args, _ = parser.parse_known_args(sys.argv[1:])
  test = parse_from_path(args.test)
