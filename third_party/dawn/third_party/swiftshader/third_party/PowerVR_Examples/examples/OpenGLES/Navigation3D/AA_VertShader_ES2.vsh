attribute highp vec3 myVertex;
attribute mediump vec2 texCoord;

uniform highp mat4 transform;
uniform mediump vec4 myColor;

varying mediump vec4 fragColor;
varying mediump vec2 texCoordOut;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);
	fragColor = myColor;
	texCoordOut = texCoord;
}