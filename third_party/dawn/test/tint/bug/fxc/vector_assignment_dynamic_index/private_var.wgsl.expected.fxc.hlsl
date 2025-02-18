void set_vector_element(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
static float3 v1 = float3(0.0f, 0.0f, 0.0f);

[numthreads(1, 1, 1)]
void main() {
  set_vector_element(v1, min(i[0].x, 2u), 1.0f);
  return;
}
