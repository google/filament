
import os
import urllib.request
import ssl
from PIL import Image
import argparse

# URL of the Milky Way texture (Gaia EDR3) from ESA
# Low Res PNG (2.71 MB) is sufficient for our 1024x512 target
URL = "https://www.esa.int/var/esa/storage/images/esa_multimedia/images/2020/12/the_colour_of_the_sky_from_gaia_s_early_data_release_3/22358049-1-eng-GB/The_colour_of_the_sky_from_Gaia_s_Early_Data_Release_3.png"
OUTPUT_DIR = "assets"
OUTPUT_FILENAME = "milkyway.png"
TARGET_WIDTH = 1024
TARGET_HEIGHT = 512

def main():
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    output_path = os.path.join(OUTPUT_DIR, OUTPUT_FILENAME)
    temp_path = os.path.join(OUTPUT_DIR, "temp_milkyway.png")

    print(f"Downloading Milky Way texture from {URL}...")
    
    # Bypass SSL verification globally
    if hasattr(ssl, '_create_unverified_context'):
        ssl._create_default_https_context = ssl._create_unverified_context

    try:
        req = urllib.request.Request(URL, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response, open(temp_path, 'wb') as out_file:
            out_file.write(response.read())
    except Exception as e:
        print(f"Failed to download: {e}")
        return

    if not os.path.exists(temp_path):
        print("Error: Download failed.")
        return

    print("Processing image...")
    with Image.open(temp_path) as img:
        # Convert to RGB (remove alpha if present, though this is likely opaque)
        img = img.convert("RGB")
        
        # Resize to user requested dimensions
        print(f"Resizing to {TARGET_WIDTH}x{TARGET_HEIGHT}...")
        img = img.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.Resampling.LANCZOS)
        
        # Save
        print(f"Saving to {output_path}...")
        img.save(output_path, "PNG")

    # Cleanup
    if os.path.exists(temp_path):
        os.remove(temp_path)

    print("Done!")

if __name__ == "__main__":
    main()
