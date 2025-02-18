
static bool t = false;
bool4 m() {
  t = true;
  return bool4((t).xxxx);
}

void f() {
  float4 v = float4(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

