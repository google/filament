// RUN: %dxc -E main -T ps_6_0 %s -Gec | FileCheck -check-prefix=OUT %s

// RUN: %dxc -E main -T ps_6_0 %s -Gec | FileCheck -input-file=stderr -check-prefix=ERROR %s

// OUT: SV_Position              0   xyzw        0      POS   float
// OUT: SV_Target                1   xyzw        1   TARGET   float   xyzw
// OUT: SV_Depth                 0    N/A   oDepth    DEPTH   float    YES

// ERROR: warning: DX9-style semantic "VPOS" mapped to DX10 system semantic "SV_Position" due to -Gec flag. This functionality is deprecated in newer language versions.

struct PSIn
{
 float4 Position;
};

struct PSOut
{
  float4 c : COLOR1;
  float d  : DEPTH;
};

PSOut main(PSIn In : VPOS) 
{
    PSOut retValue = { {1, 0, 1, 0}, 0.5 };
    return retValue;
}
