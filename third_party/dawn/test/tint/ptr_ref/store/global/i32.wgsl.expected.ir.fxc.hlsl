
static int I = int(0);
[numthreads(1, 1, 1)]
void main() {
  I = int(123);
  I = int(123);
}

