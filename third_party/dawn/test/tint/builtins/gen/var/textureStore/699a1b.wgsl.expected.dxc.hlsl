//
// fragment_main
//
RWTexture2DArray<float4> arg_0 : register(u0, space1);

void textureStore_699a1b() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  float4 arg_3 = (1.0f).xxxx;
  arg_0[uint3(arg_1, arg_2)] = arg_3;
}

void fragment_main() {
  textureStore_699a1b();
  return;
}
//
// compute_main
//
RWTexture2DArray<float4> arg_0 : register(u0, space1);

void textureStore_699a1b() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  float4 arg_3 = (1.0f).xxxx;
  arg_0[uint3(arg_1, arg_2)] = arg_3;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_699a1b();
  return;
}
