
[numthreads(1, 1, 1)]
void main() {
  int i = int(0);
  {
    int i_1 = int(0);
    while(true) {
      if ((i_1 < int(2))) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(i_1) {
        case int(0):
        {
          tint_continue = true;
          break;
        }
        default:
        {
          break;
        }
      }
      if (tint_continue) {
        {
          i_1 = (i_1 + int(1));
        }
        continue;
      }
      bool tint_continue_1 = false;
      switch(i_1) {
        case int(0):
        {
          tint_continue_1 = true;
          break;
        }
        default:
        {
          break;
        }
      }
      if (tint_continue_1) {
        {
          i_1 = (i_1 + int(1));
        }
        continue;
      }
      {
        i_1 = (i_1 + int(1));
      }
      continue;
    }
  }
}

