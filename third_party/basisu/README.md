<!-- Copyright 2016-2026 Binomial LLC -->
<!-- SPDX-License-Identifier: Apache-2.0 -->

# basis_universal v2.1
An LDR/HDR portable GPU supercompressed texture transcoding system. 

[![Build status](https://img.shields.io/appveyor/build/BinomialLLC/basis-universal/master.svg)](https://ci.appveyor.com/project/BinomialLLC/basis-universal)

----

Intro
-----

Basis Universal™ v2.1 is an open source [supercompressed](http://gamma.cs.unc.edu/GST/gst.pdf) LDR/HDR GPU compressed texture interchange system from Binomial LLC that supports two intermediate file formats: the [.KTX2 open standard from the Khronos Group](https://registry.khronos.org/KTX/specs/2.0/ktxspec.v2.html), and our own ".basis" file format. These file formats support rapid transcoding to virtually any compressed [GPU texture format](https://grokipedia.com/page/texture_compression) released over the past quarter century. 

## GPU Textures are Infrastructure

Our overall goal is to simplify the encoding and efficient distribution of *portable* LDR and HDR GPU texture, image, and short [texture video](https://github.com/BinomialLLC/basis_universal/wiki/Encoding-ETC1S-and-XUASTC-LDR-Texture-Video) content in a way that is compatible with any GPU or rendering/graphics API. 

The system supports seven modes (or codecs). In the order they were implemented:
1. **ETC1S**: A supercompressed subset of ETC1 designed for very fast transcoding to other LDR texture formats, low/medium quality but high compression, slightly faster transcoding to other LDR texture formats vs. libjpeg.
2. **UASTC LDR 4x4 (with or without RDO)**: Custom ASTC 4x4-like format designed for very fast transcoding to other LDR texture formats, high quality
3. **UASTC HDR 4x4**: Standard ASTC HDR 4x4 texture data, but constrained for very fast transcoding to BC6H
4. **ASTC HDR 6x6 (with or without RDO)**: Standard ASTC HDR 6x6
5. **UASTC HDR 6x6 Intermediate ("GPU Photo HDR")**: Supercompressed ASTC HDR 6x6
6. **ASTC LDR 4x4-12x12 (all 14 standard ASTC block sizes, with or without basic windowed RDO)**: Standard ASTC LDR 4x4-12x12
7. **XUASTC LDR 4x4-12x12 (all 14 standard ASTC block sizes, "GPU Photo LDR/SDR")**: Latent-space supercompressed ASTC LDR with Weight Grid DCT ([Discrete Cosine Transform](https://grokipedia.com/page/Discrete_cosine_transform)) for very high quality, extreme bitrate scalability, optional adaptive deblocking (CPU or using a [simple GPU pixel shader](https://github.com/BinomialLLC/basis_universal/tree/master/shader_deblocking) compatible with mipmapping and filtering), three entropy coding profiles (Zstd, arithmetic or hybrid). See [JPEG for ASTC](https://github.com/BinomialLLC/basis_universal/wiki/JPEG-for-ASTC), and the [ASTC and XUASTC LDR Usage Guide](https://github.com/BinomialLLC/basis_universal/wiki/ASTC-and-XUASTC-LDR-Usage-Guide).

The C/C++ encoder and transcoder libraries can be compiled to native code or WebAssembly (web or WASI), and all encoder/transcoder features can be accessed from JavaScript via a C++ wrapper library which optionally supports [WASM multithreading](https://web.dev/articles/webassembly-threads) for fast encoding in the browser. [WASM WASI](https://wasi.dev/) builds, for the command line tool and the encoder/transcoder as a WASI module using a pure C API, are also supported. 

Full Python support for encoding/transcoding is now available, supporting native or WASM modules, but is still in the early stages of development.

License/Legal
-------------

The reference encoder library, transcoder, and most specification documents in this repo (unless otherwise explictly indicated) are Copyright © 2016–2026 Binomial LLC. All rights reserved except as granted under the [Apache 2.0 LICENSE](https://github.com/BinomialLLC/basis_universal/blob/master/LICENSE). Basis Universal™ is a trademark of Binomial LLC. KTX™ is a trademark of [The Khronos Group Inc.](https://www.khronos.org/ktx/) See our Apache 2.0 [NOTICE file](https://github.com/BinomialLLC/basis_universal/wiki/NOTICE). If you modify the Basis Universal reference source code, specifications, or wiki documents and redistribute the files, you must cause any modified files to carry prominent notices stating that you changed the files (see Apache 2.0 §4(b)).

See our [DEP5 file](https://github.com/BinomialLLC/basis_universal/blob/master/.reuse/dep5) for the complete list of software and their licenses in this repo. The encoder library is Apache 2.0, but it utilizes some open source 3rd party modules (in 'encoder/3rdparty' and in the 'Zstd' directory) to load [.QOI](https://qoiformat.org/), [.DDS](https://github.com/DeanoC/tiny_dds), [.EXR](https://github.com/syoyo/tinyexr) images, to handle [Zstd](https://github.com/facebook/zstd) compression, and to unpack ASTC texture blocks. See the [LICENSES](https://github.com/BinomialLLC/basis_universal/tree/master/LICENSES) folder. The transcoder utilizes no 3rd party libraries or dependencies, other than Zstd (which is optional but limits the transcoder to non-Zstd utilizing codecs).

Links
-----

- [Wiki/Specifications](https://github.com/BinomialLLC/basis_universal/wiki)
- [Release Notes](https://github.com/BinomialLLC/basis_universal/wiki/Release-Notes)
- [Live Compression/Transcoding Testbed](https://subquantumtech.com/xu/ktx2_encode_test/) - A WASM64 compatible browser is recommended (such as Chrome/Edge/Firefox), especially for XUASTC LDR compression, but it works under plain WASM too (with resolution limits due to less available memory).
- [Live WebGL Examples](https://subquantumtech.com/xu/)
- [JavaScript API/WASM/WebGL info](https://github.com/BinomialLLC/basis_universal/tree/master/webgl)
- [XUASTC LDR Specification](https://github.com/BinomialLLC/basis_universal/wiki/XUASTC-LDR-Specification-v1.0)

### UASTC HDR 4x4/6x6 Specific Links:

- [UASTC HDR 4x4 Example Images](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-Examples)
- [UASTC HDR 6x6 Example Images](https://github.com/BinomialLLC/basis_universal/wiki/ASTC-HDR-6x6-Example-Images)
- [UASTC HDR 6x6 Support Notes](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-6x6-Support-Notes)
- [Quick comparison of ARM's astcenc HDR 6x6 encoder vs. ours](https://github.com/richgel999/junkdrawer/wiki/ASTC-HDR-6x6-Encoder-Comparisons)

----

Supported LDR GPU Texture Formats
---------------------------------

ETC1S, UASTC LDR 4x4, XUASTC LDR 4x4-12x12 and ASTC LDR 4x4-12x12 files can be transcoded to:
- ASTC LDR 4x4 L/LA/RGB/RGBA 8bpp
- ASTC LDR 4x4-12x12 (XUASTC/ASTC), 0.89-8bpp
- BC1-5 RGB/RGBA/X/XY
- BC7 RGB/RGBA
- ETC1 RGB, ETC2 RGBA, and ETC2 EAC R11/RG11
- PVRTC1 4bpp RGB/RGBA and PVRTC2 RGB/RGBA
- ATC RGB/RGBA and FXT1 RGB
- Uncompressed LDR raster image formats: 8888/565/4444

Supported HDR GPU Texture Formats
---------------------------------

UASTC HDR 4x4, ASTC HDR 6x6, and UASTC HDR 6x6 files can be transcoded to:
- ASTC HDR 4x4 (8bpp, UASTC HDR 4x4 only)
- ASTC HDR 6x6 RGB (3.56bpp, ASTC HDR 6x6 or UASTC HDR 6x6 intermediate only)
- BC6H RGB (8bpp, either UASTC HDR 4x4 or UASTC HDR 6x6)
- Uncompressed HDR raster image formats: RGB_16F/RGBA_16F (half float/FP16 RGB, 48 or 64bpp), or 32-bit/pixel shared exponent [RGB_9E5](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt)

----

Supported Texture Compression/Supercompression Modes
----------------------------------------------------

1. **[ETC1S](https://github.com/BinomialLLC/basis_universal/wiki/.basis-File-Format-and-ETC1S-Texture-Video-Specification)**: A roughly .3-3bpp low to medium quality supercompressed mode based on a subset of [ETC1](https://en.wikipedia.org/wiki/Ericsson_Texture_Compression) called "ETC1S". This mode supports variable quality vs. file size levels (like JPEG), alpha channels, built-in compression, and texture arrays optionally compressed as a video sequence using skip blocks ([Conditional Replenishment](https://en.wikipedia.org/wiki/MPEG-1)). This mode can be rapidly transcoded to all of the supported LDR texture formats.

2. **[UASTC LDR 4x4](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-LDR-4x4-Texture-Specification)**: An 8 bits/pixel LDR high quality mode. UASTC LDR is a 19 mode subset of the standard [ASTC LDR](https://en.wikipedia.org/wiki/Adaptive_scalable_texture_compression) 4x4 (8bpp) texture format, but with a custom block format containing transcoding hints. Transcoding UASTC LDR to ASTC LDR and BC7 is particularly fast and simple, because UASTC LDR is a common subset of both BC7 and ASTC. The transcoders for the other texture formats are accelerated by several format-specific hint bits present in each UASTC LDR block.

This mode supports an optional [Rate-Distortion Optimized (RDO)](https://en.wikipedia.org/wiki/Rate%E2%80%93distortion_optimization) post-process stage that conditions the encoded UASTC LDR texture data in the .KTX2/.basis file so it can be more effectively LZ compressed. More details [here](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-implementation-details).

Here is the [UASTC LDR 4x4 specification document](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-LDR-4x4-Texture-Specification).

3. **[UASTC HDR 4x4](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-4x4-Texture-Specification)**: An 8 bits/pixel HDR high quality mode. This is a 24 mode subset of the standard [ASTC HDR](https://en.wikipedia.org/wiki/Adaptive_scalable_texture_compression) 4x4 (8bpp) texture format. It's designed to be high quality, supporting the 27 partition patterns in common between BC6H and ASTC, and fast to transcode with very little loss (typically a fraction of a dB PSNR) to the BC6H HDR texture format. Notably, **UASTC HDR 4x4 data is 100% standard ASTC texture data**, so no transcoding at all is required on devices or APIs that support ASTC HDR. This mode can also be transcoded to various 32-64bpp uncompressed HDR texture/image formats.

Here is the [UASTC HDR 4x4 specification document](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-4x4-Texture-Specification-v1.0), and here are some compressed [example images](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-Examples).

4. **ASTC HDR 6x6 or RDO ASTC HDR 6x6**: A 3.56 bits/pixel (or less with RDO+Zstd) HDR high quality mode. Just like mode #3, **ASTC HDR 6x6 data is 100% standard ASTC texture data**. Here's a [page with details](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-6x6-Support-Notes). The current encoder supports weight grid upsampling, 1-3 subsets, single or dual planes, CEM's 7 and 11, and all unique ASTC partition patterns.

The ASTC HDR decoder, used in the transcoder module, supports the entire ASTC HDR format.

5. **UASTC HDR 6x6 Intermediate ("GPU Photo HDR")**: A custom compressed intermediate format that can be rapidly transcoded to ASTC HDR 6x6, BC6H, and various uncompressed HDR formats. The custom compressed file format is [described here](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-6x6-Intermediate-File-Format-(Basis-GPU-Photo-6x6)). The format supports 75 unique ASTC configurations, weight grid upsampling, 1-3 subsets, single or dual planes, CEM's 7 and 11, and all unique ASTC partition patterns. One of the first HDR GPU texture codecs supporting the [delta E ITP (ICtCp) colorspace metric](https://www.portrait.com/resource-center/about-deltae-e/) and perceptual saliency maps.

6. **Standard ASTC LDR-4x4-12x12**. Supports all standard 14 ASTC block sizes. Transcodable from any ASTC block size to any other supported LDR texture format with adaptive deblocking, including BC7 using the [bc7f "one-shot" analytical BC7 encoder](https://github.com/BinomialLLC/basis_universal/wiki/Transcoder-Internals-Analytical-Real-Time-Encoders) (supporting all BC7 modes/features) and ETC1 (using etc1f, which also supports the entire ETC1 format).

The ASTC LDR decoder, used in the transcoder module, supports the entire standard ASTC LDR format (i.e. not just ASTC texture blocks generated using our encoder). The ASTC LDR transcoder can transcode any block size ASTC (4x4 - 12x12) to the other LDR texture formats.
 
7. **XUASTC LDR 4x4-12x12 ("GPU Photo LDR/SDR")**: Supercompressed ASTC with **Weight Grid DCT**, supporting all 14 standard ASTC block sizes, with adaptive deblocking when transcoding to other texture/pixel formats. Bitrates range from approximately 0.3–5.7 bpp, depending on content, profile, block size, windowed RDO, and Weight Grid DCT quality settings. Typical XUASTC LDR 4×4 (**8 bpp in memory**) transmission/on-disk bitrate with Weight Grid DCT (where it is least effective) is **1.15–3.5 bpp (typical ≈2.25 bpp)**, with larger block sizes achieving even lower usable bitrates, down to approximately 0.3 bpp. Like ASTC LDR, the XUASTC LDR transcoder can transcode any block size ASTC (4x4 - 12x12) to the other LDR texture formats, but with additional block-size specific optimizations.

Supports three profiles: context-based range/arithmetic coding (for higher compression ratios), Zstd (for faster and simpler transcoding), or a hybrid profile using both approaches. Transcodable to all other supported LDR texture formats, including fully featured (all 8 modes, all dual-plane channel configurations, all mode settings) BC7. Certain common block sizes (4×4, 6×6, and 8×6) have specializations for particularly fast transcoding directly to BC7, bypassing analytical BC7 encoding (using [bc7f](https://github.com/BinomialLLC/basis_universal/wiki/Transcoder-Internals-Analytical-Real-Time-Encoders)) entirely for the most common ASTC configurations (solid color and single-subset CEMs).

Weight Grid DCT can be disabled; however, supercompression remains available with optional, configurable windowed RDO. Compatible with all major image and texture content types, including photographic images, lightmaps, albedo/specular textures, various types of normal maps, luminance-only maps, and geospatial mapping signals.

Supports adaptive deblocking when transcoding from larger block sizes; this can be disabled using a transcoder flag.

One interesting use of XUASTC LDR which works with any of the 14 block sizes: the efficient distribution of texture content compressed to very low bitrates vs. older systems, resulting in game-changing download time reductions. Using the larger XUASTC block sizes (beyond 6x6) with Weight Grid DCT and adaptive deblocking on either the CPU or [GPU using a simple shader](https://github.com/BinomialLLC/basis_universal/tree/master/shader_deblocking), **any developer can now distribute texture and image content destined for BC7 at .35-1.5 bpp**, and **cache the transcoded BC7 data on a modern Gen 4 or 5 (10+ GB/sec.) SSD**.

XUASTC LDR supports the following ASTC configurations: L/LA/RGB/RGBA CEMs; base+scale or RGB/RGBA direct; base+offset CEMs; Blue Contraction encoding; 1–3 subsets; all partition patterns; and single- or dual-plane modes. Here is the [XUASTC LDR specification](https://github.com/BinomialLLC/basis_universal/wiki/XUASTC-LDR-Specification-v1.0). Also see the [ASTC and XUASTC LDR Usage Guide](https://github.com/BinomialLLC/basis_universal/wiki/ASTC-and-XUASTC-LDR-Usage-Guide).

Notes:  
- Mode #1 (ETC1S) has special support and optimizations for basic temporal supercompression ([texture video](https://github.com/BinomialLLC/basis_universal/wiki/Encoding-ETC1S-and-XUASTC-LDR-Texture-Video)).
- Modes #3 (UASTC HDR 4x4) and #4 (RDO ASTC HDR 6x6), and #6 (ASTC LDR 4x4-12x12) output 100% standard ASTC texture data (with or without RDO), like any other ASTC encoder. The .KTX2 files are just plain textures.
- The other modes (#1, #2, #5, #7) output compressed data in various custom supercompressed formats, which our transcoder library can convert in real-time to various GPU texture or pixel formats.
- Modes #4 (ASTC HDR 6x6) and #5 (UASTC HDR 6x6) internally use the same unified ASTC HDR 6x6 encoder.
- Modes #6 (ASTC LDR 4x4-12x12) and #7 (XUASTC LDR 4x4-12x12) internally use the same unified ASTC LDR ASTC encoder.

### Other Features

Both .basis and .KTX2 files support mipmap levels, texture arrays, cubemaps, cubemap arrays, and texture video, in all modes. Additionally, .basis files support non-uniform texture arrays, where each image in the file can have a different resolution or number of mipmap levels.

In ETC1S mode, the compressor is able to exploit color and pattern correlations across all the images in the entire file using global endpoint/selector codebooks, so multiple images with mipmaps can be stored efficiently in a single file. The ETC1S mode also supports skip blocks (Conditional Replenishment) for short video sequences, to prevent sending blocks which haven't changed relative to the previous frame.

The LDR image formats supported for reading are .PNG, [.DDS with mipmaps](https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide), .TGA, .QOI, and .JPG. The HDR image formats supported for reading are .EXR, .HDR, and .DDS with mipmaps. The library can write .basis, .KTX2, .DDS, .KTX (v1), .ASTC, .OUT, .EXR, and .PNG files.

The system now supports loading basic 2D .DDS files with optional mipmaps, but the .DDS file must be in one of the supported uncompressed formats: 24bpp RGB, 32bpp RGBA/BGRA, half-float RGBA, or float RGBA. Using .DDS files allows the user to control exactly how the mipmaps are generated before compression.

----

Building (Native)
-----------------

The encoding library and command line tool have no required 3rd party dependencies that are not already in the repo itself. The transcoder is a single .cpp source file (in `transcoder/basisu_transcoder.cpp`) which has no 3rd party dependencies.

We build and test under:
- Windows x86/x64 using Visual Studio 2026, MSVC or clang
- Windows ARM using Visual Studio 2022 ARM 17.13.0
- Ubuntu Linux 24.04.3 LTS (noble) with gcc 13.3.0 or clang 18.1.3 
- macOS (M1) with clang 16.0.0
- Arch Linux ARM, on a [Pinebook Pro](https://pine64.org/devices/pinebook_pro/), with gcc 12.1.
- Ubuntu Linux 24.04 on RISC-V (Orange PI RV2)
- cmake: 3.28.3, emcc: 4.0.19

Under Windows with Visual Studio you can use the included `basisu.sln` file. Alternatively, you can use cmake to create new VS solution/project files.

To build, first [install cmake](https://cmake.org/), then:

```
mkdir build
cd build
cmake ..
make
```

To build with SSE 4.1 support on x86/x64 systems (ETC1S encoding is roughly 15-30% faster), add `-DBASISU_SSE=TRUE` to the cmake command line. Add `-DBASISU_OPENCL=TRUE` to build with (optional) OpenCL support. Use `-DCMAKE_BUILD_TYPE=Debug` to build in debug. To build 32-bit executables, add `-DBASISU_BUILD_X64=FALSE`.

After building, the native command line tool used to create, validate, and transcode/unpack .KTX2/.basis files is `bin/basisu`.

*Note we use C++17 for compiling the software. Anything later is too new for us. Compiling the software with a newer C++ version is not supported by us yet.*

----

Running the Precompiled WASM WASI Executables
---------------------------------------------

For smaller images/textures (~4 megatexels or less), there are precompiled, secure, cross-platform 32-bit .WASM WASI executables checked into the `bin` directory: `basisu_mt.wasm` (multithreaded) and `basisu_st.wasm` (single threaded). Quick testing - ETC1S/UASTC LDR 4x4 (all platforms) - multithreaded and single threaded, using [wasmtime](https://wasmtime.dev/):

Tested with wasmtime v39.0.0:

```
cd bin
wasmtime run --dir=. --dir=../test_files --wasm threads=yes --wasi threads=yes ./basisu_mt.wasm -test
wasmtime run --dir=. --dir=../test_files ./basisu_st.wasm -test
```

For newer versions of wasmtime such as v42.0.1 add `--wasm shared-memory=yes`:

```
wasmtime run --dir=. --dir=../test_files --wasm threads=yes --wasm shared-memory=yes --wasi threads=yes ./basisu_mt.wasm -test
```

See the `runwt.sh`, `runwt.bat`, `runw.sh`, or `runw.bat` scripts for examples on how to run the WASM executables using wasmtime. Windows example for XUASTC LDR 6x6 compression using the arithmetic profile, with Weight Grid DCT level 70:

```
cd bin
runwt.bat ../test_files/tough.png -xuastc_ldr_6x6 -quality 70 -xuastc_arith
runwt.bat tough.ktx2
```

Linux/macOS:

```
cd bin
chmod +x runwt.sh
./runwt.sh ../test_files/tough.png -xuastc_ldr_6x6 -quality 70 -xuastc_arith
./runwt.sh tough.ktx2
```

Unfortunately, 32-bit WASM WASI executables have tradeoffs vs. native executables: Limited memory, and slower performance (somewhat mitigatable using WASM threading, which we support). **32-bit WASM WASI memory constraints limit the maximum image/texture size that can be compressed to ASTC LDR or XUASTC LDR to around 4 megapixels.** (The other codecs have lower memory requirements.) For Web, we support both WASM and WASM64 (with or without threading), which greatly improves the WASM memory situation. As far as we know as of 2/2026, wasmtime supports WASM64, but the WASI SDK still [doesn't officially support the wasm64-wasi target](https://github.com/WebAssembly/wasi-sdk/issues/212), but once it does we'll support it.

Building (WASM WASI)
--------------------

To build the WASM WASI executables, you will need the [WASM WASI SDK](https://github.com/WebAssembly/wasi-sdk) installed. The `WASI_SDK_PATH` environment variable must be set to the correct path where the SDK is installed. 

Multithreaded:
```
mkdir build_wasm_mt
cd build_wasm_mt
cmake -DCMAKE_TOOLCHAIN_FILE=$WASI_SDK_PATH/share/cmake/wasi-sdk-pthread.cmake -DCMAKE_BUILD_TYPE=Release -DBASISU_WASM_THREADING=ON ..
make
```

Single threaded:
```
mkdir build_wasm_st
cd build_wasm_st
cmake -DCMAKE_TOOLCHAIN_FILE=$WASI_SDK_PATH/share/cmake/wasi-sdk.cmake -DCMAKE_BUILD_TYPE=Release -DBASISU_WASM_THREADING=OFF ..
make
```

The WASM WASI executables will be placed in the `bin` directory. These platform-independent executables are fully functional, and can be executed using a WASM WASI runtime such as [wasmtime](https://github.com/bytecodealliance/wasmtime).

----

### Testing the Codec

The command line tool includes some automated LDR/HDR encoding/transcoding tests:

```
cd ../bin
basisu -test
basisu -test_hdr_4x4
basisu -test_hdr_6x6
basisu -test_hdr_6x6i
basisu -test_xuastc_ldr
```

To test the codec in OpenCL mode (must have OpenCL libs/headers/drivers installed and have compiled OpenCL support in by running cmake with `-DBASISU_OPENCL=TRUE`):

```
basisu -test -opencl
```

----

Compressing and Unpacking .KTX2/.basis Files
--------------------------------------------

- To compress an LDR sRGB PNG/QOI/TGA/JPEG/DDS image to a supercompressed XUASTC LDR 6x6 .KTX2 file, at quality level 75 (**valid quality levels 1-100, where higher values=higher quality**), effort level 4 (**valid effort levels 0-10, higher values=slower compression, default effort is 3**):

`basisu -xuastc_ldr_6x6 -quality 75 -effort 4 x.png`

`-quality 100` disables Weight Grid DCT, leaving just lossless supercompression of ASTC. An alias for `-xuastc_ldr_6x6` is `-ldr_6x6i` (where 'i'="intermediate"). All **[14 standard ASTC block sizes](https://developer.nvidia.com/astc-texture-compression-for-game-assets) are supported, from 4x4-12x12**: 4x4, 5x4, 5x5, 6x5, 6x6, 8x5, 8x6, 10x5, 10x6, 8x8, 10x8, 10x10, 12x10 and 12x12. The **XUASTC LDR to BC7 transcoder has special optimizations for several common block sizes: 4x4, 6x6 and 8x6**. When transcoding XUASTC LDR at these particular block sizes, most XUASTC blocks are *directly* transcoded to BC7 (i.e. directly from the XUASTC latent to the BC7 latent), skipping the real-time analytical bc7f encoding step.

More XUASTC LDR specific options (many of these also apply to standard ASTC - see our [ASTC/XUASTC Usage Guide](https://github.com/BinomialLLC/basis_universal/wiki/ASTC-and-XUASTC-LDR-Usage-Guide)):

  - The options `-xuastc_arith`, `-xuastc_zstd` (the default), and `-xuastc_hybrid` control the **XUASTC LDR profile used**. The arithmetic profile trades off transcoding throughput for roughly 5-18% better compression vs. the Zstd profile, and the hybrid profile is a balance between the two. 

  - `-ts` or `-srgb` enables the **sRGB profile (the default)**, and `-tl` or `-linear` **enables the linear profile**. Ideally this setting will match how the ASTC texture is sampled by the GPU. Use linear on normal maps.

  - `-weights X Y Z W` sets the unsigned integer **channel error weights**, used to favor certain channels during compression.

  - Another set of XUASTC specific options overrides the **windowed RDO behavior** (windowed or bounded RDO is a separate and optional perceptual optimization vs. Weight Grid DCT): `-xy` enables and `-xyd` disables windowed RDO. By default, if Weight Grid DCT is not enabled (i.e. `-quality` isn't specified, or is set to 100), windowed RDO is disabled. Windowed RDO is automatically enabled if the quality level is less than 100, unless `-xyd` is specified. Also see the tool's [help text](https://github.com/BinomialLLC/basis_universal/blob/master/cmd_help/cmd_help.txt) for additional windowed RDO options: `-ls_min_psnr`, `-ls_min_alpha_psnr`, `-ls_thresh_psnr`, `-ls_thresh_alpha_psnr`, etc.

  - `-xs` disables 2-3 subset usage, and `-xp` disables dual plane usage (slightly higher compression, faster direct transcoding to BC7 will occur more often)
  - `-higher_quality_transcoding`: Permits slower but higher quality transcoding
  - `-no_deblocking`: Disables adaptive deblocking on ASTC block sizes > 8x6 (faster)
  - `-force_deblocking`: Always use adaptive deblocking filter, even for block sizes <= 8x6 (slower)
  - `-stronger_deblocking`: Use stronger deblocking when it's enabled (same performance)
  - `-fast_xuastc_ldr_bc7_transcoding` and `-no_fast_xuastc_ldr_bc7_transcoding`: Controls faster direct XUASTC->BC7 transcoding (defaults to enabled, which is slightly lower quality)

- To compress an LDR sRGB image to a standard ASTC LDR 6x6 .KTX2 file, using effort level 4 (valid effort levels 0-10):

`basisu -astc_ldr_6x6 -effort 4 x.png`

An alias for `-astc_ldr_6x6` is `-ldr_6x6`. 

Just like XUASTC LDR, all 14 standard ASTC block sizes are supported, from 4x4-12x12. Internally the XUASTC LDR encoder is used, but standard ASTC block data is output, instead of supercompressed XUASTC LDR. Most XUASTC LDR options also work in ASTC LDR mode.

- To compress an LDR sRGB image to an ETC1S .KTX2 file, at quality level 100 (the highest):

`basisu -quality 100 x.png`

- For a linear LDR image, in ETC1S mode, at default quality (`-quality 50`, or the older `-q 128`):

`basisu -linear x.png`

- To compress to UASTC LDR 4x4, which is much higher quality than ETC1S, but lower maximum quality vs. ASTC/XUASTC LDR 4x4:

`basisu -uastc x.png`

- To compress an [.EXR](https://en.wikipedia.org/wiki/OpenEXR), [Radiance .HDR](https://paulbourke.net/dataformats/pic/), or .DDS HDR image to a UASTC HDR 4x4 .KTX2 file:

`basisu x.exr`

- To compress a standard ASTC HDR 6x6 file (~3.56 bpp):

```
basisu -hdr_6x6 x.exr  
basisu -hdr_6x6 -lambda 500 x.exr  
basisu -hdr_6x6_level 5 -lambda 500 x.exr
```

- To compress a UASTC HDR 6x6i file (using the compressed intermediate format) for smaller files (~1.75-3.0 bpp):

```
basisu -hdr_6x6i x.exr  
basisu -hdr_6x6i -lambda 500 x.exr  
basisu -hdr_6x6i_level 5 -lambda 500 x.exr
```

Note the unified `-quality` and `-effort` options work in HDR, too. These examples use the older non-unified options, which allow more direct/precise control.

Be aware that the .EXR reader we use is [TinyEXR's](https://github.com/syoyo/tinyexr), which doesn't support all possible .EXR compression modes. Tools like [ImageMagick](https://imagemagick.org/) can be used to create .EXR files that TinyEXR can read.

Alternatively, LDR images (such as .PNG) can be compressed to an HDR format by specifying `-hdr`, `-hdr_6x6`, or `-hdr_6x6i`. By default LDR images, when compressed to an HDR format, are first upconverted to HDR by converting them from sRGB to linear light and scaled to 100 [nits - candela per sq. meter, cd/m²](https://grokipedia.com/page/Candela_per_square_metre). The sRGB conversion step can be disabled by specifying `-hdr_ldr_no_srgb_to_linear`, and the normalized RGB linear light to nit multiplier can be changed by specifying `-hdr_ldr_upconversion_nit_multiplier X`.

Note: If you're compressing LDR/SDR image files to an HDR format, the codec's default behavior is to convert the 8-bit image data to linear light (by undoing the sRGB transfer function). It then multiplies the linear light RGB values by the LDR->HDR upconversion multiplier, which is in nits. In previous versions of the codec, this multiplier was effectively 1 nit, but it now defaults to 100 nits in all modes. (The typical luminance of LDR monitors is 80-100 nits.) To change this, use the "-hdr_ldr_upconversion_nit_multiplier X" command line option. (This is done because the HDR 6x6 codecs function internally in the [ICtCp HDR colorspace](https://en.wikipedia.org/wiki/ICtCp). LDR/SDR images must be upconverted to linear light HDR images scaled to a proper max. luminance based on how the image data will be displayed on actual SDR/HDR monitors.)

### Some Useful Command Line Options

- All codecs now support simple unified "quality" and "effort" settings. `-effort X` [0,10] controls how much of the search space (and how slowly) compression proceeds, and `-quality X` [1,100] controls the quality vs. bitrate tradeoff. Internally these settings will be mapped to each codec's specific configuration settings. Almost all the older settings still work, however. Previously, `-q X`, where X ranged from [1,255], controlled the ETC1S quality setting. This option is still available, but `-quality` is preferred now.

- `-debug` causes the encoder to print internal and developer-oriented verbose debug information.

- `-stats` to see various quality (PSNR) statistics. 

- `-linear`: ETC1S defaults to sRGB colorspace metrics, UASTC LDR currently always uses linear metrics, and UASTC HDR defaults to weighted RGB metrics (with 2,3,1 weights). If the input is a normal map, or some other type of non-sRGB (non-photographic) texture content, be sure to use `-linear` to avoid extra unnecessary artifacts. (Angular normal map metrics for UASTC LDR/HDR are definitely doable and on our TODO list.)

- Specifying `-opencl` enables OpenCL mode, which currently only accelerates ETC1S encoding if it's been enabled at compile time.

- The compressor is multithreaded by default, which can be disabled using the `-no_multithreading` command line option. The transcoder is currently single threaded, although it is thread safe (i.e. it supports decompressing multiple texture slices in parallel).

More Example Command Lines
--------------------------

- To compress an sRGB PNG/QOI/TGA/JPEG/DDS image to an RDO (Rate-Distortion Optimization) UASTC LDR .KTX2 file with mipmaps:

`basisu -uastc -uastc_rdo_l 1.0 -mipmap x.png`

`-uastc_rdo_l X` controls the RDO ([Rate-Distortion Optimization](https://en.wikipedia.org/wiki/Rate%E2%80%93distortion_optimization)) quality setting. The lower this value, the higher the quality, but the larger the compressed file size. Good values to try are between .2-3.0. The default is 1.0.

- To add automatically generated mipmaps to an ETC1S .KTX2 file:

`basisu -mipmap -quality 75 x.png`

There are several mipmap options to change the filter kernel, the filter colorspace for the RGB channels (linear vs. sRGB), the smallest mipmap dimension, etc. The tool also supports generating cubemap files, 2D/cubemap texture arrays, etc. To bypass the automatic mipmap generator, you can create LDR or HDR uncompressed [.DDS texture files](https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide) and feed them to the compressor.

- To create a slightly higher quality ETC1S .KTX2 file (one with higher quality endpoint/selector codebooks) at the default quality level (128) - note this is much slower to encode:

`basisu -comp_level 2 x.png`

On some rare images (ones with blue sky gradients come to mind), you may need to increase the ETC1S `-comp_level` setting, which ranges from 1 to 6. This controls the amount of overall effort the encoder uses to optimize the ETC1S codebooks and the compressed data stream. Higher -comp_level's are *significantly* slower. 

- To manually set the ETC1S codebook sizes (instead of using -quality, or the older -q options), with a higher codebook generation level (this is useful with texture video):

`basisu x.png -comp_level 2 -max_endpoints 16128 -max_selectors 16128`

- To [tonemap](https://en.wikipedia.org/wiki/Tone_mapping) an HDR .EXR or .HDR image file to multiple LDR .PNG files at different exposures, using the Reinhard tonemap operator:

`basisu -tonemap x.exr`

- To compare two LDR images and print PSNR statistics:

`basisu -compare a.png b.png`

- To compare two HDR .EXR/.HDR images and print FP16 PSNR statistics:

`basisu -compare_hdr a.exr b.exr`

See the help text for a complete listing of the tool's command line options. The command line tool is just a thin wrapper on top of the encoder library.

Unpacking .KTX2/.basis files to .PNG/.EXR/.KTX/.DDS files
---------------------------------------------------------

You can either use the command line tool or [call the transcoder directly](https://github.com/BinomialLLC/basis_universal/wiki/How-to-Use-and-Configure-the-Transcoder) from JavaScript or C/C++ code to decompress .KTX2/.basis files to GPU texture data or uncompressed image data. To unpack a .KTX2 or .basis file to multiple .png/.exr/.ktx/.dds files:

`basisu x.ktx2`

Use the `-no_ktx` and `-etc1_only`/`-format_only` options to unpack to less files. 

`-info` and `-validate` will just display file information and not output any files. 

The written mipmapped, cubemap, or texture array .KTX/.DDS files will be in a wide variety of compressed GPU texture formats (PVRTC1 4bpp, ETC1-2, BC1-5, BC7, etc.), and to our knowledge there is unfortunately (as of 2024) still no single .KTX or .DDS viewer tool that correctly and reliably supports every GPU texture format that we support. BC1-5 and BC7 files are viewable using AMD's Compressonator, ETC1/2 using Mali's Texture Compression Tool, and PVRTC1 using Imagination Tech's PVRTexTool. [RenderDoc](https://renderdoc.org/) has a useful texture file viewer for many formats. The macOS *Finder* app supports previewing .EXR, .ASTC and .KTX files in various GPU formats, including ASTC LDR/ HDR. The Windows 11 Explorer can preview .DDS files. The [online OpenHDR Viewer](https://viewer.openhdr.org/) is useful for viewing .EXR/.HDR image files. 

----

Pixel Shader Deblocking Sample: CPU + GPU Deblocking Everywhere
---------------------------------------------------------------

The [shader_deblocking sample](https://github.com/BinomialLLC/basis_universal/blob/master/shader_deblocking/README.md) in the repo demonstrates how to use a simple pixel shader to deblock sampled textures of any block size between 4x4-12x12, greatly reducing block artifacts. The sample shader is compatible with mipmapping and bilinear or trilinear filtering. Ultimately, shader deblocking enables the usage of larger ASTC block sizes, reducing bitrate and increasing transcoding speeds. Deblocking is a standard feature of modern image and video codecs, and there's no reason why it can't be used while sampling (or transcoding) GPU textures. Using larger ASTC block sizes can significantly reduce GPU memory bandwidth. If bandwidth is the bottleneck — as it often is — the modest ALU and texture sampling cost of deblocking can be effectively free.

XUASTC LDR's transcoder supports adaptive deblocking when transcoding to other (non-ASTC) formats like BC7, and GPU shader deblocking can be used for ASTC, resulting in a complete deblocking system for ASTC.

----

Python Support
--------------

All key encoder and all transcoder functionality is now available from Python, but this is still in the early stages of development. See the README files in the python directory for how to build the native SO's/PYD's. The Python support module supports both native and WASM modules, which is used as a fallback if native libraries can't be loaded. Python support has been tested under Ubuntu Linux and Windows 11 so far.

Example:
```
cd python
python3 -m tests.test_backend_loading
========== BACKEND LOADING TEST ==========

Testing native backend...
[Encoder] Using native backend
  [OK] Native backend loaded
Hello from basisu_wasm_api.cpp version 200
  Native get_version() ? 200
  Native alloc() returned ptr = 190977024
  Native free() OK
  [OK] Native basic operations working.

Testing WASM backend...
[WASM Encoder] Loaded: /mnt/c/dev/xuastc4/python/basisu_py/wasm/basisu_module_st.wasm
[Encoder] Using WASM backend
  [OK] WASM backend loaded
Hello from basisu_wasm_api.cpp version 200
  WASM get_version() ? 200
  WASM alloc() returned ptr = 26920160
  WASM free() OK
  [OK] WASM basic operations working.

========== DONE ==========
```

----

WebGL Examples
--------------

The 'WebGL' directory contains several simple WebGL demos that use the transcoder and compressor compiled to [WASM](https://webassembly.org/) with [Emscripten](https://emscripten.org/). These demos are online [here](https://subquantumtech.com/xu/). See more details in the readme file [here](webgl/README.md).

![Screenshot of 'texture' example running in a browser.](webgl/texture_test/preview.png)
![Screenshot of 'gltf' example running in a browser.](webgl/gltf/preview.png)
![Screenshot of 'encode_test' example running in a browser.](webgl/ktx2_encode_test/preview.png)

----

Building the WASM Modules with [Emscripten](https://emscripten.org/) 
--------------------------------------------------------------------

Both the transcoder and encoder may be compiled using Emscripten to WebAssembly and used on the web. A set of JavaScript wrappers to the codec, written in C++ with Emscripten extensions, is located in [`webgl/transcoding/basis_wrappers.cpp`](https://github.com/BinomialLLC/basis_universal/blob/master/webgl/transcoder/basis_wrappers.cpp). The JavaScript wrapper supports nearly all features and modes, including texture video. See the [README.md](https://github.com/BinomialLLC/basis_universal/tree/master/webgl) and CMakeLists.txt files in `webgl/transcoder` and `webgl/encoder`. 

To build the WASM transcoder, after installing Emscripten:

```
cd webgl/transcoder/build
emcmake cmake ..
make
```

To build the WASM encoder:

```
cd webgl/encoder/build
emcmake cmake ..
make
```

There are several simple encoding/transcoding web demos, located in `webgl/ktx2_encode_test` and `webgl/texture_test`, that show how to use the encoder's and transcoder's JavaScript wrapper APIs. They are [live on the web here](https://subquantumtech.com/xu/).

----

Low-level C++ Encoder/Transcoder API Examples
---------------------------------------------

Some simple examples showing how to directly call the C++ encoder and transcoder library APIs are in [`example/example.cpp`](https://github.com/BinomialLLC/basis_universal/blob/master/example/example.cpp).

----

Installation using the vcpkg dependency manager
-----------------------------------------------

You can download and install Basis Universal using the [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    vcpkg install basisu

The Basis Universal port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository. (9/10/2024: UASTC HDR support is not available here yet.)

---

Project Policies
----------------

See our wiki page: [Project Policies: PRs, compiler warnings, release cadence etc.](https://github.com/BinomialLLC/basis_universal/wiki/Project-Policies:-PR's,-compiler-warnings,-release-cadence,-etc.).

----

KTX2 Support Status
-------------------

Note as of March 2026 we are working with Khronos on the exact details of how we embed XUASTC LDR supercompressed texture data into the KTX2 file format. KTX2 texture files using our previous codecs (including the recently added UASTC HDR 4x4 and UASTC HDR 6x6i formats) can now be interchanged with other KTX2 tools. See our [KTX2 technical information document](https://github.com/BinomialLLC/basis_universal/wiki/KTX2-File-Format-Support-Technical-Details) for more info.

Whenever possible, we keep full introspection/transcode compatibility with all of our previously written KTX2 files, even if during standardization a file format change is made. We don't expect how we embed XUASTC LDR into KTX2 in basisu v2.1 to change.

----

Repository Licensing with REUSE
-------------------------------

The repository has been updated to be compliant with the REUSE license
checking tool (https://reuse.software/). See the [.reuse](https://github.com/BinomialLLC/basis_universal/tree/master/.reuse) subdirectory.

External Links
--------------

- [btx - KTX2 Command Line Tool](https://github.com/BinomialLLC/KTX-Software-Binomial-Fork) - Our fork of KTX-Software with bug fixes, working HDR quality/effort controls, new options, and new codec integrations. This tool can validate, extract, and compress KTX2 files compatible with our project.
- [ARM's astcenc](https://github.com/ARM-software/astc-encoder) - Crucial official tool from ARM which can unpack ASTC format LDR/HDR .astc and .ktx files to .png or .exr for testing and verification purposes.
- [Online .EXR and .HDR Image File Viewer](https://viewer.openhdr.org/) - OpenHDR Viewer. Has a very well implemented tone mapper, auto-exposure, and HDR histogram.
- [Windows HDR + WCG Image Viewer](https://13thsymphony.github.io/hdrimageviewer/) - A true HDR image viewer for Windows which works on HDR monitors. Also see [the github repo](https://github.com/13thsymphony/HDRImageViewer).
- [AMD Compressonator](https://gpuopen.com/compressonator/) - .DDS viewer, can view .KTX files in some formats.
- [PVRTexTool](https://www.imgtec.com/developers/powervr-sdk-tools/pvrtextool/) - Can view .ASTC and .KTX files in some formats. (Note: .DDS viewer seems busted for BC1, doesn't support BC7 at all.)
- [Microsoft's DirectXTex](https://github.com/microsoft/DirectXTex) - Samples contain a basic .DDS viewer. (Note: May still have issues loading .DDS files with texture dimensions that aren't divisible by 4 texels.)
- [RenderDoc](https://renderdoc.org/) - Reliable viewer for LDR/HDR .DDS files in BC1-7 formats.
- [Paint.NET](https://www.getpaint.net/) - Windows app: built-in [.DDS file loading](https://github.com/0xC0000054/pdn-ddsfiletype-plus), supports BC1-7 and [cubemaps](https://github.com/0xC0000054/pdn-ddsfiletype-plus/wiki/Cube-Maps)
- [Mali Texture Compression Tool](https://community.arm.com/support-forums/f/graphics-gaming-and-vr-forum/52390/announcement-mali-texture-compression-tool-end-of-life) - Now deprecated.
- [Our GitHub wiki content statically mirrored as HTML](https://subquantumtech.com/basisu_wiki/Home.html), which lags behind the [live GitHub wiki](https://github.com/BinomialLLC/basis_universal/wiki)
 
For more useful links, papers, and tools/libraries, see the end of the [UASTC HDR 4x4 texture specification](https://github.com/BinomialLLC/basis_universal/wiki/UASTC-HDR-4x4-Texture-Specification#papersfurther-reading).

----

E-mail: info @ binomial dot info, or contact us on [Twitter](https://twitter.com/_binomial)

Here's the [Sponsors](https://github.com/BinomialLLC/basis_universal/wiki/Sponsors-and-Supporters) wiki page.
