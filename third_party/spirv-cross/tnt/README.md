To update to the spirv-cross that's currently on GitHub master, do the following.

```
curl -L https://github.com/KhronosGroup/SPIRV-Cross/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Cross-master/ spirv-cross/ --delete
git checkout spirv-cross/tnt/*
rm -rf SPIRV-Cross-master master.zip
git add spirv-cross
```

Please be sure to test Filament before uploading your CL.