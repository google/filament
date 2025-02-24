// RUN: %clang_cc1 -fsyntax-only -Wno-unused-value -ffreestanding -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 indexing-operator.hlsl

Buffer g_b;
StructuredBuffer<float4> g_sb;
Texture1D g_t1d;
Texture1DArray g_t1da;
Texture2D g_t2d;
Texture2DArray g_t2da;
Texture2D g_t2a2 [17];
// fxc error X3000: syntax error: unexpected token 'g_t2dms_err'
//Texture2DMS g_t2dms_err; // expected-error {{too few template arguments for class template 'Texture2DMS'}} fxc-error {{X3000: syntax error: unexpected token 'g_t2dms_err'}} 
Texture2DMS<float4, 8> g_t2dms;
// fxc error X3000: syntax error: unexpected token 'g_t2dmsa_err'
//Texture2DMSArray g_t2dmsa_err; // expected-error {{too few template arguments for class template 'Texture2DMSArray'}} fxc-error {{X3000: syntax error: unexpected token 'g_t2dmsa_err'}} 
Texture2DMSArray<float4, 8> g_t2dmsa;
Texture3D g_t3d;
TextureCube g_tc;
TextureCubeArray g_tca;

RWStructuredBuffer<float4> g_rw_sb;
// fxc error X3000: syntax error: unexpected token 'g_rw_b_err'
//RWBuffer g_rw_b_err; // expected-error {{too few template arguments for class template 'RWBuffer'}} fxc-error {{X3000: syntax error: unexpected token 'g_rw_b_err'}} 
RWBuffer<float4> g_rw_b;
// fxc error X3000: syntax error: unexpected token 'g_rw_t1d_err'
//RWTexture1D g_rw_t1d_err; // expected-error {{too few template arguments for class template 'RWTexture1D'}} fxc-error {{X3000: syntax error: unexpected token 'g_rw_t1d_err'}} 
RWTexture1D<float4> g_rw_t1d;
// fxc error X3000: syntax error: unexpected token 'g_rw_t1da_err'
//RWTexture1DArray g_rw_t1da_err; // expected-error {{too few template arguments for class template 'RWTexture1DArray'}} fxc-error {{X3000: syntax error: unexpected token 'g_rw_t1da_err'}} 
RWTexture1DArray<float4> g_rw_t1da;
// fxc error X3000: syntax error: unexpected token 'g_rw_t2d_err'
//RWTexture2D g_rw_t2d_err; // expected-error {{too few template arguments for class template 'RWTexture2D'}} fxc-error {{X3000: syntax error: unexpected token 'g_rw_t2d_err'}} 
RWTexture2D<float4> g_rw_t2d;
// fxc error X3000: syntax error: unexpected token 'g_rw_t2da_err'
//RWTexture2DArray g_rw_t2da_err; // expected-error {{too few template arguments for class template 'RWTexture2DArray'}} fxc-error {{X3000: syntax error: unexpected token 'g_rw_t2da_err'}} 
RWTexture2DArray<float4> g_rw_t2da;
// fxc error X3000: syntax error: unexpected token 'g_rw_t3d_err'
//RWTexture3D g_rw_t3d_err; // expected-error {{too few template arguments for class template 'RWTexture3D'}} fxc-error {{X3000: syntax error: unexpected token 'g_rw_t3d_err'}} 
RWTexture3D<float4> g_rw_t3d;

// No such thing as these:
// RWTexture2DMS g_rw_t2dms;
// RWTexture2DMSArray g_rw_t2dmsa;
// RWTextureCube g_rw_tc;
// RWTextureCubeArray g_rw_tca;

float test_vector_indexing()
{
  float4 f4 = { 1, 2, 3, 4 };
  float f = f4[0];
  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f = f4[0][1];
  return f;
}

