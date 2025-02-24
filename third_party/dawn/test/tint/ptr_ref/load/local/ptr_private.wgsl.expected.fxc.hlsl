static int i = 123;

[numthreads(1, 1, 1)]
void main() {
  int u = (i + 1);
  return;
}
