// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// Tests that we prevent multiple inputs/outputs from having the same semantics.
// Also serves as a regresion test for an SROA crash in the struct case.

// CHECK: validation errors
// CHECK: Semantic 'TEXCOORD' overlap at 0

struct Texcoords
{
    float u : TEXCOORD0;
    float v : TEXCOORD0;
};

float main(Texcoords texcoords) : SV_Target
{
    return 1;
}