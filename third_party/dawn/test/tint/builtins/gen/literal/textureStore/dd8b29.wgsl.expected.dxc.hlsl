//
// fragment_main
//
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_dd8b29() {
  arg_0[int3((1).xx, 1)] = (1u).xxxx;
}

void fragment_main() {
  textureStore_dd8b29();
  return;
}
//
// compute_main
//
RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_dd8b29() {
  arg_0[int3((1).xx, 1)] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_dd8b29();
  return;
}
