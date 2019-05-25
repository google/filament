Examples
========

Denoise
-------

A minimal working example demonstrating how to use Open Image Denoise can be
found at `examples/denoise.cpp`, which uses the C++11 convenience wrappers of
the C99 API.

This example is a simple command-line application that denoises the provided
image, which can optionally have auxiliary feature images as well (e.g. albedo
and normal). The images must be stored in the [Portable
FloatMap](http://www.pauldebevec.com/Research/HDR/PFM/) (PFM) format, and the
color values must be encoded in little-endian format.

Running `./denoise` without any arguments will bring up a list of command line
options.

