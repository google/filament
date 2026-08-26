// RUN: %dxc -E main -T ps_6_0 %s -Gec | FileCheck -check-prefix=OUT %s

// RUN: %dxc -E main -T ps_6_0 %s -Gec | FileCheck -input-file=stderr -check-prefix=ERROR %s

// OUT: COLOR                    0   xyzw        0     NONE   float
// OUT: TEXCOORD                 0   xy          1     NONE   float
// OUT: SV_Position              0   xyzw        2      POS   float
// OUT: SV_Target                1   xyzw        1   TARGET   float   xyzw

//ERROR: warning: DX9-style semantic "CoLoR" mapped to DX10 system semantic "SV_Target" due to -Gec flag. This functionality is deprecated in newer language versions.

struct VOut
{
 float4 color : COLOR0;
 float2 UV    : TEXCOORD0;
 float4 pos   : VPOS;
};

float4 main(VOut In) : CoLoR1
{
 return float4(0,0,0,0);
}
