//
// fragment_main
//

RWTexture1D<uint4> arg_0 : register(u0, space1);
void textureStore_d26166() {
  uint arg_1 = 1u;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_d26166();
}

//
// compute_main
//

RWTexture1D<uint4> arg_0 : register(u0, space1);
void textureStore_d26166() {
  uint arg_1 = 1u;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_d26166();
}

