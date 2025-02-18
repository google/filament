SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffle_4752bd() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_4752bd()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_4752bd()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,15-42): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
