// RUN: not %dxc -DTEST0 -T ps_6_5 -fcgl -spirv %s 2>&1 | FileCheck %s --check-prefix=CHECK0
// RUN: not %dxc -DTEST1 -T ps_6_5 -fcgl -spirv %s 2>&1 | FileCheck %s --check-prefix=CHECK1
// RUN: not %dxc -DTEST2 -T ps_6_5 -fcgl -spirv %s 2>&1 | FileCheck %s --check-prefix=CHECK2
// RUN: not %dxc -DTEST3 -T ps_6_5 -fcgl -spirv %s 2>&1 | FileCheck %s --check-prefix=CHECK3

#ifdef TEST0
// CHECK0: error: Texture resource type 'FeedbackTexture2D' is not supported with -spirv
FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMip;

#elif TEST1
// CHECK1: error: Texture resource type 'FeedbackTexture2D' is not supported with -spirv
FeedbackTexture2D<SAMPLER_FEEDBACK_MIP_REGION_USED> feedbackMipRegionUsed;

#elif TEST2
// CHECK2: error: Texture resource type 'FeedbackTexture2DArray' is not supported with -spirv
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMipArray;

#elif TEST3
// CHECK3: error: Texture resource type 'FeedbackTexture2DArray' is not supported with -spirv
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIP_REGION_USED> feebackMipRegionUsedArray;

#endif

float main() : SV_Target
{
    return 0;
}
