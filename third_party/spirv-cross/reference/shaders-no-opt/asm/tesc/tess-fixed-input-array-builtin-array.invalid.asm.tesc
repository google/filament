#version 450
layout(vertices = 3) out;

struct VertexOutput
{
    vec4 pos;
    vec2 uv;
};

struct HSOut
{
    vec4 pos;
    vec2 uv;
};

struct HSConstantOut
{
    float EdgeTess[3];
    float InsideTess;
};

struct VertexOutput_1
{
    vec2 uv;
};

struct HSOut_1
{
    vec2 uv;
};

layout(location = 0) in VertexOutput_1 p[];
layout(location = 0) out HSOut_1 _entryPointOutput[3];

HSOut _hs_main(VertexOutput p_1[3], uint i)
{
    HSOut _output;
    _output.pos = p_1[i].pos;
    _output.uv = p_1[i].uv;
    return _output;
}

HSConstantOut PatchHS(VertexOutput _patch[3])
{
    HSConstantOut _output;
    _output.EdgeTess[0] = (vec2(1.0) + _patch[0].uv).x;
    _output.EdgeTess[1] = (vec2(1.0) + _patch[0].uv).x;
    _output.EdgeTess[2] = (vec2(1.0) + _patch[0].uv).x;
    _output.InsideTess = (vec2(1.0) + _patch[0].uv).x;
    return _output;
}

void main()
{
    VertexOutput p_1[3];
    p_1[0].pos = gl_in[0].gl_Position;
    p_1[0].uv = p[0].uv;
    p_1[1].pos = gl_in[1].gl_Position;
    p_1[1].uv = p[1].uv;
    p_1[2].pos = gl_in[2].gl_Position;
    p_1[2].uv = p[2].uv;
    uint i = gl_InvocationID;
    VertexOutput param[3] = p_1;
    uint param_1 = i;
    HSOut flattenTemp = _hs_main(param, param_1);
    gl_out[gl_InvocationID].gl_Position = flattenTemp.pos;
    _entryPointOutput[gl_InvocationID].uv = flattenTemp.uv;
    barrier();
    if (int(gl_InvocationID) == 0)
    {
        VertexOutput param_2[3] = p_1;
        HSConstantOut _patchConstantResult = PatchHS(param_2);
        gl_TessLevelOuter[0] = _patchConstantResult.EdgeTess[0];
        gl_TessLevelOuter[1] = _patchConstantResult.EdgeTess[1];
        gl_TessLevelOuter[2] = _patchConstantResult.EdgeTess[2];
        gl_TessLevelInner[0] = _patchConstantResult.InsideTess;
    }
}

