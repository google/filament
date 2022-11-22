When updating spirv-tools to a new version there are several `CMakeLists.txt` files that need
to patched with Filament specific changes. This can be done by running the following command at
Filament's root:

```
git apply third_party/spirv-tools/filament-specific-changes.patch
```

The following procedure can be used to update spirv-tools. Note that there is a secondary repository
that needs to be downloaded (spirv-headers).

```
curl -L https://github.com/KhronosGroup/spirv-tools/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Tools-master/ spirv-tools/ --delete
rm -rf SPIRV-Tools-master master.zip
curl -L https://github.com/KhronosGroup/spirv-headers/archive/master.zip > master.zip
unzip master.zip
mv SPIRV-Headers-master/ spirv-tools/external/spirv-headers
rm master.zip

git add spirv-tools
```

The patch file also edits the `.gitignore` to allow `spirv-headers` to be committed.

Finally, remember to bring back the Filament-specific changes in CMakeLists.

To restore this file and the patch file do:

```
git checkout main spirv-tools/FILAMENT_README.md
git checkout main spirv-tools/filament-specific-changes.patch
```
