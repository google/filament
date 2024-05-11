#version 100

struct Buffer
{
    mat4 MVPRowMajor;
    mat4 MVPColMajor;
    mat4 M;
    mediump mat4 MRelaxed;
};

uniform Buffer _13;

attribute vec4 Position;
varying mat4 OutputMat1;
varying mat4 OutputMat2;
varying mat4 OutputMat3;
varying mat4 OutputMat4;
varying mat4 OutputMat5;
varying mat4 OutputMat6;
varying mediump mat4 OutputMat7;
varying mediump mat4 OutputMat8;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

highp mat4 spvTranspose(highp mat4 m)
{
    return mat4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
}

mediump mat4 spvTransposeMP(mediump mat4 m)
{
    return mat4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
}

void main()
{
    vec4 c0 = spvWorkaroundRowMajor(_13.M) * (Position * _13.MVPRowMajor);
    vec4 c1 = spvWorkaroundRowMajor(_13.M) * (spvWorkaroundRowMajor(_13.MVPColMajor) * Position);
    vec4 c2 = spvWorkaroundRowMajor(_13.M) * (_13.MVPRowMajor * Position);
    vec4 c3 = spvWorkaroundRowMajor(_13.M) * (Position * spvWorkaroundRowMajor(_13.MVPColMajor));
    vec4 c4 = _13.MVPRowMajor * Position;
    vec4 c5 = Position * spvWorkaroundRowMajor(_13.MVPColMajor);
    vec4 c6 = Position * _13.MVPRowMajor;
    vec4 c7 = spvWorkaroundRowMajor(_13.MVPColMajor) * Position;
    vec4 c8 = Position * (_13.MVPRowMajor * 2.0);
    vec4 c9 = Position * (spvWorkaroundRowMajor(_13.MVPColMajor) * 2.0);
    vec4 c10 = (_13.MVPRowMajor * 2.0) * Position;
    vec4 c11 = (spvWorkaroundRowMajor(_13.MVPColMajor) * 2.0) * Position;
    OutputMat1 = spvTranspose(_13.MVPRowMajor);
    OutputMat2 = spvWorkaroundRowMajor(_13.MVPColMajor);
    OutputMat3 = _13.MVPRowMajor;
    OutputMat4 = spvTranspose(spvWorkaroundRowMajor(_13.MVPColMajor));
    OutputMat5 = spvTranspose(spvWorkaroundRowMajor(_13.M));
    OutputMat6 = spvTransposeMP(spvWorkaroundRowMajor(_13.MRelaxed));
    OutputMat7 = spvTranspose(spvWorkaroundRowMajor(_13.M));
    OutputMat8 = spvTransposeMP(spvWorkaroundRowMajor(_13.MRelaxed));
    gl_Position = ((((((((((c0 + c1) + c2) + c3) + c4) + c5) + c6) + c7) + c8) + c9) + c10) + c11;
}

