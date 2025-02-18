[numthreads(1, 1, 1)]
void main() {
  int j = 0;
  bool tint_continue = false;
  {
    for(int i = 0; (i < 2); i = (i + 2)) {
      tint_continue = false;
      switch(i) {
        case 0: {
          switch(j) {
            case 0: {
              tint_continue = true;
              break;
            }
            default: {
              break;
            }
          }
          if (tint_continue) {
            break;
          }
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
