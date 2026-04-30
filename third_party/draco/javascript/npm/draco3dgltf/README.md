
<p align="center">
<img src="https://github.com/google/draco/raw/master/docs/DracoLogo.jpeg" />
</p>

Description - glTF Draco Mesh Compression Extension
===================================================

The `draco3dgltf` package is a subset of the `draco3d` package, containing only
features of the Draco library that are relevant to compression in the glTF file
format. glTF files (`.gltf`, `.glb`) can contain Draco-compressed mesh geometry,
as defined by the glTF extension `KHR_draco_mesh_compression`.

This library does not directly read/write glTF files, but is intended for use
within tools and applications that deal with the glTF format. Examples of tools
using the Draco library to apply compression to glTF files include:

[Blender](https://www.blender.org/),
[glTF Transform](https://gltf-transform.donmccurdy.com/), and
[glTF Pipeline](https://github.com/CesiumGS/gltf-pipeline).

Draco github glTF branch URL: https://github.com/google/draco/tree/gltf_2.0_draco_extension

News
=======

Check out the [README](https://github.com/google/draco/blob/1.5.7/README.md)
file for news about this release.

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
