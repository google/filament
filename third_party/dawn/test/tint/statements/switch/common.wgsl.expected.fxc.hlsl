[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  int result = 0;
  switch(i) {
    case 0: {
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
    default: {
      result = 44;
      break;
    }
  }
  return;
}
