WebGPU does not include a trivial way to create mipmaps, and thus we must provide a render pass or compute pass to generate them.

This directory is the result of porting https://github.com/JolifantoBambla/webgpu-spd to C++, which itself is a WebGPU port of https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/blob/main/docs/samples/single-pass-downsampler.md. 

The first version of our port is primarily from a Gemini "Convert this to C++" request, which worked as an MVP with very few changes. Future commits will likely alter this version significantly, but it is being merged in this early state to facilitate rapid iteration. 