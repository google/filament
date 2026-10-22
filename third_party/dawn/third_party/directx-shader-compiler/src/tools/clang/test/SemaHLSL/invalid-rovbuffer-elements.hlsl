// RUN: %dxc -Tlib_6_3 -verify %s

// RasterizerOrderedBuffer has suficiently different errors that it requires its own verifiction test.

struct MyFloat4 {
  float4 f;
};

struct MyFloat {
  float f;
};

// Incorrect number of parameters
RasterizerOrderedBuffer              romtbuf;/* expected-error {{requires template arguments}} */
RasterizerOrderedBuffer<>            romtbufb;/* expected-error {{too few template arguments for class template}} */
RasterizerOrderedBuffer<float, bool> p2buf;/* expected-error {{too many template arguments for class template}} */

// Oversize parameters
RasterizerOrderedBuffer<uint64_t3> u3buf;/* expected-error {{elements of typed buffers and textures must fit in four 32-bit quantities}} */
RasterizerOrderedBuffer<int64_t3>  i3buf;/* expected-error {{elements of typed buffers and textures must fit in four 32-bit quantities}} */
RasterizerOrderedBuffer<double3>   d4buf;/* expected-error {{elements of typed buffers and textures must fit in four 32-bit quantities}} */

// Structs, arrays, and matrices.
RasterizerOrderedBuffer<MyFloat>  sf1buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
RasterizerOrderedBuffer<MyFloat4> sf4buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
RasterizerOrderedBuffer<float[1]> af1buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
RasterizerOrderedBuffer<float[4]> af4buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
RasterizerOrderedBuffer<float1x1> mf1buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
RasterizerOrderedBuffer<float4x4> mf4buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */

// TODO: add too long vectors here
