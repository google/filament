[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int2 v2 = (1).xx;
  int3 v3 = (1).xxx;
  int4 v4 = (1).xxxx;
}
