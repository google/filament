SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupBroadcastFirst_705aad() {
  vector<float16_t, 4> res = WaveReadLaneFirst((float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupBroadcastFirst_705aad());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupBroadcastFirst_705aad());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
