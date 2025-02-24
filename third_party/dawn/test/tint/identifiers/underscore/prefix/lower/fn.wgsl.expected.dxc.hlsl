void a() {
}

void _a() {
}

void b() {
  a();
}

void _b() {
  _a();
}

[numthreads(1, 1, 1)]
void main() {
  b();
  _b();
  return;
}
