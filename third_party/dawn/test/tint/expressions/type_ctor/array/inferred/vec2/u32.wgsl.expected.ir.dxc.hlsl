
static const uint2 v_1[2] = {(1u).xx, (2u).xx};
static uint2 arr[2] = v_1;
void f() {
  uint2 v[2] = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

