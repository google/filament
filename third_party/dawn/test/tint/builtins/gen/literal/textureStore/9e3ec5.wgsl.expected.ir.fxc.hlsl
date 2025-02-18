//
// fragment_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_9e3ec5() {
  arg_0[(int(1)).xx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_9e3ec5();
}

//
// compute_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_9e3ec5() {
  arg_0[(int(1)).xx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9e3ec5();
}

