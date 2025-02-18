//
// fragment_main
//
RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_a0022f() {
  arg_0[int3((1).xx, 1)] = (1).xxxx;
}

void fragment_main() {
  textureStore_a0022f();
  return;
}
//
// compute_main
//
RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_a0022f() {
  arg_0[int3((1).xx, 1)] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_a0022f();
  return;
}
