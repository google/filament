//
// fragment_main
//
RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_72fa64() {
  arg_0[uint3((1u).xx, uint(1))] = (1).xxxx;
}

void fragment_main() {
  textureStore_72fa64();
  return;
}
//
// compute_main
//
RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_72fa64() {
  arg_0[uint3((1u).xx, uint(1))] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_72fa64();
  return;
}
