
RWByteAddressBuffer v : register(u0);
void bar(inout float p) {
  float a = 1.0f;
  float b = 2.0f;
  bool v_1 = false;
  if ((a >= 0.0f)) {
    v_1 = (b >= 0.0f);
  } else {
    v_1 = false;
  }
  bool cond = v_1;
  p = ((cond) ? (b) : (a));
}

[numthreads(1, 1, 1)]
void foo() {
  float param = 0.0f;
  bar(param);
  v.Store(0u, asuint(param));
}

