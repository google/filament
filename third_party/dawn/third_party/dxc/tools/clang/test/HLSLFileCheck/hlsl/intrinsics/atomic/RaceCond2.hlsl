// RUN: %dxc -E main  -T cs_6_0 %s | FileCheck %s

// CHECK: @main
RWBuffer< int > g_Intensities : register(u1);

groupshared int sharedData;

int a;
int b[2];
[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
  sharedData = b[0];
  InterlockedAdd(sharedData, g_Intensities[GI]);
  g_Intensities[GI] = sharedData;
}