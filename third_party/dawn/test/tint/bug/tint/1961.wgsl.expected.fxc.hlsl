[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  bool x = false;
  bool y = false;
  bool tint_tmp = x;
  if (tint_tmp) {
    tint_tmp = true;
  }
  if ((tint_tmp)) {
  }
}
