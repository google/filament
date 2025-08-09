To update the version of SPIRV-Headers used in this project, run the `update_spirv-headers.sh` script.

The script is located in `third_party/spirv-headers/tnt`.

From the root of the repository, you can run it like this:

**Usage:**
```shell
./third_party/spirv-headers/tnt/update_spirv-headers.sh <version_tag>
```
or
```shell
./third_party/spirv-headers/tnt/update_spirv-headers.sh --sha <commit_sha>
```

To update to a specific commit:
```shell
./third_party/spirv-headers/tnt/update_spirv-headers.sh --sha 2a611a9
```

You can find the latest SHA on the SPIRV-Headers tags page:
[https://github.com/KhronosGroup/SPIRV-Headers/releases](https://github.com/KhronosGroup/SPIRV-Headers/tags)
