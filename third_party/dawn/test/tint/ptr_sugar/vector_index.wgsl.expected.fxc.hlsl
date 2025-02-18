void deref_const() {
  int3 a = int3(0, 0, 0);
  int b = a[0];
  a[0] = 42;
}

void no_deref_const() {
  int3 a = int3(0, 0, 0);
  int b = a[0];
  a[0] = 42;
}

void deref_let() {
  int3 a = int3(0, 0, 0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a[0] = 42;
}

void no_deref_let() {
  int3 a = int3(0, 0, 0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a[0] = 42;
}

void deref_var() {
  int3 a = int3(0, 0, 0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a[0] = 42;
}

void no_deref_var() {
  int3 a = int3(0, 0, 0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a[0] = 42;
}

[numthreads(1, 1, 1)]
void main() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
  return;
}
