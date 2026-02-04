#!/bin/bash
set -e

# Default size
SIZE=${1:-256}
OUTPUT_COLOR="../assets/moon_disk.png"
OUTPUT_NORMAL="../assets/moon_normal.png"

# Navigate to script directory
cd "$(dirname "$0")"

# Check dependencies
if ! python3 -c "import numpy, PIL" 2>/dev/null; then
    echo "Installing dependencies (numpy, Pillow)..."
    pip3 install numpy Pillow
fi

echo "Generating Moon Asset (Size: ${SIZE}x${SIZE})..."
python3 process_moon.py --size $SIZE --supersample 4 --blur 1.0 --output-color $OUTPUT_COLOR --output-normal $OUTPUT_NORMAL

echo "Generated $OUTPUT_COLOR and $OUTPUT_NORMAL"
