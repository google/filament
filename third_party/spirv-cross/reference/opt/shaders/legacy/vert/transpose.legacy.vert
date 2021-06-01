#version 100

struct Buffer
{
    mat4 MVPRowMajor;
    mat4 MVPColMajor;
    mat4 M;
};

uniform Buffer _13;

attribute vec4 Position;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

mat4 spvTranspose(mat4 m)
{
    return mat4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
}

void main()
{
    mat4 _55 = _13.MVPRowMajor;
    mat4 _61 = spvWorkaroundRowMajor(_13.MVPColMajor);
    mat4 _80 = spvTranspose(_13.MVPRowMajor) * 2.0;
    mat4 _87 = spvTranspose(_61) * 2.0;
    gl_Position = (((((((((((spvWorkaroundRowMajor(_13.M) * (Position * _13.MVPRowMajor)) + (spvWorkaroundRowMajor(_13.M) * (spvWorkaroundRowMajor(_13.MVPColMajor) * Position))) + (spvWorkaroundRowMajor(_13.M) * (_13.MVPRowMajor * Position))) + (spvWorkaroundRowMajor(_13.M) * (Position * spvWorkaroundRowMajor(_13.MVPColMajor)))) + (_55 * Position)) + (Position * _61)) + (Position * _55)) + (_61 * Position)) + (_80 * Position)) + (_87 * Position)) + (Position * _80)) + (Position * _87);
}

