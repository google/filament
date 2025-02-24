struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer io : register(u0);
float3 Bad(uint index, float3 rd) {
  float3 normal = (0.0f).xxx;
  float v_1 = -(float(sign(rd[min(index, 2u)])));
  float3 v_2 = normal;
  float3 v_3 = float3((v_1).xxx);
  float3 v_4 = float3((index).xxx);
  normal = (((v_4 == float3(int(0), int(1), int(2)))) ? (v_3) : (v_2));
  return normalize(normal);
}

void main_inner(uint idx) {
  io.Store3(0u, asuint(Bad(io.Load(12u), asfloat(io.Load3(0u)))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

