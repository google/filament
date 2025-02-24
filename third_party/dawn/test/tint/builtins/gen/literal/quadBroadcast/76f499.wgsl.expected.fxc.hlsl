SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 quadBroadcast_76f499() {
  int4 res = QuadReadLaneAt((1).xxxx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_76f499()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_76f499()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-40): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
