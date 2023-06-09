#version 450

layout(location = 0) in vec4 Position;
out gl_PerVertex
{
	vec4 gl_Position;
	float gl_ClipDistance[2];
};

void main()
{
    gl_Position = Position;
    gl_ClipDistance[0] = Position.x;
    gl_ClipDistance[1] = Position.y;
}
