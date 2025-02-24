#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

uniform mediump sampler2D sTexture;

uniform mediump float fFarClipDistance;

layout(std140, binding = 1) uniform StaticPerMaterial
{
	mediump float fSpecularStrength;
	mediump vec4 vDiffuseColor;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in mediump vec3 vNormal;
layout(location = 2) in highp vec3 vViewPosition;

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) mediump vec4 albedo;
	layout(rgb10_a2) mediump vec4 normal;
	layout(r32f) highp float depth;
	layout(r11f_g11f_b10f) mediump vec3 color;
} pls;

void main()
{
	// Calculate the albedo
	// Pack the specular exponent with the albedo
	mediump vec4 albedo = vec4(texture(sTexture, vTexCoord).rgb * vDiffuseColor.rgb, fSpecularStrength);
	// Scale the normal range from [-1,1] to [0, 1] to pack it into the RGB_U8 texture
	mediump vec4 normal = vec4(normalize(vNormal) * 0.5 + 0.5, 1.0);
	highp float depth = vViewPosition.z / fFarClipDistance;

	pls.albedo = albedo;
	pls.normal = normal;
	pls.depth = depth;

#ifndef GL_EXT_shader_pixel_local_storage2
	// clear pixel local storage colour when GL_EXT_shader_pixel_local_storage2 isn't supported
	pls.color = vec3(0.0);
#endif
}