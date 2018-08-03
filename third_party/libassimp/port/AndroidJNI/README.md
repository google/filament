Build Asset Importer Lib for Android
====================================
This module provides a fascade for the io-stream-access to files behind the
android-asset-management within an Android native application.
- It is built as a static library
- It requires Android NDK with android API > 9 support.

### Building ###
To use this module please provide following cmake defines:
```
-DASSIMP_ANDROID_JNIIOSYSTEM=ON
-DCMAKE_TOOLCHAIN_FILE=$SOME_PATH/android.toolchain.cmake
```

"SOME_PATH" is a path to your cmake android toolchain script.

### Code ###
A small example how to wrap assimp for Android:
```cpp
#include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>

Assimp::Importer* importer = new Assimp::Importer();
Assimp::AndroidJNIIOSystem* ioSystem = new Assimp::AndroidJNIIOSystem(app->activity);
importer->SetIOHandler(ioSystem);
```
