// RUN: %dxc -T ps_6_6 -ast-dump-implicit %s | FileCheck %s


float4 main(float4 pos : SV_Position) : SV_Target {
  Texture2D<float4> tex = ResourceDescriptorHeap[0];
  SamplerState samp = SamplerDescriptorHeap[0];
  return tex.Sample(samp, pos.xy);
}

// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit struct .Resource definition
// CHECK: HLSLDynamicResourceAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit

// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit struct .Sampler definition
// CHECK: HLSLDynamicResourceAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit IsSampler
