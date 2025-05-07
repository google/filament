import tifffile
import numpy

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
    if numpy.array_equal(img1_data, img2_data):
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
