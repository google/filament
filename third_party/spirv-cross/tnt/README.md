## Updating

To update to the spirv-cross that's currently on GitHub master, do the following.

```
cd third_party
curl -L https://github.com/KhronosGroup/SPIRV-Cross/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Cross-master/ spirv-cross/ --delete
git checkout spirv-cross/tnt/*
rm -rf SPIRV-Cross-master master.zip
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

To see all changes, run the following diff command from the `third_party/spirv-cross` directory:

```
diff CMakeLists.txt tnt/CMakeLists.txt
```
