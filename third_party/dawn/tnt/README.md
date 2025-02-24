## Updating

Updating dawn is not a trivial process since there are duplicate dependencies between dawn and filament source tree.
It is not automated at this point. Following are the rough steps on how to do it.

```
cd third_party
clone dawn from https://github.com/google/dawn or https://dawn.googlesource.com/dawn to dawn_copy.
cd dawn_copy
python3 tools/fetch_dawn_dependencies.py
find . -name .git -type d -print0 | xargs -0 rm -r
cp -r dawn/tnt/ dawn_copy/tnt/
rm -rf dawn
mv dawn_copy dawn
patch -p2 < dawn/tnt/001-dawn-static-lib.patch.
git add dawn
```

Please be sure to test Filament before uploading your CL.
Make sure to fix any new issues that are caused by this update. dawn/tnt/CMakeLists.txt may need to be edited for
an update.