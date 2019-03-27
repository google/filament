# Filament

This package contains several executables and libraries you can use to build applications using
Filament. Latest versions are available on the [project page](https://github.com/google/filament).

## Binaries

- `cmgen`, Image-based lighting asset generator
- `filamesh`, Mesh converter
- `matc`, Material compiler
- `matinfo`, Displays information about materials compiled with `matc`
- `mipgen`, Generates a series of miplevels from a source image.
- `normal-blending`, Tool to blend normal maps
- `roughness-prefilter`, Pre-filters a roughness map from a normal map to reduce aliasing
- `skygen`, Physically-based sky environment texture generator
- `specular-color`, Computes the specular color of conductors based on spectral data

You can refer to the individual documentation files in `docs/` for more information.

## Libraries

Filament is distributed as a set of static libraries you must link against:

- `backend`, Required, implements all backends
- `bluegl`, Required to render with OpenGL or OpenGL ES
- `bluevk`, Required to render with Vulkan
- `filabridge`, Support library for Filament
- `filaflat`, Support library for Filament
- `filament`, Main Filament library
- `backend`, Filament render backend library
- `utils`, Support library for Filament
- `geometry`, Geometry helper library for Filament
- `smol-v`, SPIR-V compression library

To use Filament from Java you must use the following two libraries instead:
- `filament-java.jar`, Contains Filament's Java classes
- `filament-jni`, Filament's JNI bindings

To use the Vulkan backend on macOS you must also make the following libraries available at runtime:
- `MoltenVK_icd.json`
- `libMoltenVK.dylib`
- `vulkan.1.dylib`

## Linking against Filament

This walkthrough will get you successfully compiling and linking native code
against Filament with minimum dependencies.

To start, download Filament's [latest binary release](https://github.com/google/filament/releases)
and extract into a directory of your choosing. Binary releases are suffixed
with the platform name, for example, `filament-20181009-linux.tgz`.

Create a file, `main.cpp`, in the same directory with the following contents:

```
#include <filament/FilamentAPI.h>
#include <filament/Engine.h>

using namespace filament;

int main(int argc, char** argv)
{
    Engine *engine = Engine::create();
    engine->destroy(&engine);
    return 0;
}
```

The directory should look like:

```
|-- README.md
|-- bin
|-- docs
|-- include
|-- lib
|-- main.cpp
```

We'll use a platform-specific Makefile to compile and link `main.cpp` with Filament's libraries.
Copy your platform's Makefile below into a `Makefile` inside the same directory.

### Linux

```
FILAMENT_LIBS=-lfilament -lbackend -lbluegl -lbluevk -lfilabridge -lfilaflat -lutils -lgeometry -lsmol-v
CC=clang++

main: main.o
	$(CC) -Llib/x86_64/ main.o $(FILAMENT_LIBS) -lpthread -lc++ -ldl -o main

main.o: main.cpp
	$(CC) -Iinclude/ -std=c++14 -pthread -c main.cpp

clean:
	rm -f main main.o

.PHONY: clean
```

### macOS

```
FILAMENT_LIBS=-lfilament -lbackend -lbluegl -lbluevk -lfilabridge -lfilaflat -lutils -lgeometry -lsmol-v
FRAMEWORKS=-framework Cocoa -framework Metal
CC=clang++

main: main.o
	$(CC) -Llib/x86_64/ main.o $(FILAMENT_LIBS) $(FRAMEWORKS) -o main

main.o: main.cpp
	$(CC) -Iinclude/ -std=c++14 -c main.cpp

clean:
	rm -f main main.o

.PHONY: clean
```

### Windows

Note that the static libraries distributed for Windows include several
variants: mt, md, mtd, mdd. These correspond to the [run-time library
flags](https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library?view=vs-2017)
`/MT`, `/MD`, `/MTd`, and `/MDd`, respectively. Here we use the mt variant.

When building Filament from source, the `USE_STATIC_CRT` CMake option can be
used to change the run-time library version.

```
FILAMENT_LIBS=lib/x86_64/mt/filament.lib lib/x86_64/mt/backend.lib lib/x86_64/mt/bluegl.lib \
              lib/x86_64/mt/filabridge.lib lib/x86_64/mt/filaflat.lib lib/x86_64/mt/utils.lib \
              lib/x86_64/mt/geometry.lib lib/x86_64/mt/smol-v.lib
CC=clang-cl.exe

main.exe: main.obj
	$(CC) main.obj $(FILAMENT_LIBS) gdi32.lib user32.lib opengl32.lib

main.obj: main.cpp
	$(CC) /MT /Iinclude/ /std:c++14 /c main.cpp

clean:
	del main.exe main.obj

.PHONY: clean

```

### Compiling

You should be able to invoke `make` and run the executable successfully:

```
$ make
$ ./main
FEngine (64 bits) created at 0x106471000 (threading is enabled)
```

On Windows, you'll need to open up a Visual Studio Native Tools Command Prompt
and invoke `nmake` instead of `make`.


### Generating C++ documentation

To generate the documentation you must first install `doxygen` and `graphviz`, then run the 
following commands:

```
$ cd filament/filament
$ doxygen docs/doxygen/filament.doxygen
```

Finally simply open `docs/html/index.html` in your web browser.
