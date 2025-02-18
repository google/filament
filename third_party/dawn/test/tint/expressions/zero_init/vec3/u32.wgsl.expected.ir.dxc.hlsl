
void f() {
  uint3 v = (0u).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

