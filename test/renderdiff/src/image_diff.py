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

import tifffile
import numpy as np

def evaluate_tolerance_criteria(img1_data, img2_data, tolerance_spec):
  """
  Recursively evaluate tolerance criteria with AND/OR logic.

  Args:
    img1_data: First image array
    img2_data: Second image array
    tolerance_spec: Dictionary containing tolerance criteria with structure:
      {
        "operator": "AND" | "OR",  # How to combine criteria results
        "criteria": [...]          # List of criteria or nested tolerance specs
      }
      OR for leaf criteria:
      {
        "max_pixel_diff": int,           # Max absolute difference per channel (0-255)
        "max_pixel_diff_percent": float, # Max difference as percentage (0-100%)
        "allowed_diff_pixels": float     # Percentage of pixels allowed to exceed (0-100%)
      }

  Returns:
    tuple: (bool: pass/fail, dict: detailed statistics)
  """

  if 'criteria' not in tolerance_spec:
    # Leaf criteria - evaluate single condition
    return evaluate_single_criteria(img1_data, img2_data, tolerance_spec)

  operator = tolerance_spec.get('operator', 'AND').upper()
  criteria_list = tolerance_spec['criteria']

  results_and_stats = [evaluate_tolerance_criteria(img1_data, img2_data, criteria)
                       for criteria in criteria_list]

  results = [r[0] for r in results_and_stats]  # Extract pass/fail results
  all_stats = [r[1] for r in results_and_stats]  # Extract statistics

  if operator == 'AND':
    final_result = all(results)
  elif operator == 'OR':
    final_result = any(results)
  else:
    raise ValueError(f"Unknown operator: {operator}")

  # Combine statistics
  combined_stats = {
    'operator': operator,
    'criteria_results': all_stats,
    'passed': bool(final_result)
  }

  return final_result, combined_stats

def evaluate_single_criteria(img1_data, img2_data, criteria):
  """
  Evaluate a single tolerance criteria.

  Args:
    img1_data: First image array
    img2_data: Second image array
    criteria: Dictionary with tolerance parameters:
      - max_pixel_diff: Maximum absolute difference per channel (0-255 range)
      - max_pixel_diff_percent: Maximum difference as percentage (0-100%)
      - allowed_diff_pixels: Percentage of pixels allowed to exceed tolerance (0-100%)

  Returns:
    tuple: (bool: pass/fail, dict: detailed statistics)
  """
  diff_abs = np.abs(img1_data.astype(np.int16) - img2_data.astype(np.int16))

  max_diff = criteria.get('max_pixel_diff', float('inf'))
  max_diff_percent = criteria.get('max_pixel_diff_percent', float('inf'))
  allowed_diff_pixels = criteria.get('allowed_diff_pixels', 0.0)

  # Calculate which pixels exceed absolute threshold
  exceeds_abs = diff_abs > max_diff if max_diff < float('inf') else np.zeros_like(diff_abs, dtype=bool)

  # Calculate which pixels exceed percentage threshold
  if max_diff_percent < float('inf'):
    max_val = np.maximum(img1_data, img2_data)
    # Avoid division by zero
    diff_percent = np.divide(diff_abs * 100.0, np.maximum(max_val, 1),
                            out=np.zeros_like(diff_abs, dtype=np.float32),
                            where=max_val!=0)
    exceeds_percent = diff_percent > max_diff_percent
  else:
    exceeds_percent = np.zeros_like(diff_abs, dtype=bool)

  # A pixel fails if it exceeds either threshold (OR logic at pixel level)
  exceeds_tolerance = exceeds_abs | exceeds_percent

  # Check per-pixel: pixel fails if ANY channel exceeds tolerance
  exceeds_per_pixel = np.any(exceeds_tolerance, axis=-1) if len(exceeds_tolerance.shape) > 2 else exceeds_tolerance

  # Calculate detailed statistics
  total_pixels = exceeds_per_pixel.size
  failing_pixels = np.sum(exceeds_per_pixel)
  failing_percentage = (failing_pixels / total_pixels) * 100.0

  # Distribution analysis
  max_abs_diff = np.max(diff_abs) if diff_abs.size > 0 else 0
  mean_abs_diff = np.mean(diff_abs) if diff_abs.size > 0 else 0

  # Per-channel max differences
  if len(diff_abs.shape) > 2:
    max_diff_per_channel = [np.max(diff_abs[:, :, c]) for c in range(diff_abs.shape[2])]
  else:
    max_diff_per_channel = [max_abs_diff]

  stats = {
    'criteria': criteria,
    'total_pixels': int(total_pixels),
    'failing_pixels': int(failing_pixels),
    'failing_percentage': float(failing_percentage),
    'allowed_percentage': float(allowed_diff_pixels),
    'max_abs_diff': int(max_abs_diff),
    'mean_abs_diff': float(mean_abs_diff),
    'max_diff_per_channel': [int(x) for x in max_diff_per_channel],
    'passed': bool(failing_percentage <= allowed_diff_pixels)
  }

  return failing_percentage <= allowed_diff_pixels, stats

