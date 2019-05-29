#version 450
layout(location = 0) in float v;
layout(location = 0) out float FragColor;

void set_output_depth()
{
	gl_FragDepth = 0.2;
}

void main()
{
	FragColor = 1.0;
	set_output_depth();
}
