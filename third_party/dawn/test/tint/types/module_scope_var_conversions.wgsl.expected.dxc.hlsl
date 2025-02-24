static bool bool_var1 = true;
static bool bool_var2 = true;
static bool bool_var3 = true;
static int i32_var1 = 1;
static int i32_var2 = 1;
static int i32_var3 = 1;
static uint u32_var1 = 1u;
static uint u32_var2 = 1u;
static uint u32_var3 = 1u;
static bool3 v3bool_var1 = (true).xxx;
static bool3 v3bool_var2 = (true).xxx;
static bool3 v3bool_var3 = (true).xxx;
static int3 v3i32_var1 = (1).xxx;
static int3 v3i32_var2 = (1).xxx;
static int3 v3i32_var3 = (1).xxx;
static uint3 v3u32_var1 = (1u).xxx;
static uint3 v3u32_var2 = (1u).xxx;
static uint3 v3u32_var3 = (1u).xxx;
static bool3 v3bool_var4 = (true).xxx;
static bool4 v4bool_var5 = bool4(true, false, true, false);

[numthreads(1, 1, 1)]
void main() {
  bool_var1 = false;
  bool_var2 = false;
  bool_var3 = false;
  i32_var1 = 0;
  i32_var2 = 0;
  i32_var3 = 0;
  u32_var1 = 0u;
  u32_var2 = 0u;
  u32_var3 = 0u;
  v3bool_var1 = (false).xxx;
  v3bool_var2 = (false).xxx;
  v3bool_var3 = (false).xxx;
  v3bool_var4 = (false).xxx;
  v4bool_var5 = (false).xxxx;
  v3i32_var1 = (0).xxx;
  v3i32_var2 = (0).xxx;
  v3i32_var3 = (0).xxx;
  v3u32_var1 = (0u).xxx;
  v3u32_var2 = (0u).xxx;
  v3u32_var3 = (0u).xxx;
  return;
}
