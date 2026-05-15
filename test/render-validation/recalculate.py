# Copyright (C) 2026 The Android Open Source Project
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

import argparse
import json
import os
import zipfile
import io
import re
from pathlib import Path
import numpy as np
from PIL import Image

def parse_json_with_comments(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    # Strip single-line comments
    content = re.sub(r'//.*', '', content)
    return json.loads(content)

def apply_blur(img: np.ndarray, radius: int) -> np.ndarray:
    if radius == 0:
        return img.copy()
    H, W, C = img.shape
    padded = np.pad(img, ((radius, radius), (radius, radius), (0, 0)), mode='edge').astype(np.float32)
    out = np.zeros((H, W, C), dtype=np.float32)
    count = (2 * radius + 1) ** 2
    for dy in range(2 * radius + 1):
        for dx in range(2 * radius + 1):
            out += padded[dy:dy+H, dx:dx+W, :]
    out /= count
    return out

def get_unique_blurs(config, blurs_set):
    if not config:
        return
    mode = config.get("mode", "LEAF")
    if mode == "LEAF":
        blurs_set.add(config.get("blurRadius", 0))
    elif config.get("children"):
        for child in config.get("children"):
            get_unique_blurs(child, blurs_set)

def check_pixel_tree(H, W, r_data, g_data, config, blur_cache_g, blur_cache_r):
    mode = config.get("mode", "LEAF")
    
    if mode == "LEAF":
        shift_rad = config.get("shiftRadius", 0)
        blur_rad = config.get("blurRadius", 0)
        max_allowed_diff = float(config.get("maxAbsDiff", 0.0))
        channel_mask = config.get("channelMask", 15)
        
        g_img = blur_cache_g[blur_rad]
        r_img = blur_cache_r[blur_rad]
        
        active_ch = []
        if channel_mask & 1: active_ch.append(0)
        if channel_mask & 2: active_ch.append(1)
        if channel_mask & 4: active_ch.append(2)
        if channel_mask & 8: active_ch.append(3)
        
        if not active_ch:
            return np.ones((H, W), dtype=bool)
            
        padded_g = np.pad(g_img, ((shift_rad, shift_rad), (shift_rad, shift_rad), (0, 0)), mode='edge')
        pass_leaf = np.zeros((H, W), dtype=bool)
        
        r_img_active = r_img[..., active_ch]
        
        for dy in range(2 * shift_rad + 1):
            for dx in range(2 * shift_rad + 1):
                g_shift = padded_g[dy:dy+H, dx:dx+W, :]
                g_shift_active = g_shift[..., active_ch]
                diff = np.abs(g_shift_active - r_img_active)
                match_shift = np.all(diff <= max_allowed_diff, axis=-1)
                pass_leaf = pass_leaf | match_shift
                
        return pass_leaf
        
    elif mode == "AND":
        pass_node = np.ones((H, W), dtype=bool)
        for child in config.get("children", []):
            pass_node = pass_node & check_pixel_tree(H, W, r_data, g_data, child, blur_cache_g, blur_cache_r)
        return pass_node
        
    elif mode == "OR":
        pass_node = np.zeros((H, W), dtype=bool)
        for child in config.get("children", []):
            pass_node = pass_node | check_pixel_tree(H, W, r_data, g_data, child, blur_cache_g, blur_cache_r)
        return pass_node
        
    return np.zeros((H, W), dtype=bool)

def evaluate_tolerance(rendered_img: Image.Image, golden_img: Image.Image, tolerance: dict) -> bool:
    if rendered_img.size != golden_img.size:
        return False
        
    r_data = (np.array(rendered_img).astype(np.float32) / 255.0)
    g_data = (np.array(golden_img).astype(np.float32) / 255.0)
    
    H, W, C = r_data.shape
    if C != 4 or g_data.shape[2] != 4:
        pass
        
    blurs = set()
    get_unique_blurs(tolerance, blurs)
    
    blur_cache_g = {}
    blur_cache_r = {}
    
    for rad in blurs:
        blur_cache_g[rad] = apply_blur(g_data, rad)
        blur_cache_r[rad] = apply_blur(r_data, rad)
        
    if tolerance:
        pass_tree = check_pixel_tree(H, W, r_data, g_data, tolerance, blur_cache_g, blur_cache_r)
    else:
        # default strict comparison
        diff = np.abs(r_data - g_data)
        pass_tree = np.all(diff == 0, axis=-1)
        
    failing_pixels = np.sum(~pass_tree)
    max_fraction = tolerance.get("maxFailingPixelsFraction", 0.0) if tolerance else 0.0
    
    return failing_pixels <= (max_fraction * H * W)

def get_new_tolerance(heuristics_data, test_name):
    test_id = test_name.split('.')[0] if '.' in test_name else test_name
    for test in heuristics_data.get("tests", []):
        name = test.get("name")
        if name == test_name or name == test_id:
            if "tolerance" in test:
                return test.get("tolerance")
            
            presets_to_apply = test.get("apply_presets", [])
            resolved_tolerance = None
            for preset_name in presets_to_apply:
                for preset in heuristics_data.get("presets", []):
                    if preset.get("name") == preset_name and "tolerance" in preset:
                        resolved_tolerance = preset.get("tolerance")
            
            if resolved_tolerance is not None:
                return resolved_tolerance
                
    return None

def get_original_tolerance(bundle_zip, test_name):
    try:
        config_bytes = bundle_zip.read("default_test/config.json")
        config = parse_json_with_comments_from_str(config_bytes.decode('utf-8'))
        test_id = test_name.split('.')[0] if '.' in test_name else test_name
        for test in config.get("tests", []):
            if test.get("name") == test_id:
                return test.get("tolerance")
    except (KeyError, json.JSONDecodeError, UnicodeDecodeError):
        pass
    return None

def parse_json_with_comments_from_str(content):
    content = re.sub(r'//.*', '', content)
    return json.loads(content)

def is_test_expected(config, full_test_name):
    parts = full_test_name.split('.')
    if not parts: return False
    test_id = parts[0]
    model_name = parts[2] if len(parts) > 2 else None
    
    test_node = None
    for test in config.get("tests", []):
        if test.get("name") == test_id or test.get("name") == full_test_name:
            test_node = test
            break
            
    if not test_node:
        return False
        
    expected_models = set(test_node.get("models", []))
    for preset_name in test_node.get("apply_presets", []):
        for preset in config.get("presets", []):
            if preset.get("name") == preset_name:
                expected_models.update(preset.get("models", []))
                
    if model_name:
        if model_name not in expected_models:
            return False
            
    return True

def process_single_zip(input_zip_path, output_zip_path, heuristics_data):
    with zipfile.ZipFile(input_zip_path, 'r') as zin:
        try:
            results_str = zin.read("results.json")
            results_data = json.loads(results_str)
        except KeyError:
            results_data = None
            
        try:
            bundle_bytes = zin.read("bundle.zip")
            bundle_zip = zipfile.ZipFile(io.BytesIO(bundle_bytes), 'r')
        except KeyError:
            bundle_zip = None

        if results_data and bundle_zip:
            filtered_results = []
            for test_result in results_data.get("results", []):
                test_name = test_result.get("test_name")
                
                # Check if this test case is valid according to the new heuristics config
                if not is_test_expected(heuristics_data, test_name):
                    continue
                
                # 1. Try to get new heuristic
                tolerance = get_new_tolerance(heuristics_data, test_name)
                
                # 2. Fallback to original tolerance from bundle if not provided
                if tolerance is None:
                    tolerance = get_original_tolerance(bundle_zip, test_name)
                    
                # Load rendered image
                rendered_path = f"{test_name}.png"
                try:
                    r_bytes = zin.read(rendered_path)
                    r_img = Image.open(io.BytesIO(r_bytes)).convert("RGBA")
                except KeyError:
                    test_result["passed"] = False
                    continue
                    
                # Load golden image
                golden_path = f"default_test/goldens/{test_name}.png"
                try:
                    g_bytes = bundle_zip.read(golden_path)
                    g_img = Image.open(io.BytesIO(g_bytes)).convert("RGBA")
                except KeyError:
                    test_result["passed"] = False
                    continue
                    
                # Evaluate
                passed = bool(evaluate_tolerance(r_img, g_img, tolerance))
                test_result["passed"] = passed
                filtered_results.append(test_result)
                
            results_data["results"] = filtered_results

        # Write output zip
        with zipfile.ZipFile(output_zip_path, 'w', compression=zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                if item.filename == "results.json" and results_data:
                    zout.writestr(item, json.dumps(results_data, indent=2))
                else:
                    zout.writestr(item, zin.read(item.filename))

def main():
    parser = argparse.ArgumentParser(description="Recalculate render validation results based on new heuristics.")
    parser.add_argument("input_dir", help="Directory containing input .zip result files (INPUT A)")
    parser.add_argument("heuristics_json", help="JSON file containing new test definitions/tolerances")
    parser.add_argument("output_dir", help="Directory to output the new .zip result files (OUTPUT B)")
    args = parser.parse_args()

    input_dir = Path(args.input_dir)
    output_dir = Path(args.output_dir)
    heuristics_path = Path(args.heuristics_json)
    
    if not input_dir.exists():
        print(f"Error: Input directory {input_dir} does not exist.")
        return
        
    if not heuristics_path.exists():
        print(f"Error: Heuristics file {heuristics_path} does not exist.")
        return
        
    try:
        heuristics_data = parse_json_with_comments(heuristics_path)
    except json.JSONDecodeError as e:
        print(f"Error parsing heuristics JSON: {e}")
        return

    os.makedirs(output_dir, exist_ok=True)
    
    for zip_file in input_dir.glob("*.zip"):
        print(f"Processing {zip_file.name}...")
        out_zip_path = output_dir / zip_file.name
        process_single_zip(zip_file, out_zip_path, heuristics_data)
        
    print(f"Done. Recalculated results saved to {output_dir}")

if __name__ == "__main__":
    main()
