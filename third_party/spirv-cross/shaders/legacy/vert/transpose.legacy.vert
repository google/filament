#version 310 es

uniform Buffer
{
	layout(row_major) mat4 MVPRowMajor;
	layout(column_major) mat4 MVPColMajor;
	mat4 M;
	mediump mat4 MRelaxed;
};

layout(location = 0) in vec4 Position;

layout(location = 1) out mat4 OutputMat1;
layout(location = 5) out mat4 OutputMat2;
layout(location = 9) out mat4 OutputMat3;
layout(location = 13) out mat4 OutputMat4;
layout(location = 17) out highp mat4 OutputMat5;
layout(location = 21) out highp mat4 OutputMat6;
layout(location = 25) out mediump mat4 OutputMat7;
layout(location = 29) out mediump mat4 OutputMat8;

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

	// Multiplying by scalar does not affect transposition
	vec4 c8 = (MVPRowMajor * 2.0) * Position;
	vec4 c9 = (transpose(MVPColMajor) * 2.0) * Position;
	vec4 c10 = Position * (MVPRowMajor * 2.0);
	vec4 c11 = Position * (transpose(MVPColMajor) * 2.0);

	// Outputting a matrix will trigger an actual transposition
	OutputMat1 = MVPRowMajor;
	OutputMat2 = MVPColMajor;
	OutputMat3 = transpose(MVPRowMajor);
	OutputMat4 = transpose(MVPColMajor);

	// Preserve precision of input, ignore precision of output
	OutputMat5 = transpose(M);
	OutputMat6 = transpose(MRelaxed);
	OutputMat7 = transpose(M);
	OutputMat8 = transpose(MRelaxed);

	gl_Position = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 + c11;
}

