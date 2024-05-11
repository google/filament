# cmgen

`cmgen` is a command-line tool for generating SH and mipmap levels from an env map.
Cubemaps and equirectangular formats are both supported, automatically detected according to the aspect ratio of the source image.

The tool can consume a HDR environment map in latlong format (equirectilinear) as well as "cross" cubemap formats (vertical and horizontal), and row/column cubemap formats, and can produce a mipmapped IBL(Image Based Lighting) or a blurry skybox or both.

## Usage

```shell
cmgen [options] <input-file>
cmgen [options] <uv[N]>
```

## Supported input formats

- PNG, 8 and 16 bits
- Radiance (.hdr)
- Photoshop (.psd), 16 and 32 bits
- OpenEXR (.exr)

## Options

```
--help, -h
    Print this message
--license
    Print copyright and license information
--quiet, -q
    Quiet mode. Suppress all non-error output
--type=[cubemap|equirect|octahedron|ktx], -t [cubemap|equirect|octahedron|ktx]
    Specify output type (default: cubemap)
--format=[exr|hdr|psd|rgbm|rgb32f|png|dds|ktx], -f [format]
    Specify output file format. ktx implies -type=ktx.
    KTX files are always KTX1 files, not KTX2.
    They are encoded with 3-channel RGB_10_11_11_REV data
--compression=COMPRESSION, -c COMPRESSION
    Format specific compression:
        KTX: ignored
        PNG: Ignored
        PNG RGBM: Ignored
        Radiance: Ignored
        Photoshop: 16 (default), 32
        OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)
        DDS: 8, 16 (default), 32
--size=power-of-two, -s power-of-two
    Size of the output cubemaps (base level), 256 by default
    Also applies to DFG LUT
--deploy=dir, -x dir
    Generate everything needed for deployment into <dir>
--extract=dir
    Extract faces of the cubemap into <dir>
--extract-blur=roughness
    Blurs the cubemap before saving the faces using the roughness blur
--clamp
    Clamp environment before processing
--no-mirror
    Skip mirroring of generated cubemaps (for assets with mirroring already backed in)
--ibl-samples=numSamples
    Number of samples to use for IBL integrations (default 1024)
--ibl-ld=dir
    Roughness pre-filter into <dir>
--sh-shader
    Generate irradiance SH for shader code
```
