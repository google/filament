
void a() {
  int a_1 = int(0);
  switch(a_1) {
    case int(0):
    case int(2):
    case int(4):
    {
      uint b = 3u;
      switch(b) {
        case 0u:
        {
          break;
        }
        case 1u:
        case 2u:
        case 3u:
        default:
        {
          uint c = 123u;
          switch(c) {
            case 0u:
            {
              break;
            }
            default:
            {
              return;
            }
          }
          return;
        }
      }
      break;
    }
    case int(1):
    default:
    {
      return;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

