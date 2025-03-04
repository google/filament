// Rewrite unchanged result:
Buffer g_b;
StructuredBuffer<float4> g_sb;
Texture1D g_t1d;
Texture1DArray g_t1da;
Texture2D g_t2d;
Texture2DArray g_t2da;
Texture2D g_t2a2[17];
Texture2DMS<float4, 8> g_t2dms;
Texture2DMSArray<float4, 8> g_t2dmsa;
Texture3D g_t3d;
TextureCube g_tc;
TextureCubeArray g_tca;
RWStructuredBuffer<float4> g_rw_sb;
RWBuffer<float4> g_rw_b;
RWTexture1D<float4> g_rw_t1d;
RWTexture1DArray<float4> g_rw_t1da;
RWTexture2D<float4> g_rw_t2d;
RWTexture2DArray<float4> g_rw_t2da;
RWTexture3D<float4> g_rw_t3d;
float test_vector_indexing() {
  float4 f4 = { 1, 2, 3, 4 };
  float f = f4[0];
  return f;
}


float4 test_scalar_indexing() {
  float4 f4 = 0;
  f4 += g_b[0];
  f4 += g_t1d[0];
  f4 += g_sb[0];
  return f4;
}


float4 test_vector2_indexing() {
  int2 offset = { 1, 2 };
  float4 f4 = 0;
  f4 += g_t1da[offset];
  f4 += g_t2d[offset];
  f4 += g_t2dms[offset];
  return f4;
}


float4 test_vector3_indexing() {
  int3 offset = { 1, 2, 3 };
  float4 f4 = 0;
  f4 += g_t2da[offset];
  f4 += g_t2dmsa[offset];
  f4 += g_t3d[offset];
  return f4;
}


float4 test_mips_indexing() {
  uint offset = 1;
  float4 f4;
  return f4;
}


float4 test_mips_double_indexing() {
  uint mipSlice = 1;
  uint pos = 1;
  uint2 pos2 = { 1, 2 };
  uint3 pos3 = { 1, 2, 3 };
  float4 f4;
  f4 += g_t1d.mips[mipSlice][pos];
  f4 += g_t1da.mips[mipSlice][pos2];
  f4 += g_t2d.mips[mipSlice][pos2];
  f4 += g_t2da.mips[mipSlice][pos3];
  f4 += g_t3d.mips[mipSlice][pos3];
  return f4;
}


float4 test_sample_indexing() {
  uint offset = 1;
  float4 f4;
  return f4;
}


float4 test_sample_double_indexing() {
  uint sampleSlice = 1;
  uint pos = 1;
  uint2 pos2 = { 1, 2 };
  uint3 pos3 = { 1, 2, 3 };
  float4 f4;
  f4 += g_t2dms.sample[sampleSlice][pos2];
  f4 += g_t2dmsa.sample[sampleSlice][pos3];
  return f4;
}


Texture1D t1d;
void my_subscripts() {
  int i;
  int2 i2;
  int ai2[2];
  int2x2 i22;
  int ai1[1];
  int ai65536[65536];
  i2[0] = 1;
  ai2[0] = 1;
  i22[0][0] = 2;
  i22[0] = i2;
  float fone = 1;
  i2[fone] = 1;
}


float4 plain(float4 param4) {
  return 0;
}


