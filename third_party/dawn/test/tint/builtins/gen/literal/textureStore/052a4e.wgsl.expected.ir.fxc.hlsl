//
// fragment_main
//

RWTexture1D<float4> arg_0 : register(u0, space1);
void textureStore_052a4e() {
  arg_0[1u] = (1.0f).xxxx;
}

void fragment_main() {
  textureStore_052a4e();
}

//
// compute_main
//

RWTexture1D<float4> arg_0 : register(u0, space1);
void textureStore_052a4e() {
  arg_0[1u] = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_052a4e();
}

