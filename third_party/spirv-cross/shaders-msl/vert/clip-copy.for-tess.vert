#version 450

out float gl_ClipDistance[4];

layout(location = 0) out float F_array[4];

layout(location = 4) out Block
{
	float block0[4];
};

void in_func()
{
	gl_Position = vec4(1, 2, 3, 4);
	const float clips[4] = float[](1.0, 2.0, -1.0, -2.0);
	float non_const_clips[4];
	gl_ClipDistance = clips;
	non_const_clips = gl_ClipDistance;
	gl_ClipDistance = non_const_clips;
	F_array = non_const_clips;
	block0 = clips;
}

void main()
{
	in_func();
}
