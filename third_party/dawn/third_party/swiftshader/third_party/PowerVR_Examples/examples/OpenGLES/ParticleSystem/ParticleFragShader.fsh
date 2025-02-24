#version 300 es

in mediump float lifespan;
layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float f = length((gl_PointCoord - vec2(0.5,0.5)) * 2.);
	mediump vec4 texcolor = vec4(1.,1.,1., 1. - f);

	oColor = vec4(texcolor.rgb * vec3(1, lifespan*.5 + .5, lifespan), texcolor.a);
}
