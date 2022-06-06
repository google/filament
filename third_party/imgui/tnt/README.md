## Updating

To update imgui to a public release on GitHub, do the following.

First, find the release tag you want on https://github.com/ocornut/imgui/releases/.
These steps assume v1.77.

```
cd third_party
curl -L https://github.com/ocornut/imgui/archive/v1.77.zip > imgui.zip
unzip imgui.zip
rsync -r imgui-1.77/ imgui/ --delete
git checkout imgui/tnt
rm -rf imgui-1.77 imgui.zip
rm imgui/examples/libs/glfw/lib-vc2010-32/*.lib
rm imgui/examples/libs/glfw/lib-vc2010-64/*.lib
git add imgui
```

Make any necessary changes to tnt/CMakeLists.txt to get Filament to compile.

Please be sure to test Filament before uploading your CL.
