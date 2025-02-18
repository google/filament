static int I = 0;

[numthreads(1, 1, 1)]
void main() {
  I = 123;
  I = 123;
  return;
}
