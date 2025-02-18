
void X(float2 a, float2 b) {
}

float2 Y() {
  return (0.0f).xx;
}

void f() {
  float2 v = (0.0f).xx;
  X((0.0f).xx, v);
  X((0.0f).xx, Y());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

