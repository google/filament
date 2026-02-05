# Result: /Users/mathias/sources/git/filament/out/cmake-release/tools/matc/matc
MATC="../../../../out/cmake-release/tools/matc/matc"

# Navigate to script directory to ensure relative paths work
cd "$(dirname "$0")"

set -e

$MATC -a opengl -p mobile -o assets/simulated_skybox.filamat simulated_skybox.mat
echo "Material recompiled to assets/simulated_skybox.filamat"
