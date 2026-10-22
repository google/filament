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
    mat4 _55 = _13.MVPRowMajor;
    mat4 _61 = spvWorkaroundRowMajor(_13.MVPColMajor);
    mat4 _80 = _13.MVPRowMajor * 2.0;
    mat4 _87 = _61 * 2.0;
    OutputMat1 = spvTranspose(_13.MVPRowMajor);
    OutputMat2 = spvWorkaroundRowMajor(_13.MVPColMajor);
    OutputMat3 = _55;
    OutputMat4 = spvTranspose(_61);
    mat4 _121 = spvWorkaroundRowMajor(_13.M);
    OutputMat5 = spvTranspose(_121);
    mediump mat4 _126 = spvWorkaroundRowMajor(_13.MRelaxed);
    OutputMat6 = spvTransposeMP(_126);
    OutputMat7 = spvTranspose(_121);
    OutputMat8 = spvTransposeMP(_126);
    gl_Position = (((((((((((spvWorkaroundRowMajor(_13.M) * (Position * _13.MVPRowMajor)) + (spvWorkaroundRowMajor(_13.M) * (spvWorkaroundRowMajor(_13.MVPColMajor) * Position))) + (spvWorkaroundRowMajor(_13.M) * (_13.MVPRowMajor * Position))) + (spvWorkaroundRowMajor(_13.M) * (Position * spvWorkaroundRowMajor(_13.MVPColMajor)))) + (_55 * Position)) + (Position * _61)) + (Position * _55)) + (_61 * Position)) + (Position * _80)) + (Position * _87)) + (_80 * Position)) + (_87 * Position);
}

