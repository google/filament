
[numthreads(1, 1, 1)]
void f() {
  int i = int(0);
  int result = int(0);
  {
    while(true) {
      result = int(44);
      break;
    }
  }
}

