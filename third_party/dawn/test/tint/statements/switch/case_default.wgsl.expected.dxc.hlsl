[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  int result = 0;
  switch(i) {
    default: {
      result = 10;
      break;
    }
    case 1: {
      result = 22;
      break;
    }
    case 2: {
      result = 33;
      break;
    }
  }
  return;
}