float4 test_scalar_indexing()
{
  float4 f4 = 0;
  f4 += g_b[0];
  f4 += g_t1d[0];
  f4 += g_sb[0];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t1da[0]; // expected-error {{no viable overloaded operator[] for type 'Texture1DArray<>'}} expected-note {{no known conversion from 'int' to 'uint2'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120 : invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2d[0]; // expected-error {{no viable overloaded operator[] for type 'Texture2D<>'}} expected-note {{no known conversion from 'int' to 'uint2'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120 : invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2da[0]; // expected-error {{no viable overloaded operator[] for type 'Texture2DArray<>'}} expected-note {{no known conversion from 'int' to 'uint3'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120 : invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2dms[0]; // expected-error {{no viable overloaded operator[] for type 'Texture2DMS<float4, 8>'}} expected-note {{no known conversion from 'int' to 'uint2'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120 : invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2dmsa[0]; // expected-error {{no viable overloaded operator[] for type 'Texture2DMSArray<float4, 8>'}} expected-note {{no known conversion from 'int' to 'uint3'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120 : invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t3d[0]; // expected-error {{no viable overloaded operator[] for type 'Texture3D<>'}} expected-note {{no known conversion from 'int' to 'uint3'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc  error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f4 += g_tc[0]; // expected-error {{type 'TextureCube<>' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  // fxc  error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f4 += g_tca[0]; // expected-error {{type 'TextureCubeArray<>' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  return f4;
}

