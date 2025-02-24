SKIP: INVALID

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared float16_t arg_0;
float16_t workgroupUniformLoad_e07d08() {
  GroupMemoryBarrierWithGroupSync();
  float16_t v = arg_0;
  GroupMemoryBarrierWithGroupSync();
  float16_t res = v;
  return res;
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    arg_0 = float16_t(0.0h);
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store<float16_t>(0u, workgroupUniformLoad_e07d08());
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

FXC validation failure:
<scrubbed_path>(7,13-21): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
