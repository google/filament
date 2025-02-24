//
// fragment_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_f64d69() {
  arg_0[int(1)] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_f64d69();
}

//
// compute_main
//

RWTexture1D<int4> arg_0 : register(u0, space1);
void textureStore_f64d69() {
  arg_0[int(1)] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_f64d69();
}

