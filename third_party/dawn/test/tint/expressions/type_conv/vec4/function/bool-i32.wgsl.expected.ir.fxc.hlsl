
static bool t = false;
bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

void f() {
  int4 v = int4(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

