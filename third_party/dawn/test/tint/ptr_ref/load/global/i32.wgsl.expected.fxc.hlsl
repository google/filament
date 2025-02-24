static int I = 0;

[numthreads(1, 1, 1)]
void main() {
  int i = I;
  int u = (i + 1);
  return;
}
