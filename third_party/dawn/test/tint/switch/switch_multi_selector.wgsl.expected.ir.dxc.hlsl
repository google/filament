
void a() {
  int a_1 = int(0);
  switch(a_1) {
    case int(0):
    case int(2):
    case int(4):
    {
      break;
    }
    case int(1):
    default:
    {
      return;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

