#version 450

struct type_PushConstant_Matrix
{
    mat4 transform;
};

uniform type_PushConstant_Matrix matrix_constants;

layout(location = 0) in vec4 in_var_POSITION;

void main()
{
    gl_Position = matrix_constants.transform * in_var_POSITION;
}

