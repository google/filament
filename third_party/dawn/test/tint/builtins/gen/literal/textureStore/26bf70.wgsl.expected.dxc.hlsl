//
// fragment_main
//
RWTexture2D<uint4> arg_0 : register(u0, space1);

void textureStore_26bf70() {
  arg_0[(1).xx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_26bf70();
  return;
}
//
// compute_main
//
RWTexture2D<uint4> arg_0 : register(u0, space1);

void textureStore_26bf70() {
  arg_0[(1).xx] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_26bf70();
  return;
}
