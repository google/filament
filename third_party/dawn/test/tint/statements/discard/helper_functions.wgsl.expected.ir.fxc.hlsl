
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);
static bool continue_execution = true;
void foo() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    continue_execution = false;
  }
}

void bar() {
  float v = ddx(1.0f);
  if (continue_execution) {
    output.Store(0u, asuint(v));
  }
}

void main() {
  foo();
  bar();
  if (!(continue_execution)) {
    discard;
  }
}

