To update the version of SPIRV-Tools used in this project, run the `update_spirv-tools.sh` script.

The script is located in `third_party/spirv-tools/tnt`.

From the root of the repository, you can run it like this:

**Usage:**
```shell
./third_party/spirv-tools/tnt/update_spirv-tools.sh <version_tag>
```
or
```shell
./third_party/spirv-tools/tnt/update_spirv-tools.sh --sha <commit_sha>
```

For example, to update to version `2023.4`:
```shell
./third_party/spirv-tools/tnt/update_spirv-tools.sh 2023.4
```

To update to a specific commit:
```shell
./third_party/spirv-tools/tnt/update_spirv-tools.sh --sha a1b2c3d4
```

You can find the latest version number on the SPIRV-Tools releases page:
[https://github.com/KhronosGroup/SPIRV-Tools/releases](https://github.com/KhronosGroup/SPIRV-Tools/releases)
