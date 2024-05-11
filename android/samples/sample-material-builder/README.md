# Android `material-builder` Sample

The `material-builder` sample shows how to generate materials programatically on the device as
opposed to with the `matc` tool on the host machine. The cost is a binary size increase of your app
due to the relatively larger size of the `filamat` library.

The `buildMaterial` method inside `MainActivity.kt` uses a `MaterialBuilder()` instance to compile a
material package whose payload is sent to the engine where it is applied towards the `Renderable`.

## Before building

The `filamat` static library is needed to fascilitate material building on the device. Compiling the
library is slower and thus it is not built by the default `build.sh` command. To build, add the `-l`
flag to the `build.sh` command:

```
./build.sh -i -p android release
```

Explanation of flags:

- The `-i` flag installs the libraries to `out/android-release/`, so this sample can find them.
- The `-p` flag and `android` argument choose Android as the platform to build for.
