When updating spirv-tools to a new version, make sure to preserve all the changes marked with
the following in `CMakeLists.txt` and `source/CMakeLists.txt`:

`# Filament specific changes`

You can easily apply these changes by running the following command at Filament's root:

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

Edit the .gitignore so it doesn't prevent `spirv-headers` from being committed.

Finally, remember to bring back the Filament-specific changes in CMakeLists.
