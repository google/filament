//
// fragment_main
//

RWTexture1D<uint4> arg_0 : register(u0, space1);
void textureStore_9d7c62() {
  arg_0[int(1)] = (1u).xxxx;
}

void fragment_main() {
  textureStore_9d7c62();
}

//
// compute_main
//

RWTexture1D<uint4> arg_0 : register(u0, space1);
void textureStore_9d7c62() {
  arg_0[int(1)] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9d7c62();
}

