// RUN: %dxc -E fn -T ps_6_0 %s | FileCheck %s

// CHECK: @fn

void fn() {
    // Can't use this initialization syntax yet.
    // float4 myvar = float4(1,2,3,4);
    float4 myvar;
    myvar.x = 1.0f;
    myvar.y = 1.0f;
    myvar.z = 1.0f;
    myvar.w = 1.0f;

    float4 myothervar;
}