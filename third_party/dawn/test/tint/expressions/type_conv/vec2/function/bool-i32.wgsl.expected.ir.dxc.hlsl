
static bool t = false;
bool2 m() {
  t = true;
  return bool2((t).xx);
}

void f() {
  int2 v = int2(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

