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
    vec4 c0 = spvWorkaroundRowMajor(_13.M) * (Position * _13.MVPRowMajor);
    vec4 c1 = spvWorkaroundRowMajor(_13.M) * (spvWorkaroundRowMajor(_13.MVPColMajor) * Position);
    vec4 c2 = spvWorkaroundRowMajor(_13.M) * (_13.MVPRowMajor * Position);
    vec4 c3 = spvWorkaroundRowMajor(_13.M) * (Position * spvWorkaroundRowMajor(_13.MVPColMajor));
    vec4 c4 = _13.MVPRowMajor * Position;
    vec4 c5 = Position * spvWorkaroundRowMajor(_13.MVPColMajor);
    vec4 c6 = Position * _13.MVPRowMajor;
    vec4 c7 = spvWorkaroundRowMajor(_13.MVPColMajor) * Position;
    vec4 c8 = (spvTranspose(_13.MVPRowMajor) * 2.0) * Position;
    vec4 c9 = (spvTranspose(spvWorkaroundRowMajor(_13.MVPColMajor)) * 2.0) * Position;
    vec4 c10 = Position * (spvTranspose(_13.MVPRowMajor) * 2.0);
    vec4 c11 = Position * (spvTranspose(spvWorkaroundRowMajor(_13.MVPColMajor)) * 2.0);
    gl_Position = ((((((((((c0 + c1) + c2) + c3) + c4) + c5) + c6) + c7) + c8) + c9) + c10) + c11;
}

