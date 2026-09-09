// RUN: not %dxc -T ps_6_0 -E main -fvk-bind-register t5 0 1 2 -vkbr s3 1 3 4 -fcgl  %s -spirv  2>&1 | FileCheck %s

Texture2D MyTexture    : register(t5);
SamplerState MySampler : register(s3, space1);

int globalInteger;
float4 globalFloat4;

float4 main() : SV_Target {
  return MyTexture.Sample(MySampler, float2(0.1, 0.2));
}

// CHECK: error: -fvk-bind-register requires Globals buffer to be bound with -fvk-bind-globals
