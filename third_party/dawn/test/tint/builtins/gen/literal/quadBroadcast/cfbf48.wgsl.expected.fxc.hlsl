SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 quadBroadcast_cfbf48() {
  float2 res = QuadReadLaneAt((1.0f).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_cfbf48()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_cfbf48()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'QuadReadLaneAt'


tint executable returned error: exit status 1
