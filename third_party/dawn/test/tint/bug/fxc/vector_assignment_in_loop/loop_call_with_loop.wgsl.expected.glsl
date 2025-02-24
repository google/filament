#version 310 es

vec2 v2f = vec2(0.0f);
ivec3 v3i = ivec3(0);
uvec4 v4u = uvec4(0u);
bvec2 v2b = bvec2(false);
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      v2f[min(uint(i), 1u)] = 1.0f;
      v3i[min(uint(i), 2u)] = 1;
      v4u[min(uint(i), 3u)] = 1u;
      v2b[min(uint(i), 1u)] = true;
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      foo();
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
