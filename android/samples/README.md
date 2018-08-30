# Filament sample Android apps

This directory contains several sample Android applications that demonstrate how to use the
Filament APIs:

- `hello-triangle`: Minimal example showing how to setup a rendering surface for Filament 

## Prerequisites

Before you start, make sure to read [Filament's README](../../README.md). You need to be able to
compile Filament's native library and Filament's AAR for this project. The easiest way to proceed
is to install all the required dependencies and to run the following commands at the root of the
source tree:

```
$ ./build.sh -p desktop -i release
$ ./build.sh -p android release
```

This will build all the native components and the AAR required by this sample application.

If you do not use the build script, you must set the `filament_tools_dir` property when invoking
Gradle, either from the command line or from `local.properties`. This property must point to the
distribution/install directory for desktop (produced by make/ninja install). This directory must
contain `bin/matc` and `bin/cmgen`.

Example:
```
$ ./gradlew -Pfilament_tools_dir=../../dist-release assembleDebug
```

## Android Studio

Due to issues with composite builds in Android Studio 3.1, it is highly recommended to use
Android Studio 3.2 to open this project.
