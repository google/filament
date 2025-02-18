[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared float a[10];
groupshared float b[20];

void f() {
  float x = a[0];
  float y = b[0];
}
