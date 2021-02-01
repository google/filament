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

	vec4 c4 = transpose(MVPRowMajor) * Position;
	vec4 c5 = transpose(MVPColMajor) * Position;
	vec4 c6 = Position * transpose(MVPRowMajor);
	vec4 c7 = Position * transpose(MVPColMajor);

	// Multiplying by scalar forces resolution of the transposition
	vec4 c8 = (MVPRowMajor * 2.0) * Position;
	vec4 c9 = (transpose(MVPColMajor) * 2.0) * Position;
	vec4 c10 = Position * (MVPRowMajor * 2.0);
	vec4 c11 = Position * (transpose(MVPColMajor) * 2.0);

	gl_Position = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 + c11;
}

