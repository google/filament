static int I = 0;

void main_1() {
  I = 123;
  I = 123;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
