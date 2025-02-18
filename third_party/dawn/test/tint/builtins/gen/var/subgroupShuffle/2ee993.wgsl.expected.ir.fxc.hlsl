SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupShuffle_2ee993() {
  int4 arg_0 = (int(1)).xxxx;
  int arg_1 = int(1);
  int4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_2ee993()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_2ee993()));
}

FXC validation failure:
<scrubbed_path>(6,14-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
