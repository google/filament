#version 450

in float gl_ClipDistance[2];

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0 - gl_ClipDistance[0] - gl_ClipDistance[1]);
}
