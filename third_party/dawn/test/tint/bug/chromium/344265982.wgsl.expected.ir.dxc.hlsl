
RWByteAddressBuffer buffer : register(u0);
void foo() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(4))) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(asint(buffer.Load((0u + (min(uint(i), 3u) * 4u))))) {
        case int(1):
        {
          tint_continue = true;
          break;
        }
        default:
        {
          buffer.Store((0u + (min(uint(i), 3u) * 4u)), asuint(int(2)));
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

void main() {
  foo();
}

