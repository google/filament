//
// fragment_main
//
RWTexture1D<uint4> arg_0 : register(u0, space1);

void textureStore_3bec15() {
  arg_0[1] = (1u).xxxx;
}

void fragment_main() {
  textureStore_3bec15();
  return;
}
//
// compute_main
//
RWTexture1D<uint4> arg_0 : register(u0, space1);

void textureStore_3bec15() {
  arg_0[1] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_3bec15();
  return;
}
