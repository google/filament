#version 300 es

#define POSITION_ARRAY	0
#define LIFESPAN_ARRAY	1

layout(location = POSITION_ARRAY) in highp vec3 inPosition;
layout(location = LIFESPAN_ARRAY) in mediump float inLifespan;

uniform highp mat4 uModelViewProjectionMatrix;

out mediump float lifespan;

const mediump float particleFullSize = 1.f;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inPosition, 1.0);
	lifespan = inLifespan;
	gl_PointSize = particleFullSize * 1000. * (1. - gl_Position.z / gl_Position.w);
}
