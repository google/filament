[numthreads(1, 1, 1)]
void main() {
  int i = 0;
  bool tint_continue = false;
  {
    for(int i_1 = 0; (i_1 < 2); i_1 = (i_1 + 1)) {
      tint_continue = false;
      switch(i_1) {
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
      switch(i_1) {
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
