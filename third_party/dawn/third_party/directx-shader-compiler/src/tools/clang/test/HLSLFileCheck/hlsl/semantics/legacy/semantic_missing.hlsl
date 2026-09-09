// RUN: %dxc /Tps_6_0 %s | FileCheck %s 
// CHECK: error: Semantic must be defined for all outputs of an entry function or patch constant function

[RootSignature("")]
float main(float a : A, uint b : SV_Coverage)
{
    return a;
}