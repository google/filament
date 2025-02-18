
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
static float3 v1 = (0.0f).xxx;
[numthreads(1, 1, 1)]
void main() {
  uint v = i[0u].x;
  float3 v_1 = v1;
  float3 v_2 = float3((1.0f).xxx);
  float3 v_3 = float3((v).xxx);
  v1 = (((v_3 == float3(int(0), int(1), int(2)))) ? (v_2) : (v_1));
}

