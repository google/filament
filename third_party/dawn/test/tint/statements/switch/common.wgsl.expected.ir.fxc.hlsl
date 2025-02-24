
[numthreads(1, 1, 1)]
void f() {
  int i = int(0);
  int result = int(0);
  switch(i) {
    case int(0):
    {
      result = int(10);
      break;
    }
    case int(1):
    {
      result = int(22);
      break;
    }
    case int(2):
    {
      result = int(33);
      break;
    }
    default:
    {
      result = int(44);
      break;
    }
  }
}

