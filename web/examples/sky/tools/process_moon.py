import argparse
import urllib.request
import os
import sys
import math
import ssl
from PIL import Image

# Ensure numpy is available
try:
    import numpy as np
except ImportError:
    print("Error: numpy is required. Please install it with: pip install numpy")
    sys.exit(1)

# Default URLs
COLOR_URL = "https://svs.gsfc.nasa.gov/vis/a000000/a004700/a004720/lroc_color_2k.jpg"
DISP_URL = "https://svs.gsfc.nasa.gov/vis/a000000/a004700/a004720/ldem_4.tif"

SRC_COLOR_FILENAME = "lroc_color_2k.jpg"
SRC_DISP_FILENAME = "ldem_4.tif"

def download_file(url, filename):
    if os.path.exists(filename):
        print(f"File {filename} already exists. Skipping download.")
        return

    print(f"Downloading {url} to {filename}...")
    # Create unverified context to avoid potential SSL cert issues
    context = ssl._create_unverified_context()
    try:
        with urllib.request.urlopen(url, context=context) as response, open(filename, 'wb') as out_file:
            data = response.read()
            out_file.write(data)
        print(f"Download of {filename} complete.")
    except Exception as e:
        print(f"Error downloading file {filename}: {e}")
        # Don't exit hard if it's just one file, maybe?
        # But for this script, we likely need it.
        sys.exit(1)

def equirectangular_to_orthographic(src_img, size, mode=None):
    """
    Reprojects an equirectangular image to an orthographic projection (sphere view).
    src_img: PIL Image (Equirectangular)
    size: Output size (width, height) - usually square
    """
    print(f"Reprojecting to {size}x{size} Orthographic Disk...")
    
    width, height = size, size
    src_w, src_h = src_img.size
    
    # Create coordinate grid centered at 0,0 (-1 to 1)
    y, x = np.mgrid[size/2:-size/2:-1, -size/2:size/2] # Note: y goes high to low
    
    # Normalize to -1..1
    x = x / (size / 2)
    y = y / (size / 2)
    
    # Mask for points outside the circle
    r2 = x*x + y*y
    mask = r2 <= 1.0
    
    # Calculate sphere coordinates (z > 0 for front face)
    z = np.zeros_like(r2)
    z[mask] = np.sqrt(1.0 - r2[mask])
    
    # Vector P = (x, y, z) on unit sphere
    # Lat = asin(y)
    # Lon = atan2(x, z) 
    
    # Apply mask to avoid invalid calculations
    lat = np.arcsin(y * mask)
    lon = np.arctan2(x * mask, z * mask)
    
    # Map to UV [0, 1]
    u = (lon / (2 * math.pi)) + 0.5
    v = (lat / math.pi) + 0.5
    
    # Map to Source Pixels
    u = np.clip(u, 0, 1)
    v = np.clip(v, 0, 1)
    
    src_x = (u * (src_w - 1)).astype(np.int32)
    src_y = ((1.0 - v) * (src_h - 1)).astype(np.int32) # Flip V for image coords
    
    # Sample pixels
    src_array = np.array(src_img)
    
    # Handle dimensions
    if len(src_array.shape) == 2:
        # Grayscale / Single channel
        out_channels = 1
        src_array = src_array[:, :, np.newaxis] # Expand for consistent indexing
    else:
        out_channels = src_array.shape[2]
        
    out_array = np.zeros((height, width, out_channels), dtype=src_array.dtype)
    
    # Advanced indexing
    valid_y, valid_x = np.where(mask)
    
    # Extract coordinates for valid pixels
    sx = src_x[valid_y, valid_x]
    sy = src_y[valid_y, valid_x]
    
    out_array[valid_y, valid_x] = src_array[sy, sx]
    
    # Squeeze if single channel
    if out_channels == 1:
        out_array = out_array.squeeze(axis=2)
        
    return Image.fromarray(out_array, mode or src_img.mode)

