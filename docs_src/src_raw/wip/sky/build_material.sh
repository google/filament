# Result: /Users/mathias/sources/git/filament/out/release/filament/bin/matc
MATC="../../../out/release/filament/bin/matc"

# Navigate to script directory to ensure relative paths work
cd "$(dirname "$0")"

$MATC -a opengl -p mobile -o assets/simulated_skybox.filamat simulated_skybox.mat
echo "Material recompiled to assets/simulated_skybox.filamat"
