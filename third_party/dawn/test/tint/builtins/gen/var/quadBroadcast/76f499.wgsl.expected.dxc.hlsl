//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 quadBroadcast_76f499() {
  int4 arg_0 = (1).xxxx;
  int4 res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_76f499()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int4 quadBroadcast_76f499() {
  int4 arg_0 = (1).xxxx;
  int4 res = QuadReadLaneAt(arg_0, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_76f499()));
  return;
}
