// RUN: %dxc -T ps_6_2 -E main -Od %s | FileCheck %s

// Regression test for a bug where the legalize sample offset pass would
// run vanilla SROA which will turn an unoptimized "alloca i1" for a return value
// into an "alloca i8", which is invalid DXIL.

// CHECK: @main

SamplerState samp;
Texture1D tex;
bool ShouldWriteFeedback() { return true; }
float4 main() : SV_Target
{
    ShouldWriteFeedback();
    int zero = 0;
    return tex.Sample(samp, 0, zero);
}