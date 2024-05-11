#version 440
layout(triangles) in;
layout(max_vertices = 5, triangle_strip) out;

struct Data
{
    vec4 ApiPerspectivePosition;
};

void Copy(inout Data inputStream[3])
{
    inputStream[0].ApiPerspectivePosition = gl_in[0].gl_Position;
}

void main()
{
    Data inputStream[3];
    Data param[3] = inputStream;
    Copy(param);
    inputStream = param;
    gl_Position = inputStream[0].ApiPerspectivePosition;
}

