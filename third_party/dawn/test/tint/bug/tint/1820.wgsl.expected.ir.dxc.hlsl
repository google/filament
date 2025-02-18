
static int global = int(0);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void foo(float x) {
  switch(tint_f32_to_i32(x)) {
    default:
    {
      break;
    }
  }
}

int baz(int x) {
  global = int(42);
  return x;
}

void bar(float x) {
  switch(baz(tint_f32_to_i32(x))) {
    default:
    {
      break;
    }
  }
}

void main() {
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

