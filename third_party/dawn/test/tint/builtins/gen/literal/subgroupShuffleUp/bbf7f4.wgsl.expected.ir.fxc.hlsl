SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupShuffleUp_bbf7f4() {
  float16_t res = WaveReadLaneAt(float16_t(1.0h), (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupShuffleUp_bbf7f4());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupShuffleUp_bbf7f4());
}

FXC validation failure:
<scrubbed_path>(3,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
