// RUN: %dxc -E main -T ps_6_5 %s | FileCheck %s

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMip;
Texture2D<float> texture2D;
SamplerState samp;

// CHECK: define void @main()
float main(float2 coord0 : TEXCOORD0, float2 coord1 : TEXCOORD1) : SV_Target
{
    // Ensure WriteSamplerFeedback coord is considered convergent, and fmul does not sink
    // CHECK: fmul
    // CHECK: fmul
    float2 a = coord0 * coord1;

    // CHECK: br i1
    if (coord0.x > 0.5) {
        // CHECK: call void @dx.op.writeSamplerFeedback
        feedbackMinMip.WriteSamplerFeedback(texture2D, samp, a, 1.0);
    }
    // CHECK: br label

    return 0;
}
