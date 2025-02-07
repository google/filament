## Updating
To update to the spire-cross of a specific chromium commit, do the following.
```
cd third_party
curl -L https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross/+archive/b8fcf307.tar.gz > spirv-cross-src.tar.gz
mkdir spirv-cross-new
tar -xzf spirv-cross-src.tar.gz -C spirv-cross-new
rsync -r spirv-cross-new/ spirv-cross/ --delete
git restore spirv-cross/tnt
patch -p2 < spirv-cross/tnt/0001-convert-floats-to-their-smallest-string-representati.patch
patch -p2 < spirv-cross/tnt/0002-localeconv-api-level-check.patch
rm -rf spirv-cross-new spirv-cross-src.tar.gz
git add spirv-cross
```


To update to the spirv-cross that's currently on GitHub master, do the following.

```
cd third_party
curl -L https://github.com/KhronosGroup/SPIRV-Cross/archive/main.zip > main.zip
unzip main.zip
rsync -r SPIRV-Cross-main/ spirv-cross/ --delete
git restore spirv-cross/tnt
patch -p2 < spirv-cross/tnt/0001-convert-floats-to-their-smallest-string-representati.patch
patch -p2 < spirv-cross/tnt/0002-localeconv-api-level-check.patch
rm -rf SPIRV-Cross-main main.zip
git add spirv-cross
```

Please be sure to test Filament before uploading your CL.

## Filament-specific changes to CMakeLists.txt

The Filament-specific `CMakeLists.txt` under the `tnt` directory has the following changes made from
`spirv-cross`'s provided `CMakeLists.txt`:
- Exceptions turned off in favor of assertions
- Removal of installation rules
- Removal of unused `spirv-cross` libraries
- Removal of the `spirv-cross` executable
- Removal of `spirv-cross` test cases
- Added `convert_to_smallest_string` in `spirv_common.hpp` and used for float conversion

To see all changes, run the following diff command from the `third_party/spirv-cross` directory:

```
diff CMakeLists.txt tnt/CMakeLists.txt
```
