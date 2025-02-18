//
// fragment_main
//

RWTexture2D<float4> arg_0 : register(u0, space1);
void textureStore_2c76db() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_2c76db();
}

//
// compute_main
//

RWTexture2D<float4> arg_0 : register(u0, space1);
void textureStore_2c76db() {
  arg_0[(1u).xx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_2c76db();
}

