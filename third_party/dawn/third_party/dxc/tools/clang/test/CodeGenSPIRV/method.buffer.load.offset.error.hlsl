// RUN: not %dxc -T ps_6_2 -E main -fcgl -spirv %s 2>&1 | FileCheck %s

Texture2D SourceTexture;
void main(in float4 SvPosition : SV_Position, out float4 OutColor : SV_Target0) {
    // CHECK: error: Offsets to texture access operations must be immediate values.
    OutColor = SourceTexture.Load(SvPosition.x, SvPosition.y);
}
