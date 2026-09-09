// RUN: %dxc -T ps_6_0 -Od -E main %s | FileCheck %s 

// CHECK: @main

// make sure 'center' is allowed as an interpolation modifier
float main(center float t : T) : SV_TARGET
{
    // and also as an identifier
    float center = 10.0f;
    return center * 2;
}
