#version 450
#extension GL_AMD_gpu_shader_half_float : require

layout(location = 3) in f16vec4 v4;

void main()
{
    f16vec4 _505;
    f16vec4 _577 = modf(v4, _505);
}

