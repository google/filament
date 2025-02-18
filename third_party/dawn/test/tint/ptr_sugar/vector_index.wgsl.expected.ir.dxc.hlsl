
void deref_const() {
  int3 a = (int(0)).xxx;
  int b = a.x;
  a.x = int(42);
}

void no_deref_const() {
  int3 a = (int(0)).xxx;
  int b = a.x;
  a.x = int(42);
}

void deref_let() {
  int3 a = (int(0)).xxx;
  int i = int(0);
  int b = a[min(uint(i), 2u)];
  a.x = int(42);
}

void no_deref_let() {
  int3 a = (int(0)).xxx;
  int i = int(0);
  int b = a[min(uint(i), 2u)];
  a.x = int(42);
}

void deref_var() {
  int3 a = (int(0)).xxx;
  int i = int(0);
  int b = a[min(uint(i), 2u)];
  a.x = int(42);
}

void no_deref_var() {
  int3 a = (int(0)).xxx;
  int i = int(0);
  int b = a[min(uint(i), 2u)];
  a.x = int(42);
}

[numthreads(1, 1, 1)]
void main() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
}

