
[numthreads(1, 1, 1)]
void f() {
  float3 a = float3(1.0f, 2.0f, 3.0f);
  float3 b = float3(4.0f, 5.0f, 6.0f);
  float3 v = (a / b);
  float3 r = (a - ((((v < (0.0f).xxx)) ? (ceil(v)) : (floor(v))) * b));
}

