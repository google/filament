
static float2 v2f = (0.0f).xx;
static int3 v3i = (int(0)).xxx;
static uint4 v4u = (0u).xxxx;
static bool2 v2b = (false).xx;
void foo() {
  int i = int(0);
  int v = i;
  float2 v_1 = v2f;
  float2 v_2 = float2((1.0f).xx);
  float2 v_3 = float2((v).xx);
  v2f = (((v_3 == float2(int(0), int(1)))) ? (v_2) : (v_1));
  int v_4 = i;
  int3 v_5 = v3i;
  int3 v_6 = int3((int(1)).xxx);
  int3 v_7 = int3((v_4).xxx);
  v3i = (((v_7 == int3(int(0), int(1), int(2)))) ? (v_6) : (v_5));
  int v_8 = i;
  uint4 v_9 = v4u;
  uint4 v_10 = uint4((1u).xxxx);
  uint4 v_11 = uint4((v_8).xxxx);
  v4u = (((v_11 == uint4(int(0), int(1), int(2), int(3)))) ? (v_10) : (v_9));
  int v_12 = i;
  bool2 v_13 = v2b;
  bool2 v_14 = bool2((true).xx);
  bool2 v_15 = bool2((v_12).xx);
  v2b = (((v_15 == bool2(int(0), int(1)))) ? (v_14) : (v_13));
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      foo();
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

