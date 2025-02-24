#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(i) {
        case 0:
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
          i = (i + 1);
        }
        continue;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
