// This file is not used directly for testing.
// This is the HLSL source for validation of disallowed 6.9 features in previous shader models.
// It is used to generate LitDxilValidation/vector-validation.ll using `dxc -T ps_6_9`.
// Output is modified to have shader model 6.8 instead.

RWStructuredBuffer<float4> VecBuf;

// some simple ways to generate the vector ops in question.
float4 main(float val : VAL) :SV_Position {
  float4 vec = VecBuf[1];
  VecBuf[0] = val;
  return vec[2];
}

