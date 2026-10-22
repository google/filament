// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: @main

// Regression test for a validation error caused by nested LCSSA formation uses
// the wrong set of blocks when constructing LCSSA PHI's for a parent loop. This
// causes wrong assumptions to be made about which users are actually in the
// loop.

float bilinearWeights;
float2 main() : OUT {
    float2 output = float2(0.0f,0);
    for(uint sampleIdx = 0; sampleIdx < 1; ++sampleIdx) {
        float totalWeight = 0.0f;

        [unroll]
        for(uint i = 0; i < 1; ++i) {
            totalWeight += bilinearWeights;
        }

        float2 occlusion = float2(1.0f, 0);
        if(totalWeight > 0.0f)
            occlusion = 1 / totalWeight;

        output += occlusion;
    }

    return output;
}