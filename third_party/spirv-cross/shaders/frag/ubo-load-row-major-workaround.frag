#version 450

struct RowMajor
{
	mat4 B;
};

struct NestedRowMajor
{
	RowMajor rm;
};

layout(set = 0, binding = 0, row_major) uniform UBO
{
	mat4 A;
	layout(column_major) mat4 C; // This should also be worked around.
};


layout(set = 0, binding = 1, row_major) uniform UBO2
{
	RowMajor rm;
};

layout(set = 0, binding = 2, row_major) uniform UBO3
{
	NestedRowMajor rm2;
};

layout(set = 0, binding = 3) uniform UBONoWorkaround
{
	mat4 D;
};

layout(location = 0) in vec4 Clip;
layout(location = 0) out vec4 FragColor;

void main()
{
	NestedRowMajor rm2_loaded = rm2;
	FragColor = rm2_loaded.rm.B * rm.B * A * C * Clip;
	FragColor += D * Clip;
	FragColor += A[1] * Clip;
}
