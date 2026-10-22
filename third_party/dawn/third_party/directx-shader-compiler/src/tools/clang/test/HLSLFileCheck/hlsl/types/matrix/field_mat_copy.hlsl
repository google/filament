// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure matrix only struct copy works.
// CHECK: main

struct Mat
{
 float2x2 m;
};

Mat m;

float4 main() :SV_Target {
    Mat lm = m;
    return lm.m;
}