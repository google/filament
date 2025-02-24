SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupAdd_3854ae() {
  float arg_0 = 1.0f;
  float res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
}

FXC validation failure:
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
