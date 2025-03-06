// Rewrite unchanged result:
cbuffer cb0 {
  const float X;
}
;
tbuffer tb0 {
  const float Y;
}
;
Buffer<int> g_intBuffer;
RWByteAddressBuffer g_byteBuffer;
Texture1D<double> g_tex1d;
Texture1DArray<int> g_tex1dArray;
Texture2D<float> g_tex2d;
Texture2DArray<float> g_tex2dArray;
Texture2DMS<half> g_texture2dms;
Texture2DMSArray<float> g_texture2dmsArray;
float main() : SV_Target {
  return -X + Y;
}


