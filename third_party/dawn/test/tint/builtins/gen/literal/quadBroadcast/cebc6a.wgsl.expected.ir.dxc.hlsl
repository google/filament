//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t quadBroadcast_cebc6a() {
  float16_t res = QuadReadLaneAt(float16_t(1.0h), 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, quadBroadcast_cebc6a());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t quadBroadcast_cebc6a() {
  float16_t res = QuadReadLaneAt(float16_t(1.0h), 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, quadBroadcast_cebc6a());
}

