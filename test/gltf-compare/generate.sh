#!/bin/bash

# Directory containing the GLTF files
GLTF_DIR="path/to/your/gltf/files"
GLTF_VIEWER_PATH="out/cmake-debug/samples/gltf_viewer"

GLTF_OUT_DIR="`dirname $0`/generated"

echo "The dirname is $GLTF_OUT_DIR"
# echo `dirname $0`
rm -rf $GLTF_OUT_DIR
mkdir -p $GLTF_OUT_DIR
# exit 0

# Base directory for the glTF-Sample-Assets repository
GLTF_SAMPLES_DIR="../glTF-Sample-Assets/Models"

# Loop through each subdirectory in the base directory
find "$GLTF_SAMPLES_DIR" -type f -name "*.gltf" | while read -r gltf_file; do
    # Get the base name of the file (e.g., "Box")
    base_name=$(basename "$gltf_file" .gltf)

    echo "Processing $gltf_file..."

    # Run the gltf_viewer command
    # You may need to adjust this path based on your project structure
    ./out/cmake-debug/samples/gltf_viewer --api vulkan --batch=test/gltf-compare/test.json --headless "$gltf_file"

    # Rename the generated .js and .tif files
    # The generated files are assumed to be in the same directory where the script is run

    mv "Base0.tif" "${base_name}.tif"

    # Move the newly renamed files to the Output directory

    mv "${base_name}.tif" "$GLTF_OUT_DIR/"

    echo "Generated files moved to $GLTF_OUT_DIR"   
    echo "---"
done

echo "Script finished."