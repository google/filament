#version 320 es

#define POSITION_ARRAY	0
#define LIFESPAN_ARRAY	1

layout(location = POSITION_ARRAY) in highp vec3 inPosition;
layout(location = LIFESPAN_ARRAY) in mediump float inLifespan;

layout (std140, set = 0, binding = 0) uniform DynamicModel
{
	highp mat4 uModelViewProjectionMatrix;
};

layout(location = 0) out mediump float lifespan;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.0);
	gl_PointSize = 1.5;
	lifespan = inLifespan;
}
