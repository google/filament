// This file is not used directly for testing.
// This is the HLSL source for validation of various invalid load/store parameters.
// It is used to generate LitDxilValidation/load-store-validation.ll using `dxc -T ps_6_9`.
// Output is modified to trigger various validation errors.

Texture1D<float4> Tex;
RWTexture1D<float4> RwTex;
SamplerState Samp;

StructuredBuffer<float4> VecBuf;
StructuredBuffer<float> ScalBuf;
ByteAddressBuffer BaBuf;

RWStructuredBuffer<float4> OutVecBuf;
RWStructuredBuffer<float> OutScalBuf;
RWByteAddressBuffer OutBaBuf;

// Some simple ways to generate the vector ops in question.
float4 main(int i : IX) : SV_Target {
  // Texture provides some invalid handles to plug in.
  float4 TexVal = Tex.Sample(Samp, i);
  RwTex[0] = TexVal;

  // For invalid RC on Load (and inevitably invalid RK).
  float BadRCLd = ScalBuf[0];
  // For invalid RK on Load.
  float BadRKLd = ScalBuf[1];
  // For non-constant alignment on Load.
  float BadAlnLd = ScalBuf[2];
  // For undefined offset on Structured Buffer Load.
  float BadStrOffLd = ScalBuf[3];
  // For defined (and therefore invalid) offset on Byte Address Buffer Load.
  float BadBabOffLd = BaBuf.Load<float>(0);

  // For invalid RC on Vector Load (and inevitably invalid RK).
  float4 BadRCVcLd = VecBuf[0];
  // For invalid RK on Vector Load.
  float4 BadRKVcLd = VecBuf[1];
  // For non-constant alignment on Vector Load.
  float4 BadAlnVcLd = VecBuf[2];
  // For undefined offset on Structured Buffer Vector Load.
  float4 BadStrOffVcLd = VecBuf[3];
  // For defined (and therefore invalid) offset on Byte Address Buffer Vector Load.
  float4 BadBabOffVcLd = BaBuf.Load<float4>(4);

  // For Store to non-UAV.
  OutScalBuf[0] = BadRCLd;
  // For invalid RK on Store.
  OutScalBuf[1] = BadRKLd;
  // For non-constant alignment on Store.
  OutScalBuf[2] = BadAlnLd;
  // For undefined offset on Structured Buffer Store.
  OutScalBuf[3] = BadStrOffLd;
  // For undefined value Store.
  OutScalBuf[4] = 77;
  // For defined (and therefore invalid) offset on Byte Address Buffer Store.
  OutBaBuf.Store<float>(0, BadBabOffLd);

  // For Vector Store to non-UAV.
  OutVecBuf[0] = BadRCVcLd;
  // For invalid RK on Vector Store.
  OutVecBuf[1] = BadRKVcLd;
  // For non-constant alignment on Vector Store.
  OutVecBuf[2] = BadAlnVcLd;
  // For undefined offset on Structured Buffer Vector Store.
  OutVecBuf[3] = BadStrOffVcLd;
  // For undefinded value Vector Store.
  OutVecBuf[4] = 77;
  // For defined (and therefore invalid) offset on Byte Address Buffer Vector Store.
  OutBaBuf.Store<float4>(4, BadBabOffVcLd);

  return TexVal;
}

