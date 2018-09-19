# Tungsten

Tungsten is a node based material editor for the Filament rendering engine.

__Note: Tungsten is still a work-in-progress and is not ready for public at-large consumption yet. Use at your own risk!__

## Prerequisites

Before building Tungsten, you'll need to first build Filament. See Filament's [README](../../README.md) for instructions. Be sure to run `make install` (or the equivalent for your chosen build system) to install Filament binaries to the `dist` folder at the root of Filament. Tungsten relies on `filament-java.jar`, `libfilament-jni`, and tools such as `matc` and `cmgen` in the appropriate directories under `dist`:

```
Filament
|-- dist
|   |-- bin
|       |-- matc
|       |-- cmgen
|   |-- lib
|       |-- x86_64
|           |-- libfilament-jni.*
|       filament-java.jar
```

The location of this directory can be changed by updating the `filament_tools_dir` property inside of `gradle.properties`.

You'll also need Java 8 in order to use Tungsten. Tungsten is supported on Windows, Mac, and Linux.

## Getting Started

`cd` into the Tungsten directory. The standalone version of Tungsten can be run directly from [Gradle](https://gradle.org/):

```
$ cd tools/tungsten
$ ./gradlew :standalone:run
```

## Running inside of Android Studio

To run inside of Android Studio, set the `ANDROID_STUDIO` environment variable to point to your install of Android Studio, for example:

```
export ANDROID_STUDIO="/Applications/Android Studio.app"     # Mac
set ANDROID_STUDIO=C:\Program Files\Android\Android Studio   # Windows
```

Then run
```
$ ./gradlew :plugin:runIde
```

## Building

Tungsten comes in two flavors: a standalone app, and (in the future) an Android Studio plugin. The plugin is still in heavy development and not functional yet.

Both targets can be built using Gradle.

```
$ ./gradlew build
```

To build the standalone only:

```
$ ./gradlew :standalone:build
```

To build the plugin only:

```
$ ./gradlew :plugin:build
```

The Android Studio plugin will be built at `plugin/build/distributions/Tungsten-x.y.z.zip`.

## Running the tests

Tests can be run with

```
$ ./gradlew test
```

## Contributing

Please see Filament's [CONTRIBUTING](../../CONTRIBUTING.md) for details.

## Authors

* **Benjamin Doherty**

## License

Please see Filament's [LICENSE](../../LICENSE) for details.
