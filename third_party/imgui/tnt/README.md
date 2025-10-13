To update the version of imgui used in this project, run the `update_imgui.sh` script.

The script is located in `third_party/imgui/tnt`.

From the root of the repository, you can run it like this:

**Usage:**
```shell
./third_party/imgui/tnt/update_imgui.sh <version>
```

For example, to update to version 1.90.9:
```shell
./third_party/imgui/tnt/update_imgui.sh 1.90.9
```

- If necessary, add or remove source files from `third_party/imgui/tnt/CMakeLists.txt`.
- Compile and test Filament.

You can find the latest version number on the imgui releases page:
[https://github.com/ocornut/imgui/releases](https://github.com/ocornut/imgui/releases)
