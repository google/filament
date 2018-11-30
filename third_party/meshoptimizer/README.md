# meshoptimizer [![Build Status](https://travis-ci.org/zeux/meshoptimizer.svg?branch=master)](https://travis-ci.org/zeux/meshoptimizer) [![codecov.io](http://codecov.io/github/zeux/meshoptimizer/coverage.svg?branch=master)](http://codecov.io/github/zeux/meshoptimizer?branch=master) ![MIT](https://img.shields.io/badge/license-MIT-blue.svg)

## Purpose

When GPU renders triangle meshes, various stages of the GPU pipeline have to process vertex and index data. The efficiency of these stages depends on the data you feed to them; this library provides algorithms to help optimize meshes for these stages, as well as algorithms to reduce the mesh complexity and storage overhead.

The library provides a C and C++ interface for all algorithms; you can use it from C/C++ or from other languages via FFI (such as P/Invoke). If you want to use this library from Rust, you should use [meshopt crate](https://crates.io/crates/meshopt).

## Building

meshoptimizer is distributed as a set of C++ source files. To include it into your project, you can use one of the two options:

* Use CMake to build the library (either as a standalone project or as part of your project)
* Add source files to your project's build system

The source files are organized in such a way that you don't need to change your build-system settings, and you only need to add the files for the algorithms you use.

## Pipeline

When optimizing a mesh, you should typically feed it through a set of optimizations (the order is important!):

1. Indexing
2. Vertex cache optimization
3. Overdraw optimization
4. Vertex fetch optimization
5. Vertex quantization
6. (optional) Vertex/index buffer compression

## Indexing

Most algorithms in this library assume that a mesh has a vertex buffer and an index buffer. For algorithms to work well and also for GPU to render your mesh efficiently, the vertex buffer has to have no redundant vertices; you can generate an index buffer from an unindexed vertex buffer or reindex an existing (potentially redundant) index buffer as follows:

First, generate a remap table from your existing vertex (and, optionally, index) data:

```c++
size_t index_count = face_count * 3;
std::vector<unsigned int> remap(index_count); // allocate temporary memory for the remap table
size_t vertex_count = meshopt_generateVertexRemap(&remap[0], NULL, index_count, &unindexed_vertices[0], index_count, sizeof(Vertex));
```

Note that in this case we only have an unindexed vertex buffer; the remap table is generated based on binary equivalence of the input vertices, so the resulting mesh will render the same way.

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

After all of the above optimizations, the geometry data is optimal for GPU to consume - however, you don't have to store the data as is. In case storage size or transmission bandwidth is of importance, you might want to compress vertex and index data. While several mesh compression libraries, like Google Draco, are available, they typically are designed to maximize the compression ratio at the cost of disturbing the vertex/index order (which makes the meshes inefficient to render on GPU) or decompression performance. Additionally they frequently don't support custom game-ready quantized vertex formats and thus require to re-quantize the data after loading it, introducing extra quantization errors and making decoding slower.

Alternatively you can use general purpose compression libraries like zstd or Oodle to compress vertex/index data - however these compressors aren't designed to exploit redundancies in vertex/index data and as such compression rates can be unsatisfactory.

To that end, this library provides algorithms to "encode" vertex and index data. The result of the encoding is generally significantly smaller than initial data, and remains compressible with general purpose compressors - so you can either store encoded data directly (for modest compression ratios and maximum decoding performance), or further compress it with zstd et al, to maximize compression rate.

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
int resib = meshopt_decodeIndexBuffer(indices, index_count, &buffer[0], buffer.size());
assert(resvb == 0 && resib == 0);
```

Note that vertex encoding assumes that vertex buffer was optimized for vertex fetch, and that vertices are quantized; index encoding assumes that the vertex/index buffers were optimized for vertex cache and vertex fetch. Feeding unoptimized data into the encoders will produce poor compression rates. Both codecs are lossless - the only lossy step is quantization that happens before encoding.

Decoding functions are heavily optimized and can directly target write-combined memory; you can expect both decoders to run at 1-2 GB/s on modern desktop CPUs. Compression ratios depend on the data; vertex data compression ratio is typically around 2-4x (compared to already quantized data), index data compression ratio is around 5-6x (compared to raw 16-bit index data). General purpose lossless compressors can further improve on these results.

## Triangle strip conversion

On most hardware, indexed triangle lists are the most efficient way to drive the GPU. However, in some cases triangle strips might prove beneficial:

- On some older GPUs, triangle strips may be a bit more efficient to render
- On extremely memory constrained systems, index buffers for triangle strips could save a bit of memory

This library provides an algorithm for converting a vertex cache optimized triangle list to a triangle strip:

```c++
std::vector<unsigned int> strip(meshopt_stripifyBound(index_count));
size_t strip_size = meshopt_stripify(&strip[0], indices, index_count, vertex_count);
```

Typically you should expect triangle strips to have ~50-60% of indices compared to triangle lists (~1.5-1.8 indices per triangle) and have ~5% worse ACMR. Note that triangle strips require restart index support for rendering; using degenerate triangles to connect strips is not supported.

## Efficiency analyzers

While the only way to get precise performance data is to measure performance on the target GPU, it can be valuable to measure the impact of these optimization in a GPU-independent manner. To this end, the library provides analyzers for all three major optimization routines. For each optimization there is a corresponding analyze function, like `meshopt_analyzeOverdraw`, that returns a struct with statistics.

`meshopt_analyzeVertexCache` returns vertex cache statistics. The common metric to use is ACMR - average cache miss ratio, which is the ratio of the total number of vertex invocations to the triangle count. The worst-case ACMR is 3 (GPU has to process 3 vertices for each triangle); on regular grids the optimal ACMR approaches 0.5. On real meshes it usually is in [0.5..1.5] range depending on the amount of vertex splits. One other useful metric is ATVR - average transformed vertex ratio - which represents the ratio of vertex shader invocations to the total vertices, and has the best case of 1.0 regardless of mesh topology (each vertex is transformed once).

`meshopt_analyzeVertexFetch` returns vertex fetch statistics. The main metric it uses is overfetch - the ratio between the number of bytes read from the vertex buffer to the total number of bytes in the vertex buffer. Assuming non-redundant vertex buffers, the best case is 1.0 - each byte is fetched once.

`meshopt_analyzeOverdraw` returns overdraw statistics. The main metric it uses is overdraw - the ratio between the number of pixel shader invocations to the total number of covered pixels, as measured from several different orthographic cameras. The best case for overdraw is 1.0 - each pixel is shaded once.

Note that all analyzers use approximate models for the relevant GPU units, so the numbers you will get as the result are only a rough approximation of the actual performance.

## License

This library is available to anybody free of charge, under the terms of MIT License (see LICENSE.md).
