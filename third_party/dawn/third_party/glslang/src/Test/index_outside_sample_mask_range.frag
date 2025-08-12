#version 320 es 
layout (location = 0) out mediump vec4 fs_color; 
void main() 
{ 
	const highp int invalidIndex = (gl_MaxSamples + 31) / 32; 
	highp int invalidValue = gl_SampleMask[invalidIndex]; 
	fs_color = vec4(1.0f, 0.0f, 0.0f, 1.0f); 
} 
