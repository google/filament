#version 460

out gl_PerVertex
{
    vec4 gl_Position;
};

void in_func()
{
	gl_Position.w = float(gl_DrawID);
}

void main()
{
    gl_Position = vec4(gl_BaseVertex, gl_BaseInstance, gl_DrawID, 1);
}
