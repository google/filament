#version 310 es
#extension GL_EXT_shader_pixel_local_storage2 : enable

#ifndef GL_EXT_shader_pixel_local_storage2
#extension GL_EXT_shader_pixel_local_storage : require
#endif

layout(rgba8)  __pixel_localEXT FragDataLocal {
	layout(rgba8) mediump vec4 albedo;
	layout(rgb10_a2) mediump vec4 normal;
	layout(r32f) highp float depth;
	layout(r11f_g11f_b10f) mediump vec3 color;
} pls;

#ifndef GL_EXT_shader_pixel_local_storage2
layout(location = 0) out mediump vec4 oColorFbo;
#else
layout(location = 0, rgba8) out mediump vec4 oColorFbo;
#endif

void main()
{
	mediump vec3 outColor = pls.color.rgb;
#ifndef FRAMEBUFFER_SRGB
#ifdef SIMPLE_GAMMA_FUNCTION
	outColor = pow(outColor, vec3(0.454545));
#else
	outColor = mix(outColor * 12.92, 1.055 * pow(outColor, vec3(0.416)) - 0.055 , vec3(outColor.x > 0.00313, outColor.y > 0.00313, outColor.z > 0.00313));
#endif
#endif
	oColorFbo = vec4(outColor, 1.0);
}