float4 test_vector2_indexing()
{
  int2 offset = { 1, 2 };
  float4 f4 = 0;
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_b[offset]; // expected-error {{no viable overloaded operator[] for type 'Buffer<>'}} expected-note {{no known conversion from 'int2' to 'uint1'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t1d[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture1D<>'}} expected-note {{no known conversion from 'int2' to 'uint1'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_sb[offset]; // expected-error {{no viable overloaded operator[] for type 'StructuredBuffer<float4>'}} expected-note {{no known conversion from 'int2' to 'uint1'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t1da[offset];
  f4 += g_t2d[offset];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2da[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture2DArray<>'}} expected-note {{no known conversion from 'int2' to 'uint3'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2dms[offset];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2dmsa[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture2DMSArray<float4, 8>'}} expected-note {{no known conversion from 'int2' to 'uint3'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t3d[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture3D<>'}} expected-note {{no known conversion from 'int2' to 'uint3'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f4 += g_tc[offset]; // expected-error {{type 'TextureCube<>' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f4 += g_tca[offset]; // expected-error {{type 'TextureCubeArray<>' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  return f4;
}

float4 test_vector3_indexing()
{
  int3 offset = { 1, 2, 3 };
  float4 f4 = 0;
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_b[offset]; // expected-error {{no viable overloaded operator[] for type 'Buffer<>'}} expected-note {{no known conversion from 'int3' to 'uint1'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t1d[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture1D<>'}} expected-note {{no known conversion from 'int3' to 'uint1'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_sb[offset]; // expected-error {{no viable overloaded operator[] for type 'StructuredBuffer<float4>'}} expected-note {{no known conversion from 'int3' to 'uint1'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t1da[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture1DArray<>'}} expected-note {{no known conversion from 'int3' to 'uint2'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2d[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture2D<>'}} expected-note {{no known conversion from 'int3' to 'uint2'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2da[offset];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2dms[offset]; // expected-error {{no viable overloaded operator[] for type 'Texture2DMS<float4, 8>'}} expected-note {{no known conversion from 'int3' to 'uint2'}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2dmsa[offset];
  f4 += g_t3d[offset];
  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f4 += g_tc[offset]; // expected-error {{type 'TextureCube<>' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //f4 += g_tca[offset]; // expected-error {{type 'TextureCubeArray<>' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  return f4;
}

float4 test_mips_indexing()
{
  // .mips[uint mipSlice, uint pos]
  uint offset = 1;
  float4 f4;
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_b.mips[offset]; // expected-error {{no member named 'mips' in 'Buffer<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3022 : scalar, vector, or matrix expected
  //f4 += g_t1d.mips[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_sb.mips[offset]; // expected-error {{no member named 'mips' in 'StructuredBuffer<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3022 : scalar, vector, or matrix expected
  //f4 += g_t1da.mips[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3022 : scalar, vector, or matrix expected
  //f4 += g_t2d.mips[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3022 : scalar, vector, or matrix expected
  //f4 += g_t2da.mips[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_t2dms.mips[offset]; // expected-error {{no member named 'mips' in 'Texture2DMS<float4, 8>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_t2dmsa.mips[offset]; // expected-error {{no member named 'mips' in 'Texture2DMSArray<float4, 8>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3022 : scalar, vector, or matrix expected
  //f4 += g_t3d.mips[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_tc.mips[offset]; // expected-error {{no member named 'mips' in 'TextureCube<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_tca.mips[offset]; // expected-error {{no member named 'mips' in 'TextureCubeArray<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  return f4;
}

float4 test_mips_double_indexing()
{
  // .mips[uint mipSlice, uint pos]
  uint mipSlice = 1;
  uint pos = 1;
  uint2 pos2 = { 1, 2 };
  uint3 pos3 = { 1, 2, 3 };
  float4 f4;
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_b.mips[mipSlice][pos]; // expected-error {{no member named 'mips' in 'Buffer<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  f4 += g_t1d.mips[mipSlice][pos];
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_sb.mips[mipSlice][pos]; // expected-error {{no member named 'mips' in 'StructuredBuffer<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t1da.mips[mipSlice][pos]; // expected-error {{no viable overloaded operator[] for type 'Texture1DArray<float4>::mips_slice_type'}} expected-note {{candidate function [with element = float4] not viable: no known conversion from 'uint' to 'uint2' for 1st argument}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t1da.mips[mipSlice][pos2];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2d.mips[mipSlice][pos]; // expected-error {{no viable overloaded operator[] for type 'Texture2D<float4>::mips_slice_type'}} expected-note {{candidate function [with element = float4] not viable: no known conversion from 'uint' to 'uint2' for 1st argument}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2d.mips[mipSlice][pos2];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2da.mips[mipSlice][pos]; // expected-error {{no viable overloaded operator[] for type 'Texture2DArray<float4>::mips_slice_type'}} expected-note {{candidate function [with element = float4] not viable: no known conversion from 'uint' to 'uint3' for 1st argument}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2da.mips[mipSlice][pos3];
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_t2dms.mips[mipSlice][pos]; // expected-error {{no member named 'mips' in 'Texture2DMS<float4, 8>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_t2dmsa.mips[mipSlice][pos]; // expected-error {{no member named 'mips' in 'Texture2DMSArray<float4, 8>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t3d.mips[mipSlice][pos]; // expected-error {{no viable overloaded operator[] for type 'Texture3D<float4>::mips_slice_type'}} expected-note {{candidate function [with element = float4] not viable: no known conversion from 'uint' to 'uint3' for 1st argument}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t3d.mips[mipSlice][pos3];
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_tc.mips[mipSlice][pos]; // expected-error {{no member named 'mips' in 'TextureCube<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  // fxc error X3018: invalid subscript 'mips'
  //f4 += g_tca.mips[mipSlice][pos]; // expected-error {{no member named 'mips' in 'TextureCubeArray<float4>'}} fxc-error {{X3018: invalid subscript 'mips'}} 
  return f4;
}

float4 test_sample_indexing()
{
  // .sample[uint sampleSlice, uint pos]
  uint offset = 1;
  float4 f4;
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_b.sample[offset]; // expected-error {{no member named 'sample' in 'Buffer<float4>'}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_t1d.sample[offset]; // expected-error {{no member named 'sample' in 'Texture1D<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_sb.sample[offset]; // expected-error {{no member named 'sample' in 'StructuredBuffer<float4>'}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_t1da.sample[offset]; // expected-error {{no member named 'sample' in 'Texture1DArray<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_t2d.sample[offset]; // expected-error {{no member named 'sample' in 'Texture2D<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_t2da.sample[offset]; // expected-error {{no member named 'sample' in 'Texture2DArray<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3022: scalar, vector, or matrix expected
  //f4 += g_t2dms.sample[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3022: scalar, vector, or matrix expected
  //f4 += g_t2dmsa.sample[offset]; // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_t3d.sample[offset]; // expected-error {{no member named 'sample' in 'Texture3D<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_tc.sample[offset]; // expected-error {{no member named 'sample' in 'TextureCube<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  // fxc error X3018: invalid subscript 'sample'
  //f4 += g_tca.sample[offset]; // expected-error {{no member named 'sample' in 'TextureCubeArray<float4>'; did you mean 'Sample'?}} expected-error {{reference to non-static member function must be called}} fxc-error {{X3018: invalid subscript 'sample'}} 
  return f4;
}

float4 test_sample_double_indexing()
{
  // .sample[uint sampleSlice, uintn pos]
  uint sampleSlice = 1;
  uint pos = 1;
  uint2 pos2 = { 1, 2 };
  uint3 pos3 = { 1, 2, 3 };
  float4 f4;
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2dms.sample[sampleSlice][pos]; // expected-error {{no viable overloaded operator[] for type 'Texture2DMS<float4, 8>::sample_slice_type'}} expected-note {{candidate function [with element = float4] not viable: no known conversion from 'uint' to 'uint2' for 1st argument}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2dms.sample[sampleSlice][pos2];
  // fxc error X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions
  //f4 += g_t2dmsa.sample[sampleSlice][pos]; // expected-error {{no viable overloaded operator[] for type 'Texture2DMSArray<float4, 8>::sample_slice_type'}} expected-note {{candidate function [with element = float4] not viable: no known conversion from 'uint' to 'uint3' for 1st argument}} fxc-error {{X3120: invalid type for index - index must be a scalar, or a vector with the correct number of dimensions}} 
  f4 += g_t2dmsa.sample[sampleSlice][pos3];
  return f4;
}

Texture1D t1d;

void my_subscripts()
{
  int i;
  int2 i2;
  int ai2[2];
  int2x2 i22;

  // fxc error X3059: array dimension must be between 1 and 65536
  //int ai0[0]; // expected-error {{array dimension must be between 1 and 65536}} fxc-error {{X3059: array dimension must be between 1 and 65536}} 
  int ai1[1];
  int ai65536[65536];
  // fxc error X3059: array dimension must be between 1 and 65536
  //int ai65537[65537]; // expected-error {{array dimension must be between 1 and 65536}} fxc-error {{X3059: array dimension must be between 1 and 65536}} 

  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //i[0] = 1; // expected-error {{subscripted value is not an array, matrix, or vector}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
  i2[0] = 1;
  ai2[0] = 1;
  i22[0][0] = 2;
  // expected-warning@? {{conversion from larger type 'double' to smaller type 'unsigned int', possible loss of data}}
  //i22[1.5][0] = 2; // expected-warning {{conversion from larger type 'double' to smaller type 'unsigned int', possible loss of data}} expected-warning {{implicit conversion from 'double' to 'unsigned int' changes value from 1.5 to 1}} fxc-pass {{}} 
  i22[0] = i2;

  // Floats are fine.
  //i2[1.5f] = 1; // expected-warning {{implicit conversion from 'float' to 'unsigned int' changes value from 1.5 to 1}} fxc-pass {{}} 
  float fone = 1;
  i2[fone] = 1;
  // fxc error X3121: array, matrix, vector, or indexable object type expected in index expression
  //i = 1[ai2]; // expected-error {{HLSL does not support having the base of a subscript operator in brackets}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}} 
}

float4 plain(float4 param4 /* : FOO */) /*: FOO */{
  return 0; //  test_mips_double_indexing();
}