#version 300 es

in mediump vec3 vNormal;
in mediump vec3 vLightDirection;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float lightdist = length(vLightDirection);
	mediump float inv_lightdist = 1.0 / (lightdist * lightdist) ;
	mediump float diffuse = max(dot(normalize(vNormal), vLightDirection), 0.0);
	
	mediump float color = diffuse * inv_lightdist;

#ifndef FRAMEBUFFER_SRGB
	color = pow(color, 0.4545454545);
#endif	

	oColor = vec4(vec3(color), 1.0);
}
