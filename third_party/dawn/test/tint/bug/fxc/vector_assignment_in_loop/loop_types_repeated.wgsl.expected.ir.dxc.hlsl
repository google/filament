
[numthreads(1, 1, 1)]
void main() {
  float2 v2f = (0.0f).xx;
  float2 v2f_2 = (0.0f).xx;
  int3 v3i = (int(0)).xxx;
  int3 v3i_2 = (int(0)).xxx;
  uint4 v4u = (0u).xxxx;
  uint4 v4u_2 = (0u).xxxx;
  bool2 v2b = (false).xx;
  bool2 v2b_2 = (false).xx;
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      v2f[min(uint(i), 1u)] = 1.0f;
      v3i[min(uint(i), 2u)] = int(1);
      v4u[min(uint(i), 3u)] = 1u;
      v2b[min(uint(i), 1u)] = true;
      v2f_2[min(uint(i), 1u)] = 1.0f;
      v3i_2[min(uint(i), 2u)] = int(1);
      v4u_2[min(uint(i), 3u)] = 1u;
      v2b_2[min(uint(i), 1u)] = true;
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

