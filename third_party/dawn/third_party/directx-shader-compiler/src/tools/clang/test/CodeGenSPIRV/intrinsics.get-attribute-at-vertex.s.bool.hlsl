// RUN: not %dxc -T ps_6_1 -E main  %s -spirv  2>&1 | FileCheck %s

enum VertexID { 
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};

struct PSInput
{
    float4 position : SV_POSITION;
    nointerpolation bool3 color1 : COLOR1;
    nointerpolation bool3 color2 : COLOR2;
};

bool3 main(PSInput input ) : SV_Target
{
    bool3 vColor0 = input.color1;
    bool3 vColor1 = GetAttributeAtVertex( input.color1, VertexID::SECOND );
    bool3 vColor2 = input.color2;
    bool3 vColor3 = GetAttributeAtVertex( input.color2, VertexID::THIRD );
    return and(and(and(vColor0, vColor1), vColor2), vColor3);
}
// CHECK: error: attribute evaluation can only be done on values taken directly from inputs.
// CHECK: error: attribute evaluation can only be done on values taken directly from inputs.