//
// fragment_main
//
RWTexture2D<uint4> arg_0 : register(u0, space1);

void textureStore_5425ab() {
  int2 arg_1 = (1).xx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_5425ab();
  return;
}
//
// compute_main
//
RWTexture2D<uint4> arg_0 : register(u0, space1);

void textureStore_5425ab() {
  int2 arg_1 = (1).xx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_5425ab();
  return;
}
