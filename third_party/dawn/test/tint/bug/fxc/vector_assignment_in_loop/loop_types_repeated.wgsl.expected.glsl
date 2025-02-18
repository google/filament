#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 v2f = vec2(0.0f);
  vec2 v2f_2 = vec2(0.0f);
  ivec3 v3i = ivec3(0);
  ivec3 v3i_2 = ivec3(0);
  uvec4 v4u = uvec4(0u);
  uvec4 v4u_2 = uvec4(0u);
  bvec2 v2b = bvec2(false);
  bvec2 v2b_2 = bvec2(false);
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
      v2f_2[min(uint(i), 1u)] = 1.0f;
      v3i_2[min(uint(i), 2u)] = 1;
      v4u_2[min(uint(i), 3u)] = 1u;
      v2b_2[min(uint(i), 1u)] = true;
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
