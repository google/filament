
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);
void foo() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    discard;
  }
}

void bar() {
  output.Store(0u, asuint(ddx(1.0f)));
}

void main() {
  foo();
  bar();
}

