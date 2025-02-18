static bool tint_discarded = false;
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);

void foo() {
  if ((asint(non_uniform_global.Load(0u)) < 0)) {
    tint_discarded = true;
  }
}

void bar() {
  float tint_symbol = ddx(1.0f);
  if (!(tint_discarded)) {
    output.Store(0u, asuint(tint_symbol));
  }
}

void main() {
  foo();
  bar();
  if (tint_discarded) {
    discard;
  }
  return;
}
