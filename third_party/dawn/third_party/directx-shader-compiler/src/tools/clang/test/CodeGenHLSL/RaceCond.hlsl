// RUN: %dxc -E main  -T cs_6_0 %s | FileCheck %s

// CHECK: Race condition writing to shared memory detected, consider making this write conditional

RWBuffer< int > g_Intensities : register(u1);

groupshared int sharedData;

[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
sharedData = GI;
InterlockedAdd(sharedData, g_Intensities[GI]);
g_Intensities[GI] = sharedData;
}