// RUN: %dxc -T lib_6_6 -D ENTRY=MyMissShader66 -Fo t_66 %s | FileCheck %s -check-prefix=CHK66
// RUN: %dxc -T lib_6_5 -D ENTRY=MyMissShader65 -Fo t_65 %s | FileCheck %s -check-prefix=CHK65
// RUN: %dxc -T lib_6_6 -D ENTRY=MyOtherMissShader66 -Fo t2_66 %s | FileCheck %s -check-prefix=CHK66
// RUN: %dxc -T lib_6_5 -D ENTRY=MyOtherMissShader65 -Fo t2_65 %s | FileCheck %s -check-prefix=CHK65

// RUN: %dxl -T lib_6_6 t_65;t_66 %s  | FileCheck %s -check-prefixes=CHK66,CHKLINK
// RUN: %dxl -T lib_6_6 t_66;t_65 %s  | FileCheck %s -check-prefixes=CHK66,CHKLINK
// RUN: %dxl -T lib_6_6 t_65;t2_66 %s | FileCheck %s -check-prefixes=CHK66,CHKLINK
// RUN: %dxl -T lib_6_6 t_66;t2_65 %s | FileCheck %s -check-prefixes=CHK66,CHKLINK
// RUN: %dxl -T lib_6_6 t2_65;t_66 %s | FileCheck %s -check-prefixes=CHK66,CHKLINK
// RUN: %dxl -T lib_6_6 t2_66;t_65 %s | FileCheck %s -check-prefixes=CHK66,CHKLINK

// TODO: Test down-conversion for final target as well (currently does not work)

// You might wonder, why all the combinations?  Well, function names determine
// order of expansion, which would impact which conflicting global would win
// between one of handle type or one of the original HL cbuffer type.

// CHK66-NOT: @dx.op.createHandleForLib.ColorCb
// CHK66: @dx.op.createHandleForLib.dx.types.Handle

// CHK65-NOT: @dx.op.createHandleForLib.dx.types.Handle
// CHK65: @dx.op.createHandleForLib.ColorCb

// CHKLINK-DAG: define void {{.*}}MissShader65
// CHKLINK-DAG: define void {{.*}}MissShader66

struct Color {float4 a;};
ConstantBuffer<Color> ColorCb : register(b0);

struct RayPayload
{
    float4 color;
};

[shader("miss")]
void ENTRY(inout RayPayload payload)
{
    payload.color = ColorCb.a;
}
