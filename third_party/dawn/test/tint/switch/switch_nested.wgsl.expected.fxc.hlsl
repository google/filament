[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void a() {
  int a_1 = 0;
  switch(a_1) {
    case 0:
    case 2:
    case 4: {
      uint b = 3u;
      switch(b) {
        case 0u: {
          break;
        }
        case 1u:
        case 2u:
        case 3u:
        default: {
          uint c = 123u;
          switch(c) {
            case 0u: {
              break;
            }
            default: {
              return;
              break;
            }
          }
          return;
          break;
        }
      }
      break;
    }
    case 1:
    default: {
      return;
      break;
    }
  }
}
