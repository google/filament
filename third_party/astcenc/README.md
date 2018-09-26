# About

This is the official repository of the evaluation codec for the Adaptive Scalable Texture Compression (ASTC) standard.

ASTC technology developed by ARM® and AMD has been adopted as an official extension to both the Open GL® and OpenGL ES graphics APIs. ASTC is a major step forward in terms of image quality, reducing memory bandwidth and thus energy use.

The **ASTC Evaluation Codec** is a command-line executable that compresses and decompresses images using this texture compression standard.

Running on a standard PC, this tool allows content developers to evaluate the impressive quality and size benefits offered by this texture compression format, in advance of it being available in GPU  hardware.

# License #
By downloading the packages below you ackowledge that you accept the End User Licence Agreement for the ATSC Codec.
See [license.txt](license.txt) file for details.

# Compilation from Sources
* You can use GCC >= 4.6 and standard GNU tools to perform make-driven compilation on Linux and/or MacOS systems.
* You can use Visual Studio 2013 to compile for Windows systems (using solution file from `Source/win32-2013/astcenc` directory).

# ASTC Features & Benefits

ASTC offers a number of advantages over existing texture compression schemes:

* Flexibility, with bit rates from 8 bits per pixel (bpp) down to less than 1 bpp. This allows content developers to fine-tune the tradeoff of space against quality.
* Support for 1 to 4 color channels, together with modes for uncorrelated channels for use in mask textures and normal maps.
* Support for both low dynamic range (LDR) and high dynamic range (HDR) images.
* Support for both 2D and 3D images.
* Interoperability: Developers can choose any combination of features that suits their needs.

ASTC specification includes two profiles: LDR and Full. The smaller LDR Profile supports 2D low dynamic range images only. It is designed to be easy to integrate with existing hardware designs that already deal with compressed 2D images in other formats. The LDR Profile is a strict subset of the Full Profile, which also includes the 3D textures and high dynamic range support.

## Encoding

* Compression of PNG, TGA and KTX into ASTC format
* Control of bit rate from 0.89 bits/pixel upto 8bits/pixel in fine steps
* Control of compression time/quality tradeoff with exhaustive, thorough, medium, fast and very fast encoding speeds
* Supports both low (LDR) and high dynamic range (HDR) images
* Supports 3D texture compression
* Measure PSNR between input and output

## Decoding

* Decodes ASTC images to TGA or KTX

# Package Contents

* Source code
* Full specification of the ASTC data format
* Binaries for Windows, Mac OS X and Linux

# Getting Started
 
First, accept the [license](license.txt) and download the source code. The `Binary` subdirectory contains executable binaries for Windows, Mac OS X, and Linux. If you are running on another system, you might like to try compiling from source.
 
Open a terminal, change to the appropriate directory for your system, and run the astcenc encoder program, like this on Linux or Mac OS:
 
    ./astcenc
 
Or like this on Windows:
 
    astcenc
 
Invoking the tool with no arguments gives a very extensive help message, including usage instructions, and details of all the possible options.
 
## How do I run the tool?
 
First, find a 24-bit .png or .tga file you wish to use, say `/images/example.png` (or on Windows `C:\images\example.png`).
 
You can compress it using the `-c` option, like this (on Linux or Mac OS X):
 
    ./astcenc -c /images/example.png /images/example-compressed.astc 6x6 -medium

and on Windows:

    astcenc -c C:\images\example.png C:\images\example-compressed.astc 6x6 -medium
 
The `-c` indicates a compression operation, followed by the input and output filenames. The block footprint size follows, in this case 6x6 pixels, then the requested compression speed, medium.
 
To decompress the file again, you should use:
 
    ./astcenc -d /images/example-compressed.astc /images/example-decompressed.tga
    
and on Windows:

    astcenc -d C:\images\example-compressed.astc C:\images\example-decompressed.tga
 
The `-d` indicates decompression, followed by the input and output filenames. The output file will be an uncompressed TGA image.
 
If you just want to test what compression and decompression are like, use the test mode:
 
    ./astcenc -t /images/example.png /images/example-decompressed.tga 6x6 -medium

and on Windows:

    astcenc -t C:\images\example.png C:\images\example-compressed.tga 6x6 -medium
 
This is equivalent to compressing and then immediately decompressing again, and it also prints out statistics about the fidelity of the resulting image, using the peak signal-to-noise ratio.
 
## Experimenting
 
The block footprints go from 4x4 (8 bits per pixel) all the way up to 12x12 (0.89 bits/pixel). Like any lossy codec, such as JPEG there will come a point where selecting too aggressive a compression results in inacceptable quality loss, and ASTC is no exception. Finding this optimum balance between size and quality is one place where ASTC excels since its compression ratio is adjustable in much finer steps than other texture codecs.
 
The compression speed runs from `-veryfast`, through `-fast`, `-medium` and `-thorough`, up to `-exhaustive`. In general, the more time the encoder has to spend looking for good encodings, the better the results.

# Support
Please submit your questions and issues to the [ARM Mali Graphics forums](http://community.arm.com/groups/arm-mali-graphics).

- - - 
_Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved._
