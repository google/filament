//
// main1
//
struct main1_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int a;
void uses_a() {
  a = (a + int(1));
}

void main1_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    a = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  a = int(42);
  uses_a();
}

[numthreads(1, 1, 1)]
void main1(main1_inputs inputs) {
  main1_inner(inputs.tint_local_index);
}

//
// main2
//
struct main2_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int b;
void uses_b() {
  b = (b * int(2));
}

void main2_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    b = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  b = int(7);
  uses_b();
}

[numthreads(1, 1, 1)]
void main2(main2_inputs inputs) {
  main2_inner(inputs.tint_local_index);
}

//
// main3
//
struct main3_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int a;
groupshared int b;
void uses_a() {
  a = (a + int(1));
}

void uses_b() {
  b = (b * int(2));
}

void uses_a_and_b() {
  b = a;
}

void no_uses() {
}

void outer() {
  a = int(0);
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}

void main3_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    a = int(0);
    b = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  outer();
  no_uses();
}

[numthreads(1, 1, 1)]
void main3(main3_inputs inputs) {
  main3_inner(inputs.tint_local_index);
}

//
// main4
//

void no_uses() {
}

[numthreads(1, 1, 1)]
void main4() {
  no_uses();
}

