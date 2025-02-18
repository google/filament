[numthreads(1, 1, 1)]
void f() {
  bool tint_continue = false;
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
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
        continue;
      }
    }
  }
  return;
}
