#version 310 es

void a() {
  int a_1 = 0;
  switch(a_1) {
    case 0:
    {
      break;
    }
    case 1:
    {
      return;
    }
    default:
    {
      a_1 = (a_1 + 2);
      break;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
