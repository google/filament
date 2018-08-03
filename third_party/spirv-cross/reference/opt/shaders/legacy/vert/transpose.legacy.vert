#version 100

struct Buffer
{
    mat4 MVPRowMajor;
    mat4 MVPColMajor;
    mat4 M;
};

uniform Buffer _13;

attribute vec4 Position;

void main()
{
    gl_Position = (((_13.M * (Position * _13.MVPRowMajor)) + (_13.M * (_13.MVPColMajor * Position))) + (_13.M * (_13.MVPRowMajor * Position))) + (_13.M * (Position * _13.MVPColMajor));
}

