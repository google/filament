SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 quadBroadcast_0e0e6e() {
  int3 res = QuadReadLaneAt((1).xxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_0e0e6e()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_0e0e6e()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-40): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
