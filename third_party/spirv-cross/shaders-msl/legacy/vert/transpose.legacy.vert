#version 310 es

uniform Buffer
{
	layout(row_major) mat4 MVPRowMajor;
	layout(column_major) mat4 MVPColMajor;
	mat4 M;
};

layout(location = 0) in vec4 Position;

void main()
{
	vec4 c0 = M * (MVPRowMajor * Position);
	vec4 c1 = M * (MVPColMajor * Position);
	vec4 c2 = M * (Position * MVPRowMajor);
	vec4 c3 = M * (Position * MVPColMajor);
	gl_Position = c0 + c1 + c2 + c3;
}

