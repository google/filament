To update the version of Draco used in this project, run the `update_draco.sh` script.

The script is located in `third_party/draco/tnt`.

From the root of the repository, you can run it like this:

**Usage:**
```shell
./third_party/draco/tnt/update_draco.sh <version_tag>
```

For example, to update to a specific release:
```shell
./third_party/draco/tnt/update_draco.sh 1.5.7
```

You can find the latest version number on the Draco releases page:
[https://github.com/google/draco/releases](https://github.com/google/draco/releases)

Please be sure to test Filament before uploading your CL.

## Filament-specific changes

The following directories are excluded from the upstream `draco` repository:
- `testdata`
- `unity`
- `maya`
