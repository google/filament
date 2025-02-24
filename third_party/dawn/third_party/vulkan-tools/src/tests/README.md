## Android

### Running vkcube on Android

```sh
cd Vulkan-Tools

cd build-android

# Optional
adb uninstall com.example.VkCube

adb install -r -g --no-incremental bin/VkCube.apk

adb shell am start com.example.VkCube/android.app.NativeActivity
```

### vulkaninfo on Android

Unlike `vkcube`, `vulkaninfo` doesn't require the extra step of creating an `APK`.

So the following should be enough.

```sh
cd Vulkan-Tools

scripts/android.py --config Release --app-abi arm64-v8a --app-stl c++_static --clean

adb push build-android/cmake/arm64-v8a/vulkaninfo/vulkaninfo /data/local/tmp

adb shell /data/local/tmp/vulkaninfo --json --output /data/local/tmp/foobar.json

adb pull /data/local/tmp/foobar.json
```
