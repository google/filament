[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  bool2 v2 = (true).xx;
  bool3 v3 = (true).xxx;
  bool4 v4 = (true).xxxx;
}
