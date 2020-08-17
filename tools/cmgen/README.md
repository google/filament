# cmgen

`cmgen` is a command-line tool for generating SH and mipmap levels from an env map.
Cubemaps and equirectangular formats are both supported, automatically detected according to the aspect ratio of the source image.

The tool consumes a HDR environment map in latlong format, and produces two cubemap files: a mipmapped IBL(Image Based Lighting) and a blurry skybox.

## Usage

```
$ cmgen [options] <input-file>
$ cmgen [options] <uv[N]>
```

## Supported input formats

- PNG, 8 and 16 bits
- Radiance (.hdr)
- Photoshop (.psd), 16 and 32 bits
- OpenEXR (.exr)

## Options

- --license  
	Print copyright and license information
- --quiet, -q  
	Quiet mode. Suppress all non-error output
- --type=[cubemap|equirect|octahedron|ktx], -t [cubemap|equirect|octahedron|ktx]  
	Specify output type (default: cubemap)
- --format=[exr|hdr|psd|rgbm|rgb32f|png|dds|ktx], -f [exr|hdr|psd|rgbm|rgb32f|png|dds|ktx]  
	Specify output file format. ktx implies -type=ktx.  
	KTX files are always encoded with 3-channel RGB_10_11_11_REV data
- --compression=COMPRESSION, -c COMPRESSION  
	Format specific compression:  
	    KTX:  
	        astc_[fast|thorough]_[ldr|hdr]_WxH, where WxH is a valid block size  
	        s3tc_rgba_dxt5  
	        etc_FORMAT_METRIC_EFFORT  
	            FORMAT is rgb8_alpha, srgb8_alpha, rgba8, or srgb8_alpha8  
	            METRIC is rgba, rgbx, rec709, numeric, or normalxyz  
	            EFFORT is an integer between 0 and 100  
	    PNG: Ignored  
	    PNG RGBM: Ignored  
	    Radiance: Ignored  
    	Photoshop: 16 (default), 32  
	    OpenEXR: RAW, RLE, ZIPS, ZIP, PIZ (default)  
	    DDS: 8, 16 (default), 32
- --size=power-of-two, -s power-of-two  
	Size of the output cubemaps (base level), 256 by default  
	Also aplies to DFG LUT  
- --deploy=dir, -x dir  
	Generate everything needed for deployment into <dir>  
- --extract=dir  
	Extract faces of the cubemap into <dir>  
- --extract-blur=roughness  
	Blurs the cubemap before saving the faces using the roughness blur  
- --clamp  
	Clamp environment before processing  
- --no-mirror  
	Skip mirroring of generated cubemaps (for assets with mirroring already backed in)  
- --ibl-samples=numSamples  
	Number of samples to use for IBL integrations (default 1024)  
- --ibl-ld=dir  
	Roughness pre-filter into <dir>  
- --sh-shader  
	Generate irradiance SH for shader code  

Private use only:  
- --ibl-dfg=filename.[exr|hdr|psd|png|rgbm|rgb32f|dds|h|hpp|c|cpp|inc|txt]  
	Compute the IBL DFG LUT  
- --ibl-dfg-multiscatter  
	If --ibl-dfg is set, computes the DFG for multi-scattering GGX  
- --ibl-dfg-cloth  
	If --ibl-dfg is set, adds a 3rd channel to the DFG for cloth shading  
- --ibl-is-mipmap=dir  
	Generate mipmap for pre-filtered importance sampling  
- --ibl-irradiance=dir  
	Diffuse irradiance into <dir>  
- --ibl-no-prefilter  
	Use importance sampling instead of prefiltered importance sampling  
- --ibl-min-lod-size  
	Minimum LOD size [default: 16]  
- --sh=bands  
	SH decomposition of input cubemap  
- --sh-output=filename.[exr|hdr|psd|rgbm|rgb32f|png|dds|txt]  
	SH output format. The filename extension determines the output format  
- --sh-irradiance, -i  
	Irradiance SH coefficients  
- --sh-window=cutoff|no|auto (default), -w cutoff|no|auto (default)  
	SH windowing to reduce ringing  
- --debug, -d  
	Generate extra data for debugging  
