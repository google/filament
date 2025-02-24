//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void atomicAdd_8a199a() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_13 = sb_rwatomicAdd(0u, 1u);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicAdd_8a199a();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
//
// compute_main
//
RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void atomicAdd_8a199a() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_13 = sb_rwatomicAdd(0u, 1u);
  res = x_13;
  return;
}

void compute_main_1() {
  atomicAdd_8a199a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
