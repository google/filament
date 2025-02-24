#version 320 es

layout(location = 0) in mediump vec3 vNormal;
layout(location = 1) in mediump vec3 vLightDirection;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float inv_lightdist = 1.0 / length(vLightDirection);
	mediump float diffuse = max(dot(normalize(vNormal), vLightDirection * inv_lightdist), 0.0);
	
	oColor = vec4(vec3(diffuse) * inv_lightdist * 10.0, 1.0);
}