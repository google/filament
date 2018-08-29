# Hello Triangle

This sample shows how to build a minimal Filament application for Android.

## Android Studio

Due to issues with composite builds in Android Studio 3.1, it is highly recommended to use
Android Studio 3.2 to open this project.

## Prerequisites

Before you start, make sure to read [Filament's README](../../../README.md). You need to be able to
compile Filament's native library and Filament's AAR for this project. The easiest way to proceed
is to install all the required dependencies and to run the following commands at the root of the
source tree:

```
$ ./build.sh -p desktop -i release
$ ./build.sh -p android release
```

This will build all the native components and the AAR required by this sample application.
