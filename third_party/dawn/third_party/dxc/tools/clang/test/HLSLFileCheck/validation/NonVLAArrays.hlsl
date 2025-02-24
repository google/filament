// RUN: %dxc -E PSMain -T ps_6_0 %s | FileCheck %s

// CHECK: %"$Globals" = type { [2 x <4 x float>], [3 x <4 x float>] }

// fix me: float4 BrokenTestArray0[int(.3)*12]; 
float4 WorkingTestArray0[min(2, 3)];
float4 WorkingTestArray1[max(2, 3)];

float4 PSMain(float4 color : COLOR) : SV_TARGET
{
    return WorkingTestArray0[int(color.y)] +
           WorkingTestArray1[int(color.z)];
}