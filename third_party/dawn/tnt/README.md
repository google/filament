## Updating

Updating dawn is not a trivial process since there are duplicate dependencies between dawn and filament source tree.
It is not automated at this point. Following are the rough steps on how to do it.

```
cd third_party
mkdir dawn_copy && cd dawn_copy
git init &&
git fetch --depth=1 https://dawn.googlesource.com/dawn --tag chromium/7353
git reset --hard FETCH_HEAD
python3 tools/fetch_dawn_dependencies.py
find . -name .git -type d -print0 | xargs -0 rm -r
<update .gitignore to remove third_party/entries from there>
rm -r test testing webgpu-cts docs
cp -r ../dawn/tnt .
cd ..
rm -rf dawn
mv dawn_copy dawn
patch -p2 < dawn/tnt/dawn-generator-CMakeList.patch
# remove redundant 3rd party dependencies with Filament itself
rm -rf \
    dawn/third_party/abseil-cpp \
    dawn/third_party/glslang \
    dawn/third_party/spirv-cross \
    dawn/third_party/spirv-headers \
    dawn/third_party/spirv-tools
git add dawn
<may need to add following separately>
git add dawn/third_party/dxc/ dawn/third_party/vulkan-loader/src/ dawn/third_party/spirv-tools/src/ dawn/third_party/glslang/
```

For each matched dependency (Known: absiel, spirv-tools, spirv-headers,
glslang) follow the relevant instructions in the dependency tnt folder and update it to the
version in the new Dawn commit.

Above steps are fragile and can cause compilation or functional breakage. Please be sure to test Filament before uploading your CL.
Make sure to fix any new issues that are caused by this update. dawn/tnt/CMakeLists.txt may need to be edited for
an update.