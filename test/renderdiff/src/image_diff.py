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

def same_image(tiff_file_a, tiff_file_b):
  try:
    img1_data = tifffile.imread(tiff_file_a)
    img2_data = tifffile.imread(tiff_file_b)

    # If the dimensions (height, width, number of channels, number of pages/frames)
    # are different, the images are not the same.
    if img1_data.shape != img2_data.shape:
      print(f"Images have different shapes: {img1_data.shape} vs {img2_data.shape}")
      return False

    # numpy.array_equal() checks if two arrays have the same shape and elements.
    if np.array_equal(img1_data, img2_data):
      return True
    else:
      return False

  except FileNotFoundError:
    print(f"Error: One or both files not found ('{file_path1}', '{file_path2}').")
    return False
  except tifffile.TiffFileError as e:
    print(f"Error: One or both files are not valid TIFF files or could not be read. Details: {e}")
    return False
  except Exception as e:
    print(f"An unexpected error occurred: {e}")
    return False

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
