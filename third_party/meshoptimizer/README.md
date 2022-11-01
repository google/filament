# 🐇 meshoptimizer [![Actions Status](https://github.com/zeux/meshoptimizer/workflows/build/badge.svg)](https://github.com/zeux/meshoptimizer/actions) [![codecov.io](https://codecov.io/github/zeux/meshoptimizer/coverage.svg?branch=master)](https://codecov.io/github/zeux/meshoptimizer?branch=master) ![MIT](https://img.shields.io/badge/license-MIT-blue.svg) [![GitHub](https://img.shields.io/badge/repo-github-green.svg)](https://github.com/zeux/meshoptimizer)

## Purpose

When a GPU renders triangle meshes, various stages of the GPU pipeline have to process vertex and index data. The efficiency of these stages depends on the data you feed to them; this library provides algorithms to help optimize meshes for these stages, as well as algorithms to reduce the mesh complexity and storage overhead.

The library provides a C and C++ interface for all algorithms; you can use it from C/C++ or from other languages via FFI (such as P/Invoke). If you want to use this library from Rust, you should use [meshopt crate](https://crates.io/crates/meshopt).

[gltfpack](gltf), which is a tool that can automatically optimize glTF files, is developed and distributed alongside the library.

## Installing

meshoptimizer is hosted on GitHub; you can download the latest release using git:

```
git clone -b v0.18 https://github.com/zeux/meshoptimizer.git
```

Alternatively you can [download the .zip archive from GitHub](https://github.com/zeux/meshoptimizer/archive/v0.18.zip).

The library is also available as a package ([ArchLinux](https://aur.archlinux.org/packages/meshoptimizer/), [Debian](https://packages.debian.org/libmeshoptimizer), [Ubuntu](https://packages.ubuntu.com/libmeshoptimizer), [Vcpkg](https://github.com/microsoft/vcpkg/tree/master/ports/meshoptimizer)).

### Installing gltfpack

`gltfpack` is a CLI tool for optimizing meshes using meshoptimizer.

You can download a pre-built binary for gltfpack on [Releases page](https://github.com/zeux/meshoptimizer/releases), or install [npm package](https://www.npmjs.com/package/gltfpack). Native binaries are recommended over npm since they can work with larger files, run faster, and support texture compression.

[Learn more about gltfpack](./gltf/README.md)

## Building

meshoptimizer is distributed as a set of C++ source files. To include it into your project, you can use one of the two options:

* Use CMake to build the library (either as a standalone project or as part of your project)
* Add source files to your project's build system

The source files are organized in such a way that you don't need to change your build-system settings, and you only need to add the files for the algorithms you use.

## Installing from vcpkg

The meshoptimizer port in [vcpkg](https://github.com/Microsoft/vcpkg) is kept up to date by Microsoft team members and community contributors. You can download and install meshoptimizer using the vcpkg dependency manager, [Getting Started](https://github.com/microsoft/vcpkg#getting-started).

## Pipeline

When optimizing a mesh, you should typically feed it through a set of optimizations (the order is important!):

1. Indexing
2. (optional, discussed last) Simplification
3. Vertex cache optimization
4. Overdraw optimization
5. Vertex fetch optimization
6. Vertex quantization
7. (optional) Vertex/index buffer compression

## Indexing

Most algorithms in this library assume that a mesh has a vertex buffer and an index buffer. For algorithms to work well and also for GPU to render your mesh efficiently, the vertex buffer has to have no redundant vertices; you can generate an index buffer from an unindexed vertex buffer or reindex an existing (potentially redundant) index buffer as follows:

First, generate a remap table from your existing vertex (and, optionally, index) data:

```c++
size_t index_count = face_count * 3;
std::vector<unsigned int> remap(index_count); // allocate temporary memory for the remap table
size_t vertex_count = meshopt_generateVertexRemap(&remap[0], NULL, index_count, &unindexed_vertices[0], index_count, sizeof(Vertex));
```

Note that in this case we only have an unindexed vertex buffer; the remap table is generated based on binary equivalence of the input vertices, so the resulting mesh will render the same way. Binary equivalence considers all input bytes, including padding which should be zero-initialized if the vertex structure has gaps.

After generating the remap table, you can allocate space for the target vertex buffer (`vertex_count` elements) and index buffer (`index_count` elements) and generate them:

```c++
meshopt_remapIndexBuffer(indices, NULL, index_count, &remap[0]);
meshopt_remapVertexBuffer(vertices, &unindexed_vertices[0], index_count, sizeof(Vertex), &remap[0]);
```

You can then further optimize the resulting buffers by calling the other functions on them in-place.

## Vertex cache optimization

When the GPU renders the mesh, it has to run the vertex shader for each vertex; usually GPUs have a built-in fixed size cache that stores the transformed vertices (the result of running the vertex shader), and uses this cache to reduce the number of vertex shader invocations. This cache is usually small, 16-32 vertices, and can have different replacement policies; to use this cache efficiently, you have to reorder your triangles to maximize the locality of reused vertex references like so:

```c++
meshopt_optimizeVertexCache(indices, indices, index_count, vertex_count);
```

## Overdraw optimization

After transforming the vertices, GPU sends the triangles for rasterization which results in generating pixels that are usually first ran through the depth test, and pixels that pass it get the pixel shader executed to generate the final color. As pixel shaders get more expensive, it becomes more and more important to reduce overdraw. While in general improving overdraw requires view-dependent operations, this library provides an algorithm to reorder triangles to minimize the overdraw from all directions, which you should run after vertex cache optimization like this:

```c++
meshopt_optimizeOverdraw(indices, indices, index_count, &vertices[0].x, vertex_count, sizeof(Vertex), 1.05f);
```

The overdraw optimizer needs to read vertex positions as a float3 from the vertex; the code snippet above assumes that the vertex stores position as `float x, y, z`.

When performing the overdraw optimization you have to specify a floating-point threshold parameter. The algorithm tries to maintain a balance between vertex cache efficiency and overdraw; the threshold determines how much the algorithm can compromise the vertex cache hit ratio, with 1.05 meaning that the resulting ratio should be at most 5% worse than before the optimization.

## Vertex fetch optimization

After the final triangle order has been established, we still can optimize the vertex buffer for memory efficiency. Before running the vertex shader GPU has to fetch the vertex attributes from the vertex buffer; the fetch is usually backed by a memory cache, and as such optimizing the data for the locality of memory access is important. You can do this by running this code:

To optimize the index/vertex buffers for vertex fetch efficiency, call:

```c++
meshopt_optimizeVertexFetch(vertices, indices, index_count, vertices, vertex_count, sizeof(Vertex));
```

This will reorder the vertices in the vertex buffer to try to improve the locality of reference, and rewrite the indices in place to match; if the vertex data is stored using multiple streams, you should use `meshopt_optimizeVertexFetchRemap` instead. This optimization has to be performed on the final index buffer since the optimal vertex order depends on the triangle order.

Note that the algorithm does not try to model cache replacement precisely and instead just orders vertices in the order of use, which generally produces results that are close to optimal.

## Vertex quantization

To optimize memory bandwidth when fetching the vertex data even further, and to reduce the amount of memory required to store the mesh, it is often beneficial to quantize the vertex attributes to smaller types. While this optimization can technically run at any part of the pipeline (and sometimes doing quantization as the first step can improve indexing by merging almost identical vertices), it generally is easier to run this after all other optimizations since some of them require access to float3 positions.

Quantization is usually domain specific; it's common to quantize normals using 3 8-bit integers but you can use higher-precision quantization (for example using 10 bits per component in a 10_10_10_2 format), or a different encoding to use just 2 components. For positions and texture coordinate data the two most common storage formats are half precision floats, and 16-bit normalized integers that encode the position relative to the AABB of the mesh or the UV bounding rectangle.

The number of possible combinations here is very large but this library does provide the building blocks, specifically functions to quantize floating point values to normalized integers, as well as half-precision floats. For example, here's how you can quantize a normal:

```c++
unsigned int normal =
	(meshopt_quantizeUnorm(v.nx, 10) << 20) |
	(meshopt_quantizeUnorm(v.ny, 10) << 10) |
	 meshopt_quantizeUnorm(v.nz, 10);
```

and here's how you can quantize a position:

```c++
unsigned short px = meshopt_quantizeHalf(v.x);
unsigned short py = meshopt_quantizeHalf(v.y);
unsigned short pz = meshopt_quantizeHalf(v.z);
```

## Vertex/index buffer compression

In case storage size or transmission bandwidth is of importance, you might want to additionally compress vertex and index data. While several mesh compression libraries, like Google Draco, are available, they typically are designed to maximize the compression ratio at the cost of disturbing the vertex/index order (which makes the meshes inefficient to render on GPU) or decompression performance. They also frequently don't support custom game-ready quantized vertex formats and thus require to re-quantize the data after loading it, introducing extra quantization errors and making decoding slower.

Alternatively you can use general purpose compression libraries like zstd or Oodle to compress vertex/index data - however these compressors aren't designed to exploit redundancies in vertex/index data and as such compression rates can be unsatisfactory.

To that end, this library provides algorithms to "encode" vertex and index data. The result of the encoding is generally significantly smaller than initial data, and remains compressible with general purpose compressors - so you can either store encoded data directly (for modest compression ratios and maximum decoding performance), or further compress it with zstd/Oodle to maximize compression ratio.

> Note: this compression scheme is available as a glTF extension [EXT_meshopt_compression](https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Vendor/EXT_meshopt_compression/README.md).

To encode, you need to allocate target buffers (preferably using the worst case bound) and call encoding functions:

```c++
std::vector<unsigned char> vbuf(meshopt_encodeVertexBufferBound(vertex_count, sizeof(Vertex)));
vbuf.resize(meshopt_encodeVertexBuffer(&vbuf[0], vbuf.size(), vertices, vertex_count, sizeof(Vertex)));

std::vector<unsigned char> ibuf(meshopt_encodeIndexBufferBound(index_count, vertex_count));
ibuf.resize(meshopt_encodeIndexBuffer(&ibuf[0], ibuf.size(), indices, index_count));
```

You can then either serialize `vbuf`/`ibuf` as is, or compress them further. To decode the data at runtime, call decoding functions:

```c++
int resvb = meshopt_decodeVertexBuffer(vertices, vertex_count, sizeof(Vertex), &vbuf[0], vbuf.size());
int resib = meshopt_decodeIndexBuffer(indices, index_count, &ibuf[0], ibuf.size());
assert(resvb == 0 && resib == 0);
```

Note that vertex encoding assumes that vertex buffer was optimized for vertex fetch, and that vertices are quantized; index encoding assumes that the vertex/index buffers were optimized for vertex cache and vertex fetch. Feeding unoptimized data into the encoders will produce poor compression ratios. Both codecs are lossless - the only lossy step is quantization that happens before encoding.

To reduce the data size further, it's recommended to use `meshopt_optimizeVertexCacheStrip` instead of `meshopt_optimizeVertexCache` when optimizing for vertex cache, and to use new index codec version (`meshopt_encodeIndexVersion(1)`). This trades off some efficiency in vertex transform for smaller vertex and index data.

Decoding functions are heavily optimized and can directly target write-combined memory; you can expect both decoders to run at 1-3 GB/s on modern desktop CPUs. Compression ratios depend on the data; vertex data compression ratio is typically around 2-4x (compared to already quantized data), index data compression ratio is around 5-6x (compared to raw 16-bit index data). General purpose lossless compressors can further improve on these results.

Index buffer codec only supports triangle list topology; when encoding triangle strips or line lists, use `meshopt_encodeIndexSequence`/`meshopt_decodeIndexSequence` instead. This codec typically encodes indices into ~1 byte per index, but compressing the results further with a general purpose compressor can improve the results to 1-3 bits per index.

The following guarantees on data compatibility are provided for point releases (*no* guarantees are given for development branch):

- Data encoded with older versions of the library can always be decoded with newer versions;
- Data encoded with newer versions of the library can be decoded with older versions, provided that encoding versions are set correctly; if binary stability of encoded data is important, use `meshopt_encodeVertexVersion` and `meshopt_encodeIndexVersion` to 'pin' the data versions.

Due to a very high decoding performance and compatibility with general purpose lossless compressors, the compression is a good fit for the use on the web. To that end, meshoptimizer provides both vertex and index decoders compiled into WebAssembly and wrapped into a module with JavaScript-friendly interface, `js/meshopt_decoder.js`, that you can use to decode meshes that were encoded offline:

```js
// ready is a Promise that is resolved when (asynchronous) WebAssembly compilation finishes
await MeshoptDecoder.ready;

// decode from *Data (Uint8Array) into *Buffer (Uint8Array)
MeshoptDecoder.decodeVertexBuffer(vertexBuffer, vertexCount, vertexSize, vertexData);
MeshoptDecoder.decodeIndexBuffer(indexBuffer, indexCount, indexSize, indexData);
```

[Usage example](https://meshoptimizer.org/demo/) is available, with source in `demo/index.html`; this example uses .GLB files encoded using `gltfpack`.

## Point cloud compression

The vertex encoding algorithms can be used to compress arbitrary streams of attribute data; one other use case besides triangle meshes is point cloud data. Typically point clouds come with position, color and possibly other attributes but don't have an implied point order.

To compress point clouds efficiently, it's recommended to first preprocess the points by sorting them using the spatial sort algorithm:

```c++
std::vector<unsigned int> remap(point_count);
meshopt_spatialSortRemap(&remap[0], positions, point_count, sizeof(vec3));

// for each attribute stream
meshopt_remapVertexBuffer(positions, positions, point_count, sizeof(vec3), &remap[0]);
```

After this the resulting arrays should be quantized (e.g. using 16-bit fixed point numbers for positions and 8-bit color components), and the result can be compressed using `meshopt_encodeVertexBuffer` as described in the previous section. To decompress, `meshopt_decodeVertexBuffer` will recover the quantized data that can be used directly or converted back to original floating-point data. The compression ratio depends on the nature of source data, for colored points it's typical to get 35-40 bits per point as a result.

## Triangle strip conversion

On most hardware, indexed triangle lists are the most efficient way to drive the GPU. However, in some cases triangle strips might prove beneficial:

- On some older GPUs, triangle strips may be a bit more efficient to render
- On extremely memory constrained systems, index buffers for triangle strips could save a bit of memory

This library provides an algorithm for converting a vertex cache optimized triangle list to a triangle strip:

```c++
std::vector<unsigned int> strip(meshopt_stripifyBound(index_count));
unsigned int restart_index = ~0u;
size_t strip_size = meshopt_stripify(&strip[0], indices, index_count, vertex_count, restart_index);
```

Typically you should expect triangle strips to have ~50-60% of indices compared to triangle lists (~1.5-1.8 indices per triangle) and have ~5% worse ACMR.
Note that triangle strips can be stitched with or without restart index support. Using restart indices can result in ~10% smaller index buffers, but on some GPUs restart indices may result in decreased performance.

To reduce the triangle strip size further, it's recommended to use `meshopt_optimizeVertexCacheStrip` instead of `meshopt_optimizeVertexCache` when optimizing for vertex cache. This trades off some efficiency in vertex transform for smaller index buffers.

## Deinterleaved geometry

All of the examples above assume that geometry is represented as a single vertex buffer and a single index buffer. This requires storing all vertex attributes - position, normal, texture coordinate, skinning weights etc. - in a single contiguous struct. However, in some cases using multiple vertex streams may be preferable. In particular, if some passes require only positional data - such as depth pre-pass or shadow map - then it may be beneficial to split it from the rest of the vertex attributes to make sure the bandwidth use during these passes is optimal. On some mobile GPUs a position-only attribute stream also improves efficiency of tiling algorithms.

Most of the functions in this library either only need the index buffer (such as vertex cache optimization) or only need positional information (such as overdraw optimization). However, several tasks require knowledge about all vertex attributes.

For indexing, `meshopt_generateVertexRemap` assumes that there's just one vertex stream; when multiple vertex streams are used, it's necessary to use `meshopt_generateVertexRemapMulti` as follows:

```c++
meshopt_Stream streams[] = {
    {&unindexed_pos[0], sizeof(float) * 3, sizeof(float) * 3},
    {&unindexed_nrm[0], sizeof(float) * 3, sizeof(float) * 3},
    {&unindexed_uv[0], sizeof(float) * 2, sizeof(float) * 2},
};

std::vector<unsigned int> remap(index_count);
size_t vertex_count = meshopt_generateVertexRemapMulti(&remap[0], NULL, index_count, index_count, streams, sizeof(streams) / sizeof(streams[0]));
```

After this `meshopt_remapVertexBuffer` needs to be called once for each vertex stream to produce the correctly reindexed stream.

Instead of calling `meshopt_optimizeVertexFetch` for reordering vertices in a single vertex buffer for efficiency, calling `meshopt_optimizeVertexFetchRemap` and then calling `meshopt_remapVertexBuffer` for each stream again is recommended.

Finally, when compressing vertex data, `meshopt_encodeVertexBuffer` should be used on each vertex stream separately - this allows the encoder to best utilize corellation between attribute values for different vertices.

## Simplification

All algorithms presented so far don't affect visual appearance at all, with the exception of quantization that has minimal controlled impact. However, fundamentally the most effective way at reducing the rendering or transmission cost of a mesh is to make the mesh simpler.

This library provides two simplification algorithms that reduce the number of triangles in the mesh. Given a vertex and an index buffer, they generate a second index buffer that uses existing vertices in the vertex buffer. This index buffer can be used directly for rendering with the original vertex buffer (preferably after vertex cache optimization), or a new compact vertex/index buffer can be generated using `meshopt_optimizeVertexFetch` that uses the optimal number and order of vertices.

The first simplification algorithm, `meshopt_simplify`, follows the topology of the original mesh in an attempt to preserve attribute seams, borders and overall appearance. For meshes with inconsistent topology or many seams, such as faceted meshes, it can result in simplifier getting "stuck" and not being able to simplify the mesh fully; it's recommended to preprocess the index buffer with `meshopt_generateShadowIndexBuffer` to discard any vertex attributes that aren't critical and can be rebuilt later such as normals.

```c++
float threshold = 0.2f;
size_t target_index_count = size_t(index_count * threshold);
float target_error = 1e-2f;

std::vector<unsigned int> lod(index_count);
float lod_error = 0.f;
lod.resize(meshopt_simplify(&lod[0], indices, index_count, &vertices[0].x, vertex_count, sizeof(Vertex),
    target_index_count, target_error, &lod_error));
```

Target error is an approximate measure of the deviation from the original mesh using distance normalized to 0..1 (so 1e-2f means that simplifier will try to maintain the error to be below 1% of the mesh extents). Note that because of topological restrictions and error bounds simplifier isn't guaranteed to reach the target index count and can stop earlier.

The second simplification algorithm, `meshopt_simplifySloppy`, doesn't follow the topology of the original mesh. This means that it doesn't preserve attribute seams or borders, but it can collapse internal details that are too small to matter better because it can merge mesh features that are topologically disjoint but spatially close.

```c++
float threshold = 0.2f;
size_t target_index_count = size_t(index_count * threshold);
float target_error = 1e-1f;

std::vector<unsigned int> lod(index_count);
float lod_error = 0.f;
lod.resize(meshopt_simplifySloppy(&lod[0], indices, index_count, &vertices[0].x, vertex_count, sizeof(Vertex),
    target_index_count, target_error, &lod_error));
```

This algorithm will not stop early due to topology restrictions but can still do so if target index count can't be reached without introducing an error larger than target. It is 5-6x faster than `meshopt_simplify` when simplification ratio is large, and is able to reach ~20M triangles/sec on a desktop CPU (`meshopt_simplify` works at ~3M triangles/sec).

When a sequence of LOD meshes is generated that all use the original vertex buffer, care must be taken to order vertices optimally to not penalize mobile GPU architectures that are only capable of transforming a sequential vertex buffer range. It's recommended in this case to first optimize each LOD for vertex cache, then assemble all LODs in one large index buffer starting from the coarsest LOD (the one with fewest triangles), and call `meshopt_optimizeVertexFetch` on the final large index buffer. This will make sure that coarser LODs require a smaller vertex range and are efficient wrt vertex fetch and transform.

Both algorithms can also return the resulting normalized deviation that can be used to choose the correct level of detail based on screen size or solid angle; the error can be converted to world space by multiplying by the scaling factor returned by `meshopt_simplifyScale`.

## Mesh shading

Modern GPUs are beginning to deviate from the traditional rasterization model. NVidia GPUs starting from Turing and AMD GPUs starting from RDNA2 provide a new programmable geometry pipeline that, instead of being built around index buffers and vertex shaders, is built around mesh shaders - a new shader type that allows to provide a batch of work to the rasterizer.

Using mesh shaders in context of traditional mesh rendering provides an opportunity to use a variety of optimization techniques, starting from more efficient vertex reuse, using various forms of culling (e.g. cluster frustum or occlusion culling) and in-memory compression to maximize the utilization of GPU hardware. Beyond traditional rendering mesh shaders provide a richer programming model that can synthesize new geometry more efficiently than common alternatives such as geometry shaders. Mesh shading can be accessed via Vulkan or Direct3D 12 APIs; please refer to [Introduction to Turing Mesh Shaders](https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/) and [Mesh Shaders and Amplification Shaders: Reinventing the Geometry Pipeline](https://devblogs.microsoft.com/directx/coming-to-directx-12-mesh-shaders-and-amplification-shaders-reinventing-the-geometry-pipeline/) for additional information.

To use mesh shaders for conventional rendering efficiently, geometry needs to be converted into a series of meshlets; each meshlet represents a small subset of the original mesh and comes with a small set of vertices and a separate micro-index buffer that references vertices in the meshlet. This information can be directly fed to the rasterizer from the mesh shader. This library provides algorithms to create meshlet data for a mesh, and - assuming geometry is static - can compute bounding information that can be used to perform cluster culling, a technique that can reject a meshlet if it's invisible on screen.

To generate meshlet data, this library provides two algorithms - `meshopt_buildMeshletsScan`, which creates the meshlet data using a vertex cache-optimized index buffer as a starting point by greedily aggregating consecutive triangles until they go over the meshlet limits, and `meshopt_buildMeshlets`, which doesn't depend on any other algorithms and tries to balance topological efficiency (by maximizing vertex reuse inside meshlets) with culling efficiency (by minimizing meshlet radius and triangle direction divergence). `meshopt_buildMeshlets` is recommended in cases when the resulting meshlet data will be used in cluster culling algorithms.

```c++
const size_t max_vertices = 64;
const size_t max_triangles = 124;
const float cone_weight = 0.0f;

size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles);
std::vector<meshopt_Meshlet> meshlets(max_meshlets);
std::vector<unsigned int> meshlet_vertices(max_meshlets * max_vertices);
std::vector<unsigned char> meshlet_triangles(max_meshlets * max_triangles * 3);

size_t meshlet_count = meshopt_buildMeshlets(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), indices.data(),
    indices.size(), &vertices[0].x, vertices.size(), sizeof(Vertex), max_vertices, max_triangles, cone_weight);
```

To generate the meshlet data, `max_vertices` and `max_triangles` need to be set within limits supported by the hardware; for NVidia the values of 64 and 124 are recommended. `cone_weight` should be left as 0 if cluster cone culling is not used, and set to a value between 0 and 1 to balance cone culling efficiency with other forms of culling like frustum or occlusion culling.

Each resulting meshlet refers to a portion of `meshlet_vertices` and `meshlet_triangles` arrays; this data can be uploaded to GPU and used directly after trimming:

```c++
const meshopt_Meshlet& last = meshlets[meshlet_count - 1];

meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
meshlets.resize(meshlet_count);
```

However depending on the application other strategies of storing the data can be useful; for example, `meshlet_vertices` serves as indices into the original vertex buffer but it might be worthwhile to generate a mini vertex buffer for each meshlet to remove the extra indirection when accessing vertex data, or it might be desirable to compress vertex data as vertices in each meshlet are likely to be very spatially coherent.

After generating the meshlet data, it's also possible to generate extra data for each meshlet that can be saved and used at runtime to perform cluster culling, where each meshlet can be discarded if it's guaranteed to be invisible. To generate the data, `meshlet_computeMeshletBounds` can be used:

```c++
meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshlet_vertices[m.vertex_offset], &meshlet_triangles[m.triangle_offset],
    m.triangle_count, &vertices[0].x, vertices.size(), sizeof(Vertex));
```

The resulting `bounds` values can be used to perform frustum or occlusion culling using the bounding sphere, or cone culling using the cone axis/angle (which will reject the entire meshlet if all triangles are guaranteed to be back-facing from the camera point of view):

```c++
if (dot(normalize(cone_apex - camera_position), cone_axis) >= cone_cutoff) reject();
```

## Efficiency analyzers

While the only way to get precise performance data is to measure performance on the target GPU, it can be valuable to measure the impact of these optimization in a GPU-independent manner. To this end, the library provides analyzers for all three major optimization routines. For each optimization there is a corresponding analyze function, like `meshopt_analyzeOverdraw`, that returns a struct with statistics.

`meshopt_analyzeVertexCache` returns vertex cache statistics. The common metric to use is ACMR - average cache miss ratio, which is the ratio of the total number of vertex invocations to the triangle count. The worst-case ACMR is 3 (GPU has to process 3 vertices for each triangle); on regular grids the optimal ACMR approaches 0.5. On real meshes it usually is in [0.5..1.5] range depending on the amount of vertex splits. One other useful metric is ATVR - average transformed vertex ratio - which represents the ratio of vertex shader invocations to the total vertices, and has the best case of 1.0 regardless of mesh topology (each vertex is transformed once).

`meshopt_analyzeVertexFetch` returns vertex fetch statistics. The main metric it uses is overfetch - the ratio between the number of bytes read from the vertex buffer to the total number of bytes in the vertex buffer. Assuming non-redundant vertex buffers, the best case is 1.0 - each byte is fetched once.

`meshopt_analyzeOverdraw` returns overdraw statistics. The main metric it uses is overdraw - the ratio between the number of pixel shader invocations to the total number of covered pixels, as measured from several different orthographic cameras. The best case for overdraw is 1.0 - each pixel is shaded once.

Note that all analyzers use approximate models for the relevant GPU units, so the numbers you will get as the result are only a rough approximation of the actual performance.

## Memory management

Many algorithms allocate temporary memory to store intermediate results or accelerate processing. The amount of memory allocated is a function of various input parameters such as vertex count and index count. By default memory is allocated using `operator new` and `operator delete`; if these operators are overloaded by the application, the overloads will be used instead. Alternatively it's possible to specify custom allocation/deallocation functions using `meshopt_setAllocator`, e.g.

```c++
meshopt_setAllocator(malloc, free);
```

> Note that the library expects the allocation function to either throw in case of out-of-memory (in which case the exception will propagate to the caller) or abort, so technically the use of `malloc` above isn't safe. If you want to handle out-of-memory errors without using C++ exceptions, you can use `setjmp`/`longjmp` instead.

Vertex and index decoders (`meshopt_decodeVertexBuffer`, `meshopt_decodeIndexBuffer`, `meshopt_decodeIndexSequence`) do not allocate memory and work completely within the buffer space provided via arguments.

All functions have bounded stack usage that does not exceed 32 KB for any algorithms.

## License

This library is available to anybody free of charge, under the terms of MIT License (see LICENSE.md).
