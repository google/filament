// RUN: %dxc -E main -T ps_6_0 %s | FileCheck -input-file=stderr %s

// CHECK: warning: Ignoring unsupported 'VFACE' in the target attribute string

float4 main(float4 color : COLOR, uint vface : VFACE) : SV_TARGET
{
  return (vface > 0) ? color : (color*2);
}
