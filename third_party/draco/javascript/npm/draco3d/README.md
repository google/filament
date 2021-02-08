
<p align="center">
<img src="https://github.com/google/draco/raw/master/docs/DracoLogo.jpeg" />
</p>

News
=======

Check out the [README](https://github.com/google/draco/blob/1.4.1/README.md)
file for news about this release.

Description
===========

[Draco] is a library for compressing and decompressing 3D geometric [meshes] and
[point clouds]. It is intended to improve the storage and transmission of 3D
graphics.

Draco was designed and built for compression efficiency and speed. The code
supports compressing points, connectivity information, texture coordinates,
color information, normals, and any other generic attributes associated with
geometry. With Draco, applications using 3D graphics can be significantly
smaller without compromising visual fidelity. For users, this means apps can
now be downloaded faster, 3D graphics in the browser can load quicker, and VR
and AR scenes can now be transmitted with a fraction of the bandwidth and
rendered quickly.

[Draco] is released as C++/Javascript source code that can be used to compress 3D
graphics as well as decoders for the encoded data.

NPM Package
===========

The code shows a simple example of using Draco encoder and decoder with Node.js.
`draco_encoder_node.js` and `draco_decoder_node.js` are modified Javascript
encoding/decoding files that are compatible with Node.js.

How to run the code:

(1) Install draco3d package :

~~~~~ bash
$ npm install draco3d
~~~~~

### Working example
`draco_nodejs_example.js` has the example code for usage.

(2) Run example code to test:

~~~~~ bash
$ cp node_modules/draco3d/draco_nodejs_example.js .
$ cp node_modules/draco3d/bunny.drc .
$ node draco_nodejs_example.js
~~~~~

The code loads the [Bunny] model, it will first decode to a mesh
and then encode it with different settings.


### Minimal examples
These examples only show how to load the encoder and decoder modules. `draco_minimal_encoder_decoder_example.js`, `draco_minimal_encoder_example.js`, and `draco_minimal_decoder_example.js` has the example code for usage.

(2) Run the code to test:

~~~~~ bash
$ node draco_minimal_encoder_decoder_example.js
~~~~~

~~~~~ bash
$ node draco_minimal_encoder_example.js
~~~~~

~~~~~ bash
$ node draco_minimal_decoder_example.js
~~~~~

References
==========
[Draco]: https://github.com/google/draco
[meshes]: https://en.wikipedia.org/wiki/Polygon_mesh
[point clouds]: https://en.wikipedia.org/wiki/Point_cloud
[Bunny]: https://graphics.stanford.edu/data/3Dscanrep/

Bunny model from Stanford's graphic department <https://graphics.stanford.edu/data/3Dscanrep/>
