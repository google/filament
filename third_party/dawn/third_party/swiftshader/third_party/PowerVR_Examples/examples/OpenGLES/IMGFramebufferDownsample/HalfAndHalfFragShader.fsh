#version 310 es

uniform highp sampler2D fullDimensionColor;
uniform highp sampler2D halfDimensionColor;

uniform highp float WindowWidth;

in mediump vec2 TexCoord;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	highp float imageCoordX = gl_FragCoord.x - 0.5;
	highp float xPosition = imageCoordX / WindowWidth;

	oColor = xPosition < 0.5 ? textureLod(fullDimensionColor, TexCoord, 1.0) : (xPosition > 0.497 && xPosition < 0.503) ? vec4(1.0, 1.0, 1.0, 1.0) : textureLod(halfDimensionColor, TexCoord, 1.0);
}