def compute_normal_map(height_img, scale=1.0, blur_radius=0.0):
    print("Computing Normal Map from Height Map...")
    # Convert to float array
    h = np.array(height_img).astype(np.float32)

    # Apply Blur if requested
    if blur_radius > 0:
        try:
            from scipy.ndimage import gaussian_filter
            print(f"Applying Gaussian Blur (Radius: {blur_radius})...")
            h = gaussian_filter(h, sigma=blur_radius)
        except ImportError:
            print("Warning: scipy not found. Skipping Gaussian Blur.")
    
    # Normalize height to 0..1 for consistent gradient scale regardless of input depth
    h_min, h_max = h.min(), h.max()
    print(f"Height Map Range: {h_min} to {h_max}")
    if h_max > h_min:
        h_norm = (h - h_min) / (h_max - h_min)
    else:
        h_norm = h
    
    # Gradients
    dy, dx = np.gradient(h_norm)
    
    # Pre-emphasis scale
    bump_scale = scale 
    
    # Normal vector components
    # Map is Top-Down Y.
    nx = -dx * bump_scale
    ny = -dy * bump_scale
    nz = np.ones_like(nx) 
    
    # Mask out normals where r > 0.96 (avoid edge cliff artifacts)
    rows, cols = h.shape
    y, x = np.ogrid[:rows, :cols]
    center_y, center_x = rows/2.0, cols/2.0
    # max radius is size/2
    radius_sq = (min(rows, cols) / 2.0)**2
    dist_sq = (x - center_x)**2 + (y - center_y)**2
    mask = dist_sq < (radius_sq * 0.96 * 0.96)

    nx[~mask] = 0
    ny[~mask] = 0
    nz[~mask] = 1
    
    # Normalize
    len_n = np.sqrt(nx*nx + ny*ny + nz*nz)
    # Avoid divide by zero
    len_n[len_n == 0] = 1.0
    
    nx /= len_n
    ny /= len_n
    nz /= len_n
    
    # Pack to 0..255
    # [-1, 1] -> [0, 255]
    out_x = ((nx + 1.0) * 0.5 * 255.0).astype(np.uint8)
    out_y = ((ny + 1.0) * 0.5 * 255.0).astype(np.uint8)
    out_z = ((nz + 1.0) * 0.5 * 255.0).astype(np.uint8)
    
    out_rgb = np.dstack((out_x, out_y, out_z))
    return Image.fromarray(out_rgb, 'RGB')

def main():
    parser = argparse.ArgumentParser(description='Process Moon Texture')
    parser.add_argument('--size', type=int, default=256, help='Output resolution (square)')
    parser.add_argument('--supersample', type=int, default=2, help='Internal processing resolution multiplier')
    parser.add_argument('--blur', type=float, default=1.0, help='Gaussian blur radius for height map')
    parser.add_argument('--bump-scale', type=float, default=60.0, help='Normal map bump scale')
    parser.add_argument('--output-color', type=str, default='assets/moon_disk.png', help='Output color filename')
    parser.add_argument('--output-normal', type=str, default='assets/moon_normal.png', help='Output normal filename')
    parser.add_argument('--skip-download', action='store_true', help='Skip downloading files')
    
    args = parser.parse_args()
    
    internal_size = args.size * args.supersample
    
    # Ensure assets dir exists
    os.makedirs(os.path.dirname(args.output_color) or '.', exist_ok=True)
    
    # 1. Download
    if not args.skip_download:
        download_file(COLOR_URL, SRC_COLOR_FILENAME)
        download_file(DISP_URL, SRC_DISP_FILENAME)
    
    # 2. Process Color
    print(f"Processing Color Map (Internal Size: {internal_size}x{internal_size})...")
    try:
        img_color = Image.open(SRC_COLOR_FILENAME).convert('RGB')
        out_color = equirectangular_to_orthographic(img_color, internal_size)
        
        if args.supersample > 1:
            print(f"Downsampling Color to {args.size}x{args.size}...")
            out_color = out_color.resize((args.size, args.size), Image.LANCZOS)
            
        out_color.save(args.output_color)
        print(f"Saved {args.output_color}")
    except Exception as e:
        print(f"Error processing color: {e}")

    # 3. Process Normal
    print(f"Processing Displacement Map (Internal Size: {internal_size}x{internal_size})...")
    try:
        from PIL import ImageFile
        ImageFile.LOAD_TRUNCATED_IMAGES = True
        
        # Check if file exists
        if not os.path.exists(SRC_DISP_FILENAME):
            print(f"Displacement map {SRC_DISP_FILENAME} not found!")
            return

        img_disp = Image.open(SRC_DISP_FILENAME) 
        out_disp = equirectangular_to_orthographic(img_disp, internal_size)
        
        # Compute Normals
        out_normal = compute_normal_map(out_disp, scale=args.bump_scale, blur_radius=args.blur)
        
        if args.supersample > 1:
            print(f"Downsampling Normal to {args.size}x{args.size}...")
            out_normal = out_normal.resize((args.size, args.size), Image.LANCZOS)

        out_normal.save(args.output_normal)
        print(f"Saved {args.output_normal}")
        
    except Exception as e:
        print(f"Error processing normal: {e}")
        import traceback
        traceback.print_exc()

    print("Done.")

if __name__ == "__main__":
    main()
