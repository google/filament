[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

void foo(float x) {
  tint_ftoi(x);
  do {
  } while (false);
}

static int global = 0;

int baz(int x) {
  global = 42;
  return x;
}

void bar(float x) {
  baz(tint_ftoi(x));
  do {
  } while (false);
}

void main() {
}
