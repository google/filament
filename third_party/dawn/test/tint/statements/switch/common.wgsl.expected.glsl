#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 0;
  int result = 0;
  switch(i) {
    case 0:
    {
      result = 10;
      break;
    }
    case 1:
    {
      result = 22;
      break;
    }
    case 2:
    {
      result = 33;
      break;
    }
    default:
    {
      result = 44;
      break;
    }
  }
}
