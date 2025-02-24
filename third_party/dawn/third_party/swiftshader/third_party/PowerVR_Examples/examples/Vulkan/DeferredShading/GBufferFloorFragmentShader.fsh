#version 320 es

layout(set = 1, binding = 2) uniform mediump sampler2D sTexture;

layout(set = 0, binding = 0) uniform StaticPerScene
{
	mediump float fFarClipDistance;
};

layout(set = 1, binding = 0) uniform StaticPerMaterial
{	
	mediump float fSpecularStrength;
	mediump vec4 vDiffuseColor;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in mediump vec3 vNormal;
layout(location = 2) in highp vec3 vViewPosition;

layout(location = 0) out mediump vec4 oAlbedo;
layout(location = 1) out mediump vec3 oNormal;
layout(location = 2) out highp float oDepth;

void main()
{
	// Calculate the albedo
	mediump vec3 albedo = texture(sTexture, vTexCoord).rgb * vDiffuseColor.rgb;
	// Pack the specular exponent with the albedo
	oAlbedo = vec4(albedo, fSpecularStrength);
	
	oNormal = normalize(vNormal);
	
	// Negate and divide through by the far clip distance to bring the depth into the [0-1] range
	oDepth = -(vViewPosition.z / fFarClipDistance);
}
