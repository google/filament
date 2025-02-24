attribute highp vec3 myVertex;

uniform highp mat4 transform;
uniform mediump vec4 myColor;

varying mediump vec4 fragColor;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);
	fragColor = myColor;
}