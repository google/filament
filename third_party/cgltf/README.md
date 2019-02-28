# cgltf
**Single-file/stb-style C glTF loader**

[![Build Status](https://travis-ci.org/jkuhlmann/cgltf.svg?branch=master)](https://travis-ci.org/jkuhlmann/cgltf)

## Usage
Loading from file:
```c
#include "cgltf.h"

cgltf_options options = {0};
cgltf_data* data = NULL;
cgltf_result result = cgltf_parse_file(&options, "scene.gltf", &data);
if (result == cgltf_result_success)
{
	/* TODO make awesome stuff */
	cgltf_free(data);
}
```

Loading from memory:
```c
#include "cgltf.h"

void* buf; /* Pointer to glb or gltf file data */
size_t size; /* Size of the file data */

cgltf_options options = {0};
cgltf_data* data = NULL;
cgltf_result result = cgltf_parse(&options, buf, size, &data);
if (result == cgltf_result_success)
{
	/* TODO make awesome stuff */
	cgltf_free(data);
}
```

Note that cgltf does not load the contents of extra files such as buffers or images into memory by default. You'll need to read these files yourself using URIs from `data.buffers[]` or `data.images[]` respectively.
For buffer data, you can alternatively call `cgltf_load_buffers`, which will use `FILE*` APIs to open and read buffer files.

**For more in-depth documentation and a description of the public interface refer to the top of the `cgltf.h` file.**

## Support
cgltf supports core glTF 2.0:
- glb (binary files) and gltf (JSON files)
- meshes (including accessors, buffer views, buffers)
- materials (including textures, samplers, images)
- scenes and nodes
- skins
- animations
- cameras
- morph targets

cgltf also supports some glTF extensions:
- KHR_lights_punctual
- KHR_materials_pbrSpecularGlossiness
- KHR_materials_unlit
- KHR_texture_transform

cgltf does **not** yet support unlisted extensions or `extra` data.

## Building
The easiest approach is to integrate the `cgltf.h` header file into your project. If you are unfamiliar with single-file C libraries (also known as stb-style libraries), this is how it goes:

1. Include `cgltf.h` where you need the functionality.
1. Have exactly one source file that defines `CGLTF_IMPLEMENTATION` before including `cgltf.h`.
1. Use the cgltf functions as described above.

## Contributing
Everyone is welcome to contribute to the library. If you find any problems, you can submit them using [GitHub's issue system](https://github.com/jkuhlmann/cgltf/issues). If you want to contribute code, you should fork the project and then send a pull request.

## Dependencies
None.

C headers being used by implementation:
```
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
```

Note, this library has a copy of the [JSMN JSON parser](https://github.com/zserge/jsmn) embedded in its source.

## Testing
There is a Python script in the `test/` folder that retrieves the glTF 2.0 sample files from the glTF-Sample-Models repository (https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0) and runs the library against all gltf and glb files.

Here's one way to build and run the test:

    cd test ; mkdir build ; cd build ; cmake .. -DCMAKE_BUILD_TYPE=Debug
    make -j
    cd ..
    ./test_all.py

There is also a llvm-fuzz test in `fuzz/`. See http://llvm.org/docs/LibFuzzer.html for more information.
