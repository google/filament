## Updating

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
