//
// fragment_main
//
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_2383fc() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = 1;
  uint4 arg_3 = (1u).xxxx;
  arg_0[uint3(arg_1, uint(arg_2))] = arg_3;
}

void fragment_main() {
  textureStore_2383fc();
  return;
}
//
// compute_main
//
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_2383fc() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = 1;
  uint4 arg_3 = (1u).xxxx;
  arg_0[uint3(arg_1, uint(arg_2))] = arg_3;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2383fc();
  return;
}
