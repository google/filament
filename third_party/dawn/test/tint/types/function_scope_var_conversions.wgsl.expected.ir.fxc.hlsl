
void constant_with_non_constant() {
  float a = 0.0f;
  float2 b = float2(1.0f, a);
}

[numthreads(1, 1, 1)]
void main() {
  bool bool_var1 = true;
  bool bool_var2 = true;
  bool bool_var3 = true;
  int i32_var1 = int(123);
  int i32_var2 = int(123);
  int i32_var3 = int(1);
  uint u32_var1 = 123u;
  uint u32_var2 = 123u;
  uint u32_var3 = 1u;
  bool3 v3bool_var1 = (true).xxx;
  bool3 v3bool_var11 = (true).xxx;
  bool3 v3bool_var2 = (true).xxx;
  bool3 v3bool_var3 = (true).xxx;
  int3 v3i32_var1 = (int(123)).xxx;
  int3 v3i32_var2 = (int(123)).xxx;
  int3 v3i32_var3 = (int(1)).xxx;
  uint3 v3u32_var1 = (123u).xxx;
  uint3 v3u32_var2 = (123u).xxx;
  uint3 v3u32_var3 = (1u).xxx;
  bool3 v3bool_var4 = (true).xxx;
  bool4 v4bool_var5 = bool4(true, false, true, false);
  constant_with_non_constant();
}

