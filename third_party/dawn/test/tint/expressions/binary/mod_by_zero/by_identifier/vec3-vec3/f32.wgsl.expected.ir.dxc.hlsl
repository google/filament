
[numthreads(1, 1, 1)]
void f() {
  float3 a = float3(1.0f, 2.0f, 3.0f);
  float3 b = float3(0.0f, 5.0f, 0.0f);
  float3 v = a;
  float3 v_1 = b;
  float3 v_2 = (v / v_1);
  float3 r = (v - ((((v_2 < (0.0f).xxx)) ? (ceil(v_2)) : (floor(v_2))) * v_1));
}

