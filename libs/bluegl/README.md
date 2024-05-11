# BlueGL Mechanics

So you want to call `glClear()`?

## Step 0: Run `bluegl-gen.py`

This step is only required if updating or modifying BlueGL. These artifacts should already be checked into the Filament repository.

From the `libs/bluegl` folder, run:

```
./bluegl-gen.py
```

The `bluegl-gen.py` script generates a set of files:
- assembly (proxy) files: `BlueGLCore*.S`
- header files: `include/BlueGLDefines.h` and `include/bluegl/BlueGL.h`
- a private header:`include/private_BlueGL.h`

## Step 1: Include the BlueGL defines header:

```
#include <bluegl/BlueGLDefines.h>
```

This headers adds a bunch of defines:

```
...
#define glClear bluegl_glClear
...
```

## Step 2: Include the BlueGL header after the defines header:

```
#include <bluegl/BlueGLDefines.h>
#include <bluegl/BlueGL.h>
```

This also includes the GL headers, like `<GL/glcorearb.h>` for you.

## Step 3: Call `bluegl::bind()`

Internally, the BlueGL library maintains a list of function pointers:

```
void* __blue_glCore_glClear;
```

During `bluegl::bind()`, each function gets assigned to the appropriate symbol loaded from the OS-specific GL shared library via `dlopen`, `dlsym`, and equivalents.

## Step 4: Call `glClear()`

Because of the prior `#define`, you'll actually be calling `bluegl_glClear()`. This is a trampoline function, defined in the `BlueGLCore*.S` assembly file (the exact implementation varies slightly on each platform):

```
.private_extern _bluegl_glClear
_bluegl_glClear:
    mov ___blue_glCore_glClear@GOTPCREL(%rip), %r11
    jmp *(%r11)
```

The invokes the `__blue_glCore_glClear` function, which was previously assigned to the actual GL function.
