[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int2 v2 = (3).xx;
  int3 v3 = (3).xxx;
  int4 v4 = (3).xxxx;
}