def same_image(tiff_file_a, tiff_file_b, tolerance=None):
  """
  Compare two TIFF images for equality with optional tolerance.

  Args:
    tiff_file_a: Path to first TIFF file
    tiff_file_b: Path to second TIFF file
    tolerance: Optional tolerance specification dictionary. If None, performs exact comparison.
               Can be either:
               1. Single criteria: {"max_pixel_diff": 5, "allowed_diff_pixels": 1.0}
               2. Nested criteria: {"operator": "OR", "criteria": [...]}

  Returns:
    tuple: (bool: pass/fail, dict: detailed statistics or None)
           For exact comparison, returns (bool, None)
  """
  try:
    img1_data = tifffile.imread(tiff_file_a)
    img2_data = tifffile.imread(tiff_file_b)

    # If the dimensions (height, width, number of channels, number of pages/frames)
    # are different, the images are not the same.
    if img1_data.shape != img2_data.shape:
      print(f"Images have different shapes: {img1_data.shape} vs {img2_data.shape}")
      return False, {'error': 'Shape mismatch', 'shape1': img1_data.shape, 'shape2': img2_data.shape}

    # If no tolerance specified, use exact comparison
    if tolerance is None:
      exact_match = np.array_equal(img1_data, img2_data)
      return exact_match, None

    # Use tolerance-based comparison
    return evaluate_tolerance_criteria(img1_data, img2_data, tolerance)

  except FileNotFoundError:
    print(f"Error: One or both files not found ('{tiff_file_a}', '{tiff_file_b}').")
    return False, {'error': 'File not found'}
  except tifffile.TiffFileError as e:
    print(f"Error: One or both files are not valid TIFF files or could not be read. Details: {e}")
    return False, {'error': 'Invalid TIFF file', 'details': str(e)}
  except Exception as e:
    print(f"An unexpected error occurred: {e}")
    return False, {'error': 'Unexpected error', 'details': str(e)}

def output_image_diff(tiff_file_a, tiff_file_b, output_path):
  try:
    img1_data = tifffile.imread(tiff_file_a)
    img2_data = tifffile.imread(tiff_file_b)

    # If the dimensions (height, width, number of channels, number of pages/frames)
    # are different, the images are not the same.
    if img1_data.shape != img2_data.shape:
      raise RuntimeError(f"Images {tiff_file_a} and {tiff_file_b} have different shapes: {img1_data.shape} vs {img2_data.shape}")

    diff_img = np.abs(img1_data.astype(np.int16) - img2_data.astype(np.int16))
    diff_img = diff_img.astype(np.uint8)
    tifffile.imwrite(output_path, diff_img)

  except FileNotFoundError:
    print(f"Error: One or both files not found ('{file_path1}', '{file_path2}').")
    return False
  except tifffile.TiffFileError as e:
    print(f"Error: One or both files are not valid TIFF files or could not be read. Details: {e}")
    return False
  except Exception as e:
    print(f"An unexpected error occurred: {e}")
    return False
