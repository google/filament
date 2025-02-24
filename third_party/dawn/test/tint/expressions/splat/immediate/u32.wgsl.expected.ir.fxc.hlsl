
void f() {
  uint2 v2 = (1u).xx;
  uint3 v3 = (1u).xxx;
  uint4 v4 = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

