void set_vector_element(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

float3 Bad(uint index, float3 rd) {
  float3 normal = (0.0f).xxx;
  set_vector_element(normal, min(index, 2u), -(float(sign(rd[min(index, 2u)]))));
  return normalize(normal);
}

RWByteAddressBuffer io : register(u0);

struct tint_symbol_1 {
  uint idx : SV_GroupIndex;
};

void main_inner(uint idx) {
  io.Store3(0u, asuint(Bad(io.Load(12u), asfloat(io.Load3(0u)))));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.idx);
  return;
}
