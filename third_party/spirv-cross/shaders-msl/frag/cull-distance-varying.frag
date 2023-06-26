#version 450

in float gl_CullDistance[2];

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0 - gl_CullDistance[0] - gl_CullDistance[1]);
}
