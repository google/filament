// RUN: %dxc -DRD=RW -DRES=Buffer -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=   -DRES=Buffer -Tlib_6_3 -verify %s

// RUN: %dxc -DRD=RW -DRES=Texture1D -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=   -DRES=Texture1D -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=RW -DRES=Texture2D -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=   -DRES=Texture2D -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=RW -DRES=Texture3D -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=   -DRES=Texture3D -Tlib_6_3 -verify %s

// RUN: %dxc -DRD=RW -DRES=Texture1DArray -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=   -DRES=Texture1DArray -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=RW -DRES=Texture2DArray -Tlib_6_3 -verify %s
// RUN: %dxc -DRD=   -DRES=Texture2DArray -Tlib_6_3 -verify %s

struct MyFloat4 {
  float4 f;
};

struct MyFloat {
  float f;
};

#define PASTE_(x,y) x##y
#define PASTE(x,y) PASTE_(x,y)
#define TYPE PASTE(RD,RES)

// Incorrect number of parameters
RES              romtbuf;/* no error */
RES<>            romtbufb;/* no error */
PASTE(RW,RES)    rwmtbuf;/* expected-error {{requires template arguments}} */
PASTE(RW,RES)<>  rwmtbufb;/* expected-error {{too few template arguments for class template}} */
TYPE<float, bool> p2buf;/* expected-error {{too many template arguments for class template}} */

// Oversize parameters
TYPE<uint64_t3> u3buf;/* expected-error {{elements of typed buffers and textures must fit in four 32-bit quantities}} */
TYPE<int64_t3>  i3buf;/* expected-error {{elements of typed buffers and textures must fit in four 32-bit quantities}} */
TYPE<double3>   d4buf;/* expected-error {{elements of typed buffers and textures must fit in four 32-bit quantities}} */

// Structs, arrays, and matrices.
TYPE<MyFloat>  sf1buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
TYPE<MyFloat4> sf4buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
TYPE<float[1]> af1buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
TYPE<float[4]> af4buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
TYPE<float1x1> mf1buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */
TYPE<float4x4> mf4buf;/* expected-error {{elements of typed buffers and textures must be scalars or vectors}} */

// TODO: add too long vectors here
