#version 320 es

layout(set = 1, binding = 2) uniform mediump sampler2D sTexture;
layout(set = 1, binding = 3) uniform mediump sampler2D sBumpMap;

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
layout(location = 2) in highp vec3 vTangent;
layout(location = 3) in highp vec3 vBinormal;
layout(location = 4) in highp vec3 vViewPosition;

layout(location = 0) out mediump vec4 oAlbedo;
layout(location = 1) out mediump vec3 oNormal;
layout(location = 2) out highp float oDepth;

void main()
{
	// Calculate the albedo, Pack the specular exponent with the albedo
	oAlbedo = vec4(texture(sTexture, vTexCoord).rgb * vDiffuseColor.rgb, fSpecularStrength);

	// Calculate view space perturbed normal
	mediump vec3 bumpmap = normalize(texture(sBumpMap, vTexCoord).rgb * 2.0 - 1.0);
	highp mat3 tangentSpace = mat3(normalize(vTangent), normalize(vBinormal), normalize(vNormal));	
	oNormal = normalize(tangentSpace * bumpmap);
	
	oDepth = vViewPosition.z / fFarClipDistance;
}
