
void deref() {
  int3 a = (int(0)).xxx;
  a.x = (a.x + int(42));
}

void no_deref() {
  int3 a = (int(0)).xxx;
  a.x = (a.x + int(42));
}

void deref_inc() {
  int3 a = (int(0)).xxx;
  a.x = (a.x + int(1));
}

void no_deref_inc() {
  int3 a = (int(0)).xxx;
  a.x = (a.x + int(1));
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
}

