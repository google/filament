#version 320 es

const int COLOR_ARRAY_SIZE = 11;

layout(location = 0) in highp vec3 myVertex;
layout(location = 2) in mediump vec3 normal;

layout(std140, set = 0, binding = 0) uniform DynamicData
{
	uniform highp mat4 transform;
	uniform highp mat4 viewMatrix;
	uniform mediump vec3 lightDir;
};


layout(std140, set = 1, binding = 0) uniform StaticData
{
	uniform mat4 shadowMatrix;
};

layout (push_constant) uniform push_constant
{
    mediump vec4 myColor;
} pushConstant;
layout(location = 0) out mediump vec4 fragColor;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);
	
	mediump vec3 N = normalize(mat3(viewMatrix) * normal);

	mediump float D = max(dot(N, lightDir), 0.15);
	fragColor = vec4(pushConstant.myColor.rgb * D, pushConstant.myColor.a);
}
