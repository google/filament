// RUN: %dxc -E main -T ps_6_5 %s | FileCheck %s

// Test that metadata encodes the sampler feedback type.

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMip;
FeedbackTexture2D<SAMPLER_FEEDBACK_MIP_REGION_USED> feedbackMipRegionUsed;
Texture2D<float> texture2D;
SamplerState samp;

float main() : SV_Target 
{
    feedbackMinMip.WriteSamplerFeedback(texture2D, samp, (float2)0);
    feedbackMipRegionUsed.WriteSamplerFeedback(texture2D, samp, (float2)0);
    return 0;
}

// CHECK-DAG: !{i32 0, %"class.FeedbackTexture2D<0>"* undef, !"feedbackMinMip", i32 0, i32 0, i32 1, i32 17, i1 false, i1 false, i1 false, ![[minmip:.*]]}
// CHECK-DAG: ![[minmip]] = !{i32 2, i32 0}
// CHECK-DAG: !{i32 1, %"class.FeedbackTexture2D<1>"* undef, !"feedbackMipRegionUsed", i32 0, i32 1, i32 1, i32 17, i1 false, i1 false, i1 false, ![[mipregionused:.*]]}
// CHECK-DAG: ![[mipregionused]] = !{i32 2, i32 1}