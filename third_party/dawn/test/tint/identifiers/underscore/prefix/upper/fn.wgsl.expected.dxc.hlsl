void A() {
}

void _A() {
}

void B() {
  A();
}

void _B() {
  _A();
}

[numthreads(1, 1, 1)]
void main() {
  B();
  _B();
  return;
}
