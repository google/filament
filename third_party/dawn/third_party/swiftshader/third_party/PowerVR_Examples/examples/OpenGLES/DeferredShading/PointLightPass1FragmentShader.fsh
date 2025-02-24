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

void main()
{
	return;
}