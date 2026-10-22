#version 450

void write_outblock()
{
	gl_PointSize = 1.0;
    gl_Position = vec4(gl_PointSize);
}

void main()
{
    write_outblock();
}
