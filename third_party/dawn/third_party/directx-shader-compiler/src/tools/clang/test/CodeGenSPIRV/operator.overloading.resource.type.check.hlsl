// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct INPUT
{
    float4 Pos : SV_Position;
};

struct OUTPUT
{
    float x[3] : SV_TessFactor;
};

[outputcontrolpoints(3)]
[outputtopology("point")]
[patchconstantfunc("foo")]

INPUT main(InputPatch<INPUT, 3> Input,
           uint PointID : SV_OutputControlPointID)
{
// CHECK: [[PointID:%[0-9]+]] = OpLoad %uint %PointID
// CHECK: OpAccessChain %_ptr_Function_INPUT %Input [[PointID]]
    return Input[PointID];
}

OUTPUT foo()
{
    return (OUTPUT)0;
}
