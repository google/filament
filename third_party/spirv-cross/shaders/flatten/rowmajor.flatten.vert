#version 310 es

layout(std140) uniform UBO
{
    layout(column_major) mat4 uMVPR;
    layout(row_major) mat4 uMVPC;
    layout(row_major) mat2x4 uMVP;
};

layout(location = 0) in vec4 aVertex;

void main()
{
	vec2 v = aVertex * uMVP;
	gl_Position = uMVPR * aVertex + uMVPC * aVertex;
}
