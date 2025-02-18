[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol = 0u;
  bool tint_continue = false;
  while (true) {
    tint_continue = false;
    switch(2) {
      case 1: {
        tint_continue = true;
        break;
      }
      default: {
        break;
      }
    }
    if (tint_continue) {
      {
        if (true) { break; }
      }
      continue;
    }
    tint_symbol = (tint_symbol + 1u);
    {
      if (true) { break; }
    }
  }
  return;
}
