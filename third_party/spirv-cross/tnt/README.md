To update the version of SPIRV-Cross used in this project, run the `update_spirv-cross.sh` script.

The script is located in `third_party/spirv-cross/tnt`.

From the root of the repository, you can run it like this:

**Usage:**
```shell
./third_party/spirv-cross/tnt/update_spirv-cross.sh <version_tag>
```
or
```shell
./third_party/spirv-cross/tnt/update_spirv-cross.sh --sha <commit_sha>
```

For example, to update to a specific release:
```shell
./third_party/spirv-cross/tnt/update_spirv-cross.sh 2023-08-23
```

To update to a specific commit:
```shell
./third_party/spirv-cross/tnt/update_spirv-cross.sh --sha a1b2c3d4
```

You can find the latest version number on the SPIRV-Cross releases page:
[https://github.com/KhronosGroup/SPIRV-Cross/releases](https://github.com/KhronosGroup/SPIRV-Cross/releases)

The script will automatically apply the following patches:
- `0001-convert-floats-to-their-smallest-string-representati.patch`
- `0002-localeconv-api-level-check.patch`

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
