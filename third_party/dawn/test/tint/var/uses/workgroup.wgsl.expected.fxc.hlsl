//
// main1
//
groupshared int a;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    a = 0;
  }
  GroupMemoryBarrierWithGroupSync();
}

void uses_a() {
  a = (a + 1);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main1_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  a = 42;
  uses_a();
}

[numthreads(1, 1, 1)]
void main1(tint_symbol_1 tint_symbol) {
  main1_inner(tint_symbol.local_invocation_index);
  return;
}
//
// main2
//
groupshared int b;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    b = 0;
  }
  GroupMemoryBarrierWithGroupSync();
}

void uses_b() {
  b = (b * 2);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main2_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  b = 7;
  uses_b();
}

[numthreads(1, 1, 1)]
void main2(tint_symbol_1 tint_symbol) {
  main2_inner(tint_symbol.local_invocation_index);
  return;
}
//
// main3
//
groupshared int a;
groupshared int b;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    a = 0;
    b = 0;
  }
  GroupMemoryBarrierWithGroupSync();
}

void uses_a() {
  a = (a + 1);
}

void uses_b() {
  b = (b * 2);
}

void uses_a_and_b() {
  b = a;
}

void no_uses() {
}

void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main3_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  outer();
  no_uses();
}

[numthreads(1, 1, 1)]
void main3(tint_symbol_1 tint_symbol) {
  main3_inner(tint_symbol.local_invocation_index);
  return;
}
//
// main4
//
void no_uses() {
}

[numthreads(1, 1, 1)]
void main4() {
  no_uses();
  return;
}
