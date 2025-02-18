#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 0;
  {
    int i_1 = 0;
    while(true) {
      if ((i_1 < 2)) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(i_1) {
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
          i_1 = (i_1 + 1);
        }
        continue;
      }
      bool tint_continue_1 = false;
      switch(i_1) {
        case 0:
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
          i_1 = (i_1 + 1);
        }
        continue;
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
}
