#version 450

struct RowMajor
{
    mat4 B;
};

struct NestedRowMajor
{
    RowMajor rm;
};

layout(binding = 2, std140) uniform UBO3
{
    layout(row_major) NestedRowMajor rm2;
} _17;

layout(binding = 1, std140) uniform UBO2
{
    layout(row_major) RowMajor rm;
} _35;

layout(binding = 0, std140) uniform UBO
{
    layout(row_major) mat4 A;
    mat4 C;
} _42;

layout(binding = 3, std140) uniform UBONoWorkaround
{
    mat4 D;
} _56;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 Clip;

NestedRowMajor SPIRV_Cross_workaround_load_row_major(NestedRowMajor wrap) { return wrap; }
mat4 SPIRV_Cross_workaround_load_row_major(mat4 wrap) { return wrap; }

void main()
{
    FragColor = (((SPIRV_Cross_workaround_load_row_major(_17.rm2).rm.B * SPIRV_Cross_workaround_load_row_major(_35.rm.B)) * SPIRV_Cross_workaround_load_row_major(_42.A)) * SPIRV_Cross_workaround_load_row_major(_42.C)) * Clip;
    FragColor += (_56.D * Clip);
    FragColor += (_42.A[1] * Clip);
}

