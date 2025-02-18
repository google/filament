//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicLoad(uint offset) {
  int value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


void atomicLoad_0806ad() {
  int res = 0;
  int x_9 = sb_rwatomicLoad(0u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicLoad_0806ad();
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

int sb_rwatomicLoad(uint offset) {
  int value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


void atomicLoad_0806ad() {
  int res = 0;
  int x_9 = sb_rwatomicLoad(0u);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicLoad_0806ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
