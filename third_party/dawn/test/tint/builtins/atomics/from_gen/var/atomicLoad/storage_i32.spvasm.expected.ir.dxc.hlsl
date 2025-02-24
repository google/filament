//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicLoad_0806ad() {
  int res = int(0);
  int v = int(0);
  sb_rw.InterlockedOr(int(0u), int(0), v);
  int x_9 = v;
  res = x_9;
}

void fragment_main_1() {
  atomicLoad_0806ad();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicLoad_0806ad() {
  int res = int(0);
  int v = int(0);
  sb_rw.InterlockedOr(int(0u), int(0), v);
  int x_9 = v;
  res = x_9;
}

void compute_main_1() {
  atomicLoad_0806ad();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

