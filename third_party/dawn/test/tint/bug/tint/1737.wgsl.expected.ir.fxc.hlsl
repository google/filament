
groupshared float a[10];
groupshared float b[20];
void f() {
  float x = a[0u];
  float y = b[0u];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

