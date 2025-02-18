struct MyStruct {
  float f1;
};


static int v1 = int(1);
static uint v2 = 1u;
static float v3 = 1.0f;
static int3 v4 = (int(1)).xxx;
static uint3 v5 = uint3(1u, 2u, 3u);
static float3 v6 = float3(1.0f, 2.0f, 3.0f);
static const MyStruct v = {1.0f};
static MyStruct v7 = v;
static const float v_1[10] = (float[10])0;
static float v8[10] = v_1;
static int v9 = int(0);
static uint v10 = 0u;
static float v11 = 0.0f;
static const MyStruct v_2 = {0.0f};
static MyStruct v12 = v_2;
static MyStruct v13 = v_2;
static float v14[10] = v_1;
static int3 v15 = int3(int(1), int(2), int(3));
static float3 v16 = float3(1.0f, 2.0f, 3.0f);
[numthreads(1, 1, 1)]
void f() {
  int l1 = v1;
  uint l2 = v2;
  float l3 = v3;
  int3 l4 = v4;
  uint3 l5 = v5;
  float3 l6 = v6;
  MyStruct l7 = v7;
  float l8[10] = v8;
  int l9 = v9;
  uint l10 = v10;
  float l11 = v11;
  MyStruct l12 = v12;
  MyStruct l13 = v13;
  float l14[10] = v14;
  int3 l15 = v15;
  float3 l16 = v16;
}

