// RUN: %dxc -E main -T ps_6_0 /Gec -HV 2016  %s | FileCheck %s

// Writing to globals only supported with HV <= 2016
// CHECK: define void @main
// CHECK: ret void

float g_s;
float4 g_v = float4(1.0, -1.0, 0.0, 1.0);
int2x2 g_m;
row_major int2x2 g_m_rm;
int2x2 g_m2;
bool g_b = false;
int g_a[5];
int g_a2d[3][2];
int2x2 g_ma[4];

float4 main(uint a : A) : SV_Target {
  // update global scalar
  g_s = a;

  // update global vector
  g_v = float4(a + 1, a + 2, a + 3, a + 4);

  // update global matrix
  for (uint i = 0; i < 2; i++)
    for (uint j = 0; j < 2; j++)
      g_m[i][j] = a + i + j;
      
  // update global row_major matrix
  for (uint i = 0; i < 2; i++)
    for (uint j = 0; j < 2; j++)
      g_m_rm[i][j] = a + i + j;

  // update global matrix through element access
  g_m2._11 = a;
  g_m2._12_21 = float2(a, a + 1);
  g_m2._21_22 = a + 3;

  // update global boolean
  g_b = true;

  // update global array
  for (uint i = 0; i < 5; i++)
    g_a[i] = a + i;
  
  // update global 2d array
  for (uint i = 0; i < 3; i++)
    for (uint j = 0; j < 2; j++)
      g_a2d[i][j] = a + i + j;

  // update global matrix array
  for (uint i = 0; i < 4; i++)
    for (uint j = 0; j < 2; j++)
      for (uint k = 0; k < 2; k++)
        g_ma[i][j][k] = a + i + j;

  return float4(g_s, g_s, g_s, g_s) +
         g_v +
         float4(g_m[0][0], g_m[0][1], g_m[1][0], g_m[1][1]) +
         float4(g_m_rm[0][0], g_m_rm[0][1], g_m_rm[1][0], g_m_rm[1][1]) +
         float4(g_m2._11, g_m2._12, g_m2._21_22) +
         float4(g_a[0], g_a[1], g_a[2], g_a[3]) +
         float4(g_a2d[0][0], g_a2d[0][1], g_a2d[1][0], g_a2d[1][1]) +
         float4(g_ma[0][0][0], g_ma[1][0][1], g_ma[2][1][0], g_ma[3][1][1]);
}