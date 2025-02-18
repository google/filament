
[numthreads(1, 1, 1)]
void main() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(i) {
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
          i = (i + int(1));
        }
        continue;
      }
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

