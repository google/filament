[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  bool v = true;
  bool2 v2 = bool2((v).xx);
  bool3 v3 = bool3((v).xxx);
  bool4 v4 = bool4((v).xxxx);
}
