
<p align="center">
<img src="https://github.com/google/draco/raw/master/docs/DracoLogo.jpeg" />
</p>

Description - glTF Draco Mesh Compression Extension
===================================================


[Draco] is a library for compressing and decompressing 3D geometric [meshes] and [point clouds]. It is intended to improve the storage and transmission of 3D graphics.
The [GL Transmission Format (glTF)](https://github.com/KhronosGroup/glTF) is an API-neutral runtime asset delivery format. glTF bridges the gap between 3D content creation tools and modern 3D applications by providing an efficient, extensible, interoperable format for the transmission and loading of 3D content.

This package is a build for encoding/decoding [Draco mesh compression extension](https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_draco_mesh_compression) in glTF specification. It could be used to compress the meshes in glTF assets or to decode the buffer data that belongs to a Draco mesh compression extension. For more detail, please read the extension spec.

Draco github glTF branch URL: https://github.com/google/draco/tree/gltf_2.0_draco_extension

News
=======
### Version 1.3.6 release
* WASM and JavaScript decoders are now hosted from a static URL
  * It is recommended to always pull your Draco WASM and JavaScript decoders from this URL:
  * https://www.gstatic.com/draco/v1/decoders/
  * Users will benefit from having the Draco decoder in cache as more sites start using the static URL
* Changed web examples to pull Draco decoders from static URL
* Added new API to Draco WASM decoder, which increased performance by ~15%
* Decreased Draco WASM decoder size by ~20%
* Added support for generic and multiple attributes to Draco Unity plug-ins
* Added new API to Draco Unity, which increased decoder performance by ~15%
* Changed quantization defaults:
  * POSITION: 11
  * NORMAL: 7
  * TEX_COORD: 10
  * COLOR: 8
  * GENERIC: 8
* Code cleanup
* Bug fixes

### Version 1.3.5 release
* Added option to build Draco for Universal Scene Description
* Code cleanup
* Bug fixes

### Version 1.3.4 release
* Fixes for Unity

### Version 1.3.3 release
* Added ExpertEncoder to the JavaScript API
  * Allows developers to set quantization options per attribute id
* Bug fixes

### Version 1.3.2 release
* Bug fixes

### Version 1.3.1 release
* Fix issue with multiple attributes when skipping an attribute transform

### Version 1.3.0 release
* Improved kD-tree based point cloud encoding
  * Now applicable to point clouds with any number of attributes
  * Support for all integer attribute types and quantized floating point types
* Improved mesh compression up to 10% (on average ~2%)
  * For meshes, the 1.3.0 bitstream is fully compatible with 1.2.x decoders
* Improved JavaScript API
  * Added support for all signed and unsigned integer types
  * Added support for point clouds to our JavaScript encoder API
* Added support for integer properties to the PLY decoder
* Bug fixes

NPM Package
===========

The code shows a simple example of using Draco encoder and decoder with Node.js.
`draco_encoder_node.js` and `draco_decoder_node.js` are modified Javascript
encoding/decoding files that are compatible with Node.js.
`draco_nodejs_example.js` has the example code for usage.
Here we use a Draco file as an example, but when it's used with glTF assets, the
Draco file should be instead some buffer data contained in the binary data.

How to run the code:

(1) Install draco3dgltf package :

~~~~~ bash
$ npm install draco3dgltf
~~~~~

(2) Run example code to test:

~~~~~ bash
$ cp node_modules/draco3dgltf/draco_nodejs_example.js .
$ cp node_modules/draco3dgltf/bunny.drc .
$ node draco_nodejs_example.js
~~~~~

The code loads the [Bunny] model, it will first decode to a mesh
and then encode it with different settings.

glTF Extension
==============

The above example shows how to decode compressed data from a binary file. To use with glTF assets. The decoder should be applied to the data of the `bufferView` that belongs to a Draco extension. Please see the spec for detailed instruction on loading/exporting Draco extension.

References
==========
[Draco]: https://github.com/google/draco
[Bunny]: https://graphics.stanford.edu/data/3Dscanrep/

Bunny model from Stanford's graphic department <https://graphics.stanford.edu/data/3Dscanrep/>
