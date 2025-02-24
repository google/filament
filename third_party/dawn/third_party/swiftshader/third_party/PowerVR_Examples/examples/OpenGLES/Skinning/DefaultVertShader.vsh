#version 310 es

in highp vec3 inVertex;
in mediump vec3 inNormal;
in mediump vec2	inTexCoord;

uniform highp mat4x3 ModelMatrix;
uniform highp mat4 MVPMatrix;
uniform highp mat3 ModelWorldIT3x3;
uniform highp vec3 LightPos;

out mediump vec2 vTexCoord;
out mediump vec3 vWorldNormal;
out mediump vec3 vLightDir;
out mediump float vOneOverAttenuation;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	highp vec3 worldPos = ModelMatrix * vec4(inVertex, 1.0);

	vLightDir = LightPos - worldPos;
	mediump float light_distance = length(vLightDir);
	vLightDir /= light_distance;

	vOneOverAttenuation = 1.0 / (1.0 + 0.00005 * light_distance * light_distance);

	vWorldNormal = ModelWorldIT3x3 * inNormal;
	// Pass through texcoords
	vTexCoord = inTexCoord;
}
