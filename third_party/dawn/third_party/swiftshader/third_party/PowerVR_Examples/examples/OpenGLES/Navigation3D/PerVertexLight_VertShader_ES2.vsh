attribute highp vec3 myVertex;
attribute mediump vec3 normal;

uniform highp mat4 transform;
uniform highp mat4 viewMatrix;
uniform mediump vec3 lightDir;
uniform mediump vec4 myColor;

varying mediump vec4 fragColor;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);

	mediump vec3 N = normalize(mat3(viewMatrix) * normal);

	mediump float D = max(dot(N, lightDir), 0.15);
	fragColor = vec4(myColor.rgb * D, myColor.a);
}