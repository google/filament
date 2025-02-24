// RUN: %dxc -E MainVp -T vs_6_0 %s | %D3DReflect %s | FileCheck %s

// CHECK: ID3D12ShaderReflection:
// CHECK: OutputParameters: 2

void MainVp (out float4 rw_sv_clipdistance : SV_ClipDistance,
             out float4 rw_sv_clipdistance1 : SV_ClipDistance1)
{
   rw_sv_clipdistance = float4(0.0, 0.0, 0.0, 0.0);
   rw_sv_clipdistance1 = float4(0.0, 0.0, 0.0, 0.0);
}
