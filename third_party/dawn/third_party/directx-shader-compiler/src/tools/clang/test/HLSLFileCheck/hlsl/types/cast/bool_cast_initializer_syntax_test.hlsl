// RUN: %dxc /T ps_6_0 /E main -HV 2018 %s | FileCheck %s

// Ensure that when casting to bool, we never use fptoui or fptosi instruction
// CHECK-NOT: fptoui
// CHECK-NOT: fptosi

// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une

// CHECK: icmp ne
// CHECK: icmp ne
// CHECK: icmp ne
// CHECK: icmp ne

// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une

// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une
  
// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une

// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une
// CHECK: fcmp fast une

// CHECK: icmp ne
// CHECK: icmp ne
// CHECK: fcmp fast une

// CHECK: fcmp fast une
// CHECK: fcmp fast une

// CHECK: icmp ne
// CHECK: fcmp fast une

// CHECK: fcmp fast une
// CHECK: fcmp fast une

bool4 main (float f1 : F1, 
            float2 f2 : F2,
            float3 f3 : F3,
            float4 f4 : F4,

            int i1 : I1,
            int2 i2 : I2,
            int3 i3 : I3,
            int4 i4 : I4,

            min10float mtf1 : M1,
            min10float2 mtf2 : M2,
            min10float3 mtf3 : M3,
            min10float4 mtf4 : M4,

            min16float msf1 : M5,
            min16float2 msf2 : M6,
            min16float3 msf3 : M7,
            min16float4 msf4 : M8,

            half h1 : H1,
            half2 h2 : H2,
            half3 h3 : H3,
            half4 h4 : H4) : SV_Target
{ 
    return
    bool4(f4) &&
    bool4(i4) &&
    bool4(mtf4) &&
    bool4(msf4) &&
    bool4(h4) &&                
    bool4(bool3(f3), f1) &&
    bool4(bool2(i2), f1, h1) &&
    bool4(f3, f1) &&
    bool4(bool2(i2), bool2(msf2)) &&
    bool4(i1, f1, mtf1, h1) &&
    bool4(i4) &&
    bool4(mtf2, msf2);
}
