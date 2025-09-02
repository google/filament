// TODO: use -verify instead of FileCheck after fix https://github.com/microsoft/DirectXShaderCompiler/issues/5768
// RUN: not %dxc -E main -T vs_6_7 %s 2>&1 | FileCheck %s --check-prefix=SM67

// SM67:invalid semantic 'SV_StartVertexLocation' for vs 6.7
// SM67:invalid semantic 'SV_StartInstanceLocation' for vs 6.7

float4 main(int loc : SV_StartVertexLocation
           , uint loc2 : SV_StartInstanceLocation
           ) : SV_Position
{
    float4 r = 0;
    r += loc;
    r += loc2;
    return r;
}
