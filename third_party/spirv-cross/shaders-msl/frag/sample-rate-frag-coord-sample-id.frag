#version 450

layout(set = 0, binding = 0) uniform sampler2DArray tex;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(tex, vec3(gl_FragCoord.xy, float(gl_SampleID)));
}
