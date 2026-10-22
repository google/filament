#version 450
layout(location = 0) out float FragColors[2];
layout(location = 2) out vec2 FragColor2;
layout(location = 3) out vec3 FragColor3;
layout(location = 0) in vec3 vColor;

void set_globals()
{
	FragColors[0] = vColor.x;
	FragColors[1] = vColor.y;
	FragColor2 = vColor.xz;
	FragColor3 = vColor.zzz;
}

void main()
{
	set_globals();
}
