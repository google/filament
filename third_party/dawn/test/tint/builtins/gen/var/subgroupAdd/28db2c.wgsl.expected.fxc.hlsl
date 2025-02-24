SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupAdd_28db2c() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
