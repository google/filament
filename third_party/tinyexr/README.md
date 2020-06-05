# Tiny OpenEXR image library.

[![Total alerts](https://img.shields.io/lgtm/alerts/g/syoyo/tinyexr.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/syoyo/tinyexr/alerts/)

![Example](https://github.com/syoyo/tinyexr/blob/master/asakusa.png?raw=true)

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/k07ftfe4ph057qau/branch/master?svg=true)](https://ci.appveyor.com/project/syoyo/tinyexr/branch/master)

[![Travis build Status](https://travis-ci.org/syoyo/tinyexr.svg)](https://travis-ci.org/syoyo/tinyexr)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/5827/badge.svg)](https://scan.coverity.com/projects/5827)

`tinyexr` is a small, single header-only library to load and save OpenEXR (.exr) images.
`tinyexr` is written in portable C++ (no library dependency except for STL), thus `tinyexr` is good to embed into your application.
To use `tinyexr`, simply copy `tinyexr.h` into your project.

Current status of `tinyexr` is:

- OpenEXR v1 image
  - [x] Scanline format
  - [ ] Tiled format
    - [x] Tile format with no LoD (load).
    - [ ] Tile format with LoD (load).
    - [ ] Tile format with no LoD (save).
    - [ ] Tile format with LoD (save).
  - [x] Custom attributes
- OpenEXR v2 image
  - [ ] Multipart format
    - [x] Load multi-part image
    - [ ] Save multi-part image
    - [ ] Load multi-part deep image
    - [ ] Save multi-part deep image
- OpenEXR v2 deep image
  - [x] Loading scanline + ZIPS + HALF or FLOAT pixel type.
- Compression
  - [x] NONE
  - [x] RLE
  - [x] ZIP
  - [x] ZIPS
  - [x] PIZ
  - [x] ZFP (tinyexr extension)
  - [ ] B44?
  - [ ] B44A?
  - [ ] PIX24?
- Line order.
  - [x] Increasing, decreasing (load)
  - [ ] Random?
  - [ ] Increasing, decreasing (save)
- Pixel format (UINT, FLOAT).
  - [x] UINT, FLOAT (load)
  - [x] UINT, FLOAT (deep load)
  - [x] UINT, FLOAT (save)
  - [ ] UINT, FLOAT (deep save)
- Support for big endian machine.
  - [x] Loading scanline image
  - [x] Saving scanline image
  - [ ] Loading multi-part channel EXR
  - [ ] Saving multi-part channel EXR
  - [ ] Loading deep image
  - [ ] Saving deep image
- Optimization
  - [x] C++11 thread loading
  - [ ] C++11 thread saving
  - [ ] ISPC?
  - [x] OpenMP multi-threading in EXR loading.
  - [x] OpenMP multi-threading in EXR saving.
  - [ ] OpenMP multi-threading in deep image loading.
  - [ ] OpenMP multi-threading in deep image saving.
* C interface.
  * You can easily write language bindings (e.g. golang)

# Use case

## New TinyEXR (v0.9.5+)

* Godot. Multi-platform 2D and 3D game engine https://godotengine.org/
* Filament. PBR engine. https://github.com/google/filament
* PyEXR. Loading OpenEXR (.exr) images using Python. https://github.com/ialhashim/PyEXR
* The-Forge. The Forge Cross-Platform Rendering Framework PC, Linux, Ray Tracing, macOS / iOS, Android, XBOX, PS4 https://github.com/ConfettiFX/The-Forge
* Your project here!

## Older TinyEXR (v0.9.0)

* mallie https://github.com/lighttransport/mallie
* Cinder 0.9.0 https://libcinder.org/notes/v0.9.0
* Piccante (develop branch) http://piccantelib.net/
* Your project here!

## Examples

* [examples/deepview/](examples/deepview) Deep image view
* [examples/rgbe2exr/](examples/rgbe2exr) .hdr to EXR converter
* [examples/exr2rgbe/](examples/exr2rgbe) EXR to .hdr converter
* [examples/ldr2exr/](examples/exr2rgbe) LDR to EXR converter
* [examples/exr2ldr/](examples/exr2ldr) EXR to LDR converter
* [examples/exr2fptiff/](examples/exr2fptiff) EXR to 32bit floating point TIFF converter
  * for 32bit floating point TIFF to EXR convert, see https://github.com/syoyo/tinydngloader/tree/master/examples/fptiff2exr
* [examples/cube2longlat/](examples/cube2longlat) Cubemap to longlat (equirectangler) converter

## Experimental

* [experimental/js/](experimental/js) JavaScript port using Emscripten

## Usage

NOTE: **API is still subject to change**. See the source code for details.

Include `tinyexr.h` with `TINYEXR_IMPLEMENTATION` flag (do this only for **one** .cc file).

```cpp
//Please include your own zlib-compatible API header before
//including `tinyexr.h` when you disable `TINYEXR_USE_MINIZ`
//#define TINYEXR_USE_MINIZ 0
//#include "zlib.h"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
```

### Compile flags

* `TINYEXR_USE_MINIZ` Use embedded miniz (default = 1). Please include `zlib.h` header (before `tinyexr.h`) if you disable miniz support.
* `TINYEXR_USE_PIZ` Enable PIZ compression support (default = 1)
* `TINYEXR_USE_ZFP` Enable ZFP compression supoort (TinyEXR extension, default = 0)
* `TINYEXR_USE_THREAD` Enable threaded loading using C++11 thread (Requires C++11 compiler, default = 0)
* `TINYEXR_USE_OPENMP` Enable OpenMP threading support (default = 1 if `_OPENMP` is defined)
  * Use `TINYEXR_USE_OPENMP=0` to force disable OpenMP code path even if OpenMP is available/enabled in the compiler.

### Quickly reading RGB(A) EXR file.

```cpp
  const char* input = "asakusa.exr";
  float* out; // width * height * RGBA
  int width;
  int height;
  const char* err = NULL; // or nullptr in C++11

  int ret = LoadEXR(&out, &width, &height, input, &err);

  if (ret != TINYEXR_SUCCESS) {
    if (err) {
       fprintf(stderr, "ERR : %s\n", err);
       FreeEXRErrorMessage(err); // release memory of error message.
    }
  } else {
    ...
    free(out); // release memory of image data
  }

```

### Reading layered RGB(A) EXR file.

If you want to read EXR image with layer info (channel has a name with delimiter `.`), please use `LoadEXRWithLayer` API.

You need to know layer name in advance (e.g. through `EXRLayers` API).

```cpp
  const char* input = ...;
  const char* layer_name = "diffuse"; // or use EXRLayers to get list of layer names in .exr
  float* out; // width * height * RGBA
  int width;
  int height;
  const char* err = NULL; // or nullptr in C++11

  // will read `diffuse.R`, `diffuse.G`, `diffuse.B`, (`diffuse.A`) channels
  int ret = LoadEXRWithLayer(&out, &width, &height, input, layer_name, &err);

  if (ret != TINYEXR_SUCCESS) {
    if (err) {
       fprintf(stderr, "ERR : %s\n", err);
       FreeEXRErrorMessage(err); // release memory of error message.
    }
  } else {
    ...
    free(out); // release memory of image data
  }

```

### Loading Singlepart EXR from a file.

Scanline and tiled format are supported.

```cpp
  // 1. Read EXR version.
  EXRVersion exr_version;

  int ret = ParseEXRVersionFromFile(&exr_version, argv[1]);
  if (ret != 0) {
    fprintf(stderr, "Invalid EXR file: %s\n", argv[1]);
    return -1;
  }

  if (exr_version.multipart) {
    // must be multipart flag is false.
    return -1;
  }

  // 2. Read EXR header
  EXRHeader exr_header;
  InitEXRHeader(&exr_header);

  const char* err = NULL; // or `nullptr` in C++11 or later.
  ret = ParseEXRHeaderFromFile(&exr_header, &exr_version, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Parse EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  // // Read HALF channel as FLOAT.
  // for (int i = 0; i < exr_header.num_channels; i++) {
  //   if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
  //     exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
  //   }
  // }

  EXRImage exr_image;
  InitEXRImage(&exr_image);

  ret = LoadEXRImageFromFile(&exr_image, &exr_header, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Load EXR err: %s\n", err);
    FreeEXRHeader(&exr_header);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  // 3. Access image data
  // `exr_image.images` will be filled when EXR is scanline format.
  // `exr_image.tiled` will be filled when EXR is tiled format.

  // 4. Free image data
  FreeEXRImage(&exr_image);
  FreeEXRHeader(&exr_header);
```

### Loading Multipart EXR from a file.

Scanline and tiled format are supported.

```cpp
  // 1. Read EXR version.
  EXRVersion exr_version;

  int ret = ParseEXRVersionFromFile(&exr_version, argv[1]);
  if (ret != 0) {
    fprintf(stderr, "Invalid EXR file: %s\n", argv[1]);
    return -1;
  }

  if (!exr_version.multipart) {
    // must be multipart flag is true.
    return -1;
  }

  // 2. Read EXR headers in the EXR.
  EXRHeader **exr_headers; // list of EXRHeader pointers.
  int num_exr_headers;
  const char *err = NULL; // or nullptr in C++11 or later

  // Memory for EXRHeader is allocated inside of ParseEXRMultipartHeaderFromFile,
  ret = ParseEXRMultipartHeaderFromFile(&exr_headers, &num_exr_headers, &exr_version, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Parse EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  printf("num parts = %d\n", num_exr_headers);


  // 3. Load images.

  // Prepare array of EXRImage.
  std::vector<EXRImage> images(num_exr_headers);
  for (int i =0; i < num_exr_headers; i++) {
    InitEXRImage(&images[i]);
  }

  ret = LoadEXRMultipartImageFromFile(&images.at(0), const_cast<const EXRHeader**>(exr_headers), num_exr_headers, argv[1], &err);
  if (ret != 0) {
    fprintf(stderr, "Parse EXR err: %s\n", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  printf("Loaded %d part images\n", num_exr_headers);

  // 4. Access image data
  // `exr_image.images` will be filled when EXR is scanline format.
  // `exr_image.tiled` will be filled when EXR is tiled format.

  // 5. Free images
  for (int i =0; i < num_exr_headers; i++) {
    FreeEXRImage(&images.at(i));
  }

  // 6. Free headers.
  for (int i =0; i < num_exr_headers; i++) {
    FreeEXRHeader(exr_headers[i]);
    free(exr_headers[i]);
  }
  free(exr_headers);
```


Saving Scanline EXR file.

```cpp
  // See `examples/rgbe2exr/` for more details.
  bool SaveEXR(const float* rgb, int width, int height, const char* outfilename) {

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(width * height);
    images[1].resize(width * height);
    images[2].resize(width * height);

    // Split RGBRGBRGB... into R, G and B layer
    for (int i = 0; i < width * height; i++) {
      images[0][i] = rgb[3*i+0];
      images[1][i] = rgb[3*i+1];
      images[2][i] = rgb[3*i+2];
    }

    float* image_ptr[3];
    image_ptr[0] = &(images[2].at(0)); // B
    image_ptr[1] = &(images[1].at(0)); // G
    image_ptr[2] = &(images[0].at(0)); // R

    image.images = (unsigned char**)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = 3;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    const char* err = NULL; // or nullptr in C++11 or later.
    int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
    if (ret != TINYEXR_SUCCESS) {
      fprintf(stderr, "Save EXR err: %s\n", err);
      FreeEXRErrorMessage(err); // free's buffer for an error message
      return ret;
    }
    printf("Saved exr file. [ %s ] \n", outfilename);

    free(rgb);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

  }
```


Reading deep image EXR file.
See `example/deepview` for actual usage.

```cpp
  const char* input = "deepimage.exr";
  const char* err = NULL; // or nullptr
  DeepImage deepImage;

  int ret = LoadDeepEXR(&deepImage, input, &err);

  // access to each sample in the deep pixel.
  for (int y = 0; y < deepImage.height; y++) {
    int sampleNum = deepImage.offset_table[y][deepImage.width-1];
    for (int x = 0; x < deepImage.width-1; x++) {
      int s_start = deepImage.offset_table[y][x];
      int s_end   = deepImage.offset_table[y][x+1];
      if (s_start >= sampleNum) {
        continue;
      }
      s_end = (s_end < sampleNum) ? s_end : sampleNum;
      for (int s = s_start; s < s_end; s++) {
        float val = deepImage.image[depthChan][y][s];
        ...
      }
    }
  }

```

### deepview

`examples/deepview` is simple deep image viewer in OpenGL.

![DeepViewExample](https://github.com/syoyo/tinyexr/blob/master/examples/deepview/deepview_screencast.gif?raw=true)

## TinyEXR extension

### ZFP

#### NOTE

TinyEXR adds ZFP compression as an experimemtal support (Linux and MacOSX only).

ZFP only supports FLOAT format pixel, and its image width and height must be the multiple of 4, since ZFP compresses pixels with 4x4 pixel block.

#### Setup

Checkout zfp repo as an submodule.

    $ git submodule update --init

#### Build

Then build ZFP

    $ cd deps/ZFP
    $ mkdir -p lib   # Create `lib` directory if not exist
    $ make

Set `1` to `TINYEXT_USE_ZFP` define in `tinyexr.h`

Build your app with linking `deps/ZFP/lib/libzfp.a`

#### ZFP attribute

For ZFP EXR image, the following attribute must exist in its EXR image.

* `zfpCompressionType` (uchar).
  * 0 = fixed rate compression
  * 1 = precision based variable rate compression
  * 2 = accuracy based variable rate compression

And the one of following attributes must exist in EXR, depending on the `zfpCompressionType` value.

* `zfpCompressionRate` (double)
  * Specifies compression rate for fixed rate compression.
* `zfpCompressionPrecision` (int32)
  * Specifies the number of bits for precision based variable rate compression.
* `zfpCompressionTolerance` (double)
  * Specifies the tolerance value for accuracy based variable rate compression.

#### Note on ZFP compression.

At least ZFP code itself works well on big endian machine.

## Unit tests

See `test/unit` directory.

## TODO

Contribution is welcome!

- [ ] Compression
  - [ ] B44?
  - [ ] B44A?
  - [ ] PIX24?
- [ ] Custom attributes
  - [x] Normal image (EXR 1.x)
  - [ ] Deep image (EXR 2.x)
- [ ] JavaScript library (experimental, using Emscripten)
  - [x] LoadEXRFromMemory
  - [ ] SaveMultiChannelEXR
  - [ ] Deep image save/load
- [ ] Write from/to memory buffer.
  - [ ] Deep image save/load
- [ ] Tile format.
  - [x] Tile format with no LoD (load).
  - [ ] Tile format with LoD (load).
  - [ ] Tile format with no LoD (save).
  - [ ] Tile format with LoD (save).
- [ ] Support for custom compression type.
  - [x] zfp compression (Not in OpenEXR spec, though)
  - [ ] zstd?
- [x] Multi-channel.
- [ ] Multi-part (EXR2.0)
  - [x] Load multi-part image
  - [ ] Load multi-part deep image
- [ ] Line order.
  - [x] Increasing, decreasing (load)
  - [ ] Random?
  - [ ] Increasing, decreasing (save)
- [ ] Pixel format (UINT, FLOAT).
  - [x] UINT, FLOAT (load)
  - [x] UINT, FLOAT (deep load)
  - [x] UINT, FLOAT (save)
  - [ ] UINT, FLOAT (deep save)
- [ ] Support for big endian machine.
  - [ ] Loading multi-part channel EXR
  - [ ] Saving multi-part channel EXR
  - [ ] Loading deep image
  - [ ] Saving deep image
- [ ] Optimization
  - [ ] ISPC?
  - [x] OpenMP multi-threading in EXR loading.
  - [x] OpenMP multi-threading in EXR saving.
  - [ ] OpenMP multi-threading in deep image loading.
  - [ ] OpenMP multi-threading in deep image saving.

## Python bindings

`pytinyexr` is available: https://pypi.org/project/pytinyexr/ (loading only as of 0.9.1)

## Similar or related projects

* miniexr: https://github.com/aras-p/miniexr (Write OpenEXR)
* stb_image_resize.h: https://github.com/nothings/stb (Good for HDR image resizing)

## License

3-clause BSD

`tinyexr` uses miniz, which is developed by Rich Geldreich <richgel99@gmail.com>, and licensed under public domain.

`tinyexr` tools uses stb, which is licensed under public domain: https://github.com/nothings/stb
`tinyexr` uses some code from OpenEXR, which is licensed under 3-clause BSD license.

## Author(s)

Syoyo Fujita (syoyo@lighttransport.com)

## Contributor(s)

* Matt Ebb (http://mattebb.com): deep image example. Thanks!
* Matt Pharr (http://pharr.org/matt/): Testing tinyexr with OpenEXR(IlmImf). Thanks!
* Andrew Bell (https://github.com/andrewfb) & Richard Eakin (https://github.com/richardeakin): Improving TinyEXR API. Thanks!
* Mike Wong (https://github.com/mwkm): ZIPS compression support in loading. Thanks!
