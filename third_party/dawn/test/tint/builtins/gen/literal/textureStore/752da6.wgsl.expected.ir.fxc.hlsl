//
// fragment_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_752da6() {
  arg_0[(int(1)).xx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_752da6();
}

//
// compute_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_752da6() {
  arg_0[(int(1)).xx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_752da6();
}

