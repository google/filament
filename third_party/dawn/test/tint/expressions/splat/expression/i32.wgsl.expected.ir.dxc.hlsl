
void f() {
  int2 v2 = (int(3)).xx;
  int3 v3 = (int(3)).xxx;
  int4 v4 = (int(3)).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

