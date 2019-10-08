## Updating

To update to the SPIRV-Tools that's currently on GitHub master, do the following from the
`third_party` directory.

### Update SPIRV-Tools

```
curl -L https://github.com/KhronosGroup/SPIRV-Tools/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Tools-master/ spirv-tools/ --delete
git checkout spirv-tools/tnt/
rm -rf SPIRV-Tools-master master.zip
```

### Make Filament-specific changes

Make sure to preserve all the changes marked with the following in `spirv-tools/CMakeLists.txt` and
`spirv-toools/source/opt/CMakeLists.txt`:

`# Filament specific changes`

### Update SPIRV-Headers

```
curl -L https://github.com/KhronosGroup/SPIRV-Headers/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Headers-master/ spirv-tools/external/spirv-headers --delete
rm -rf SPIRV-Headers-master master.zip
```

### Commit
```
git add spirv-tools
git commit -m "Update SPIRV-Tools"
```

Please be sure to test Filament before uploading your PR.
