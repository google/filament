#version 450

layout(location = 0, component = 0) out float FragColor0;
layout(location = 0, component = 1) out vec2 FragColor1;

void main()
{
	FragColor0 = 1.0;
	FragColor1 = vec2(2.0, 3.0);
}
