
static bool4 u = (true).xxxx;
void f() {
  int4 v = int4(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

