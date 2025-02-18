//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicMax(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_51b9be() {
  uint res = 0u;
  uint x_9 = sb_rwatomicMax(0u, 1u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicMax_51b9be();
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

uint sb_rwatomicMax(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_51b9be() {
  uint res = 0u;
  uint x_9 = sb_rwatomicMax(0u, 1u);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicMax_51b9be();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
