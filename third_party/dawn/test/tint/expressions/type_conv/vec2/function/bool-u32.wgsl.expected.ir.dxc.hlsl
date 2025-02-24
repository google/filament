
static bool t = false;
bool2 m() {
  t = true;
  return bool2((t).xx);
}

void f() {
  uint2 v = uint2(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

