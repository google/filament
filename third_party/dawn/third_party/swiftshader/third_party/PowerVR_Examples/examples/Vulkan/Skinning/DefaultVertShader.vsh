#version 320 es

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;

// perframe / per mesh
layout(std140, set = 1, binding = 0) uniform Dynamics
{
    highp mat4 ModelMatrix;
    highp mat3 ModelWorldIT3x3;
};

// static
layout(std140, set = 2, binding = 0) uniform Statics
{
	highp mat4 VPMatrix;
	highp vec3 LightPos;
};

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out mediump vec3 vWorldNormal;
layout(location = 2) out mediump vec3 vLightDir;
layout(location = 3) out mediump float vOneOverAttenuation;

void main()
{
	gl_Position = VPMatrix * ModelMatrix * vec4(inVertex, 1.0);

	highp vec3 worldPos = (ModelMatrix * vec4(inVertex, 1.0)).xyz;

	vLightDir = LightPos - worldPos;
	mediump float light_distance = length(vLightDir);
	vLightDir /= light_distance;

	vOneOverAttenuation = 1.0 / (1.0 + 0.00005 * light_distance * light_distance);

	vWorldNormal = ModelWorldIT3x3 * inNormal;
	// Pass through texcoords
	vTexCoord = inTexCoord;
}
 
