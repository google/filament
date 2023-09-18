# Filamat

Filamat allows for generating materials programatically on the device as opposed to with the `matc`
tool on the host machine. The cost is a binary size increase of your app due to the relatively
larger size of the `filamat` library.

For a smaller-sized library, see [`filamat_lite`](#filamat-lite). It has no dependencies on
`glslang`, but can only compile materials for OpenGL and does no shader code optimization.

The filamat package is included in the releases available on
[GitHub](https://github.com/google/filament/releases).

## Libraries

Filamat is distributed as a set of static libraries you must link against:

- `filamat`, Filamat main library
- `filabridge`, Support library for Filament / Filamat
- `shaders`, Shader text for material generation
- `utils`, Support library for Filament / Filamat
- `smol-v`, SPIR-V compression library

To use Filamat from Java you must use the following two libraries instead:
- `filamat-java.jar`, Contains Filamat's Java classes
- `filamat-jni`, Filamat's JNI bindings

## Linking against Filamat

This walkthrough will get you successfully compiling and linking native code against Filamat with
minimum dependencies.

To start, download Filament's [latest binary release](https://github.com/google/filament/releases)
and extract into a directory of your choosing. Binary releases are suffixed with the platform name,
for example, `filament-20181009-linux.tgz`.

Create a file, `main.cpp`, in the same directory with the following contents:

```c++
#include <filamat/MaterialBuilder.h>

#include <iostream>

using namespace filamat;

int main(int argc, char** argv)
{
    // Must be called before any materials can be built.
    MaterialBuilder::init();

    MaterialBuilder builder;
    builder
        .name("My material")
        .material("void material (inout MaterialInputs material) {"
                  "  prepareMaterial(material);"
                  "  material.baseColor.rgb = float3(1.0, 0.0, 0.0);"
                  "}")
        .shading(MaterialBuilder::Shading::LIT)
        .targetApi(MaterialBuilder::TargetApi::ALL)
        .platform(MaterialBuilder::Platform::ALL);

    Package package = builder.build();
    if (package.isValid()) {
        std::cout << "Success!" << std::endl;
    }

    // Call when finished building all materials to release internal MaterialBuilder resources.
    MaterialBuilder::shutdown();
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

We'll use a platform-specific Makefile to compile and link `main.cpp` with Filamat's libraries.
Copy your platform's Makefile below into a `Makefile` inside the same directory.

### Linux

```make
FILAMENT_LIBS=-lfilamat -lfilabridge -lshaders -lutils -lsmol-v
CC=clang++

main: main.o
	$(CC) -Llib/x86_64/ -stdlib=libc++ main.o $(FILAMENT_LIBS) -lpthread -ldl -o main

main.o: main.cpp
	$(CC) -Iinclude/ -std=c++17 -stdlib=libc++ -pthread -c main.cpp

clean:
	rm -f main main.o

.PHONY: clean
```

### macOS

```make
FILAMENT_LIBS=-lfilamat -lfilabridge -lshaders -lutils -lsmol-v
CC=clang++

main: main.o
	$(CC) -Llib/x86_64/ main.o $(FILAMENT_LIBS) -o main

main.o: main.cpp
	$(CC) -Iinclude/ -std=c++17 -c main.cpp

clean:
	rm -f main main.o

.PHONY: clean
```

### Windows

Note that the static libraries distributed for Windows include several
variants: mt, md, mtd, mdd. These correspond to the [run-time library
flags](https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library?view=vs-2017)
`/MT`, `/MD`, `/MTd`, and `/MDd`, respectively. Here we use the mt variant.

When building Filamat from source, the `USE_STATIC_CRT` CMake option can be
used to change the run-time library version.

```make
FILAMENT_LIBS=lib/x86_64/mt/filamat.lib lib/x86_64/mt/filabridge.lib lib/x86_64/mt/shaders.lib \
              lib/x86_64/mt/utils.lib lib/x86_64/mt/smol-v.lib
CC=clang-cl.exe

main.exe: main.obj
	$(CC) main.obj $(FILAMENT_LIBS) gdi32.lib user32.lib opengl32.lib

main.obj: main.cpp
	$(CC) /MT /Iinclude/ /std:c++17 /c main.cpp

clean:
	del main.exe main.obj

.PHONY: clean
```

## Compiling

You should be able to invoke `make` and run the executable successfully:

```
$ make
$ ./main
Success!
```

On Windows, you'll need to open up a Visual Studio Native Tools Command Prompt
and invoke `nmake` instead of `make`.

## Using the Material with Filament

For simplicity, this demo doesn't do anything useful with the built material package. To use the
material with Filament, pass the material package's data into a Filament Material builder:

```c++
    Package package = builder.build();
    filament::Material* myMaterial = Material::Builder()
        .package(package.getData(), package.getSize())
        .build(*engine);
```

Note that this will require [linking against Filament's libraries](../../filament/README.md) in
addition to Filamat's.

## Filamat Lite

The `filamat_lite` library is interchangeable with `filamat`, with a few caveats:

1. Material compilation is only supported for the OpenGL backend.
1. No shader-level optimization is performed.
1. GLSL correctness is not checked.

In addition, `filamat_lite` only performs a simple text match to determine which properties on the
`MaterialInputs` structure are set. The `material` input variable must also always be refered to by
the name `material`.

```glsl
void anotherFunction(inout MaterialInputs m) {
    // Incorrect! The MaterialInputs is being referred to by the name "m".
    m.metallic = 0.0;
}

void aFunction(inout MaterialInputs material) {
    // Works, but only because the variable name "material" is used.
    material.reflectance = 0.5;
}

// The MaterialInputs variable must be named material.
void material(inout MaterialInputs material) {
    prepareMaterial(material);

    // Good.
    material.roughness = materialParams.roughness;
    material.baseColor.rgb = vec3(1.0, 0.0, 1.0);

    aFunction(material);
    anotherFunction(material);
}
```
