attribute highp vec3 myVertex;

uniform highp mat4 transform;
uniform highp mat4 shadowMatrix;

void main(void)
{
	highp vec4 worldPos = shadowMatrix * vec4(myVertex, 1.0);
	worldPos.y += 0.0001;
	gl_Position = transform * worldPos;
}