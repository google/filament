RWByteAddressBuffer buffer : register(u0);

void foo_buffer() {
  bool tint_continue = false;
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
      tint_continue = false;
      switch(asint(buffer.Load((4u * min(uint(i), 3u))))) {
        case 1: {
          tint_continue = true;
          break;
        }
        default: {
          buffer.Store((4u * min(uint(i), 3u)), asuint(2));
          break;
        }
      }
      if (tint_continue) {
        continue;
      }
    }
  }
}

void main() {
  foo_buffer();
  return;
}
