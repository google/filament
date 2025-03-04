// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// 4 GEP for copy, last one for indexing.
// CHECK: getelementptr
// CHECK: getelementptr
// CHECK: getelementptr
// CHECK: getelementptr
// CHECK: getelementptr

struct Interpolants
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

uint i;
float4 main( Interpolants In ) : SV_TARGET
{
    return In.color[i];
}