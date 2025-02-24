#version 320 es

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoords;

layout(std140, set = 0, binding = 0) uniform Dynamic
{
	highp mat4 MVMatrix;
	highp vec4 LightDir;
	highp vec4 EyePos;
	mediump float cameraNear;
	mediump float cameraFar;
};

layout(location = 0) out mediump vec2 TexCoords;
layout(location = 1) out mediump float LightIntensity;

void main()
{
	// Transform position to the paraboloid's view space
	gl_Position = MVMatrix * vec4(inVertex, 1.0);

	// Store the distance
	mediump float zDistance = -gl_Position.z;

	// Calculate and set the X and Y coordinates
	gl_Position.xyz = normalize(gl_Position.xyz);
	gl_Position.xy /= 1.0 - gl_Position.z;

	gl_Position.y = -gl_Position.y;

	// Calculate and set the Z and W coordinates
	gl_Position.z = (((zDistance - cameraNear) / (cameraFar - cameraNear)) - 0.5) * 2.0;
	gl_Position.w = 1.0;
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;

	// Pass through texture coordinates
	TexCoords = inTexCoords;

	// Calculate light intensity
	// Ambient
	LightIntensity = 0.4;

	LightIntensity += max(dot(inNormal, vec3(LightDir)), 0.0) * 0.3;
	// Specular
	mediump vec3 EyeDir = normalize(vec3(EyePos) - inVertex);
	LightIntensity += pow(max(dot(reflect(-vec3(LightDir), inNormal), EyeDir), 0.0), 5.0) * 0.8;
}
