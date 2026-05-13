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
import shutil
import zipfile
from pathlib import Path
from PIL import Image

def get_test_tolerance(config, full_test_name):
    # test name is e.g. "basic.opengl.DamagedHelmet"
    parts = full_test_name.split('.')
    if not parts: return None
    test_id = parts[0]
    
    for test in config.get("tests", []):
        if test.get("name") == test_id:
            return test.get("tolerance")
    return None

def process_zip(zip_path, output_dir, device_name):
    """Extract and process a single zip file from the results folder."""
    extract_dir = os.path.join(output_dir, "tmp", device_name)
    os.makedirs(extract_dir, exist_ok=True)
    
    with zipfile.ZipFile(zip_path, 'r') as z:
        z.extractall(extract_dir)
        
    results_json_path = os.path.join(extract_dir, "results.json")
    if not os.path.exists(results_json_path):
        return {}, []
        
    with open(results_json_path, 'r') as f:
        try:
            device_results = json.load(f)
        except json.JSONDecodeError:
            return {}, []

    metadata = device_results['metadata']

    bundle_zip_path = os.path.join(extract_dir, "bundle.zip")
    bundle_dir = os.path.join(extract_dir, "bundle")
    
    if os.path.exists(bundle_zip_path):
        os.makedirs(bundle_dir, exist_ok=True)
        with zipfile.ZipFile(bundle_zip_path, 'r') as z:
            z.extractall(bundle_dir)
            
    config_path = os.path.join(bundle_dir, "default_test", "config.json")
    config = {}
    if os.path.exists(config_path):
        with open(config_path, 'r') as f:
            config = json.load(f)
            
    run_results = []
    
    for test_result in device_results.get("results", []):
        test_name = test_result.get("test_name")
        passed = test_result.get("passed", False)
        
        rendered_path = os.path.join(extract_dir, f"{test_name}.png")
        golden_path = os.path.join(bundle_dir, "default_test", "goldens", f"{test_name}.png")
        
        if not os.path.exists(rendered_path):
            continue
            
        rendered_img = Image.open(rendered_path).convert("RGBA")
        
        # Output paths for web
        rel_test_dir = f"{device_name}/{test_name}"
        out_test_dir = os.path.join(output_dir, "assets", rel_test_dir)
        os.makedirs(out_test_dir, exist_ok=True)
        
        out_golden = os.path.join(out_test_dir, "golden.png")
        out_rendered = os.path.join(out_test_dir, "rendered.png")
        out_thumb = os.path.join(out_test_dir, "thumb.png")
        
        rendered_img.save(out_rendered)
        
        # Generate thumbnail
        thumb_size = (128, 128)
        thumb_img = rendered_img.copy()
        thumb_img.thumbnail(thumb_size)
        thumb_img.save(out_thumb)
        
        has_golden = False
        if os.path.exists(golden_path):
            has_golden = True
            shutil.copy2(golden_path, out_golden)
        else:
            Image.new("RGBA", rendered_img.size, (0,0,0,0)).save(out_golden)
            
        tolerance_config = get_test_tolerance(config, test_name)
            
        run_results.append({
            "testName": test_name,
            "passed": passed,
            "golden": f"assets/{rel_test_dir}/golden.png",
            "rendered": f"assets/{rel_test_dir}/rendered.png",
            "thumb": f"assets/{rel_test_dir}/thumb.png",
            "hasGolden": has_golden,
            "config": tolerance_config
        })
            
    return [metadata, run_results]

def main():
    parser = argparse.ArgumentParser(description="Process render validation zip results.")
    parser.add_argument("input_dir", help="Directory containing .zip result files")
    parser.add_argument("output_dir", help="Directory to output the static web viewer")
    args = parser.parse_args()

    input_dir = Path(args.input_dir)
    output_dir = Path(args.output_dir)
    
    if not input_dir.exists():
        print(f"Error: Input directory {input_dir} does not exist.")
        return
        
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(output_dir / "assets", exist_ok=True)
    
    all_results = []
    
    for zip_file in input_dir.glob("*.zip"):
        device_name = zip_file.stem
        print(f"Processing {zip_file.name} for device {device_name}...")
        
        metadata, device_results = process_zip(zip_file, output_dir, device_name)
        if device_results:
            all_results.append({
                "metadata": metadata,
                "device": device_name,
                "runs": device_results
            })
            
    # Cleanup tmp
    tmp_dir = output_dir / "tmp"
    if tmp_dir.exists():
        shutil.rmtree(tmp_dir)
        
    # Write data.json
    with open(output_dir / "data.json", "w") as f:
        json.dump(all_results, f, indent=2)
        
    # Copy web viewer files
    viewer_src = Path(__file__).parent / "result-viewer"
    if viewer_src.exists():
        for item in viewer_src.iterdir():
            if item.is_file():
                shutil.copy2(item, output_dir)
            elif item.is_dir():
                dest_dir = output_dir / item.name
                if dest_dir.exists():
                    shutil.rmtree(dest_dir)
                shutil.copytree(item, dest_dir)
                
    print("Done. To view results, run a static server in the output directory:")
    print(f"cd {output_dir} && python3 -m http.server 1234")

if __name__ == "__main__":
    main()
