[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  bool tint_continue = false;
  while (true) {
    tint_continue = false;
    switch(i) {
      case 0: {
        tint_continue = true;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      {
        i = (i + 1);
        if ((i >= 4)) { break; }
      }
      continue;
    }
    {
      i = (i + 1);
      if ((i >= 4)) { break; }
    }
  }
  return;
}
