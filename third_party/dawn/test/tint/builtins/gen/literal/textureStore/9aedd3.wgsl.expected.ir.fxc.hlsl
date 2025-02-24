//
// fragment_main
//

RWTexture3D<float4> arg_0 : register(u0, space1);
void textureStore_9aedd3() {
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_9aedd3();
}

//
// compute_main
//

RWTexture3D<float4> arg_0 : register(u0, space1);
void textureStore_9aedd3() {
  arg_0[(1u).xxx] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9aedd3();
}

