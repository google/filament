//
// fragment_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_de4b94() {
  arg_0[1u] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_de4b94();
}

//
// compute_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_de4b94() {
  arg_0[1u] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_de4b94();
}

