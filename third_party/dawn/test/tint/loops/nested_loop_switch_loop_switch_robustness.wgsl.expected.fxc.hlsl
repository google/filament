[numthreads(1, 1, 1)]
void main() {
  bool tint_continue_1 = false;
  {
    for(int i = 0; (i < 2); i = (i + 2)) {
      tint_continue_1 = false;
      switch(i) {
        case 0: {
          bool tint_continue = false;
          {
            for(int j = 0; (j < 2); j = (j + 2)) {
              tint_continue = false;
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
                continue;
              }
            }
          }
          tint_continue_1 = true;
          break;
        }
        default: {
          break;
        }
      }
      if (tint_continue_1) {
        continue;
      }
    }
  }
  return;
}
