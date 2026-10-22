// RUN: %dxc -E main -T ps_6_5 %s | FileCheck %s -check-prefix=FLAG
// RUN: %dxc -E main -T ps_6_5 -validator-version 1.8 %s | FileCheck %s -check-prefix=NOFLAG

// Verify that the Sampler feedback shader feature flag is set when any
// sampler feedback instruction is used, and that it is gated off for
// validator versions before 1.9.

// FLAG: Note: shader requires additional functionality:
// FLAG: Sampler feedback

// NOFLAG-NOT: Sampler feedback

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> feedbackMinMip;
Texture2D<float> texture2D;
SamplerState samp;

float main() : SV_Target {
  feedbackMinMip.WriteSamplerFeedback(texture2D, samp, (float2)0);
  return 0;
}
