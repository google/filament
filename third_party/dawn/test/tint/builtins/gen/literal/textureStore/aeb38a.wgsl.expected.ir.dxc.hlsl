//
// fragment_main
//

RWTexture2D<uint4> arg_0 : register(u0, space1);
void textureStore_aeb38a() {
  arg_0[(1u).xx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_aeb38a();
}

//
// compute_main
//

RWTexture2D<uint4> arg_0 : register(u0, space1);
void textureStore_aeb38a() {
  arg_0[(1u).xx] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_aeb38a();
}

