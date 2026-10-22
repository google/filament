struct HullInputType
{
    float4 position : SV_Position;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};
struct EmptyStruct {};

struct HullOutputType {};

void blob(InputPatch<HullInputType, 3> patch)
{
}

ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

	// Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = 2;
    output.edges[1] = 2;
    output.edges[2] = 2;

	// Set the tessellation factor for tessallating inside the triangle.
    output.inside = 2;

    return output;
}


// Hull Shader
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
HullOutputType main(EmptyStruct stage_input, InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HullOutputType output;
    blob(patch);

    return output;
}

