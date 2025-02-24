#version 300 es
in highp vec3 inVertex;
in mediump vec2 inHighResTexCoord;
in mediump vec2 inLowResTexCoord;
out mediump vec2 HighResTexCoord;
out mediump vec2 LowResTexCoord;
void main()
{
	// Transform position
	gl_Position = vec4(inVertex, 1.0);
	// Pass through texcoords
	HighResTexCoord = inHighResTexCoord;
    LowResTexCoord = inLowResTexCoord;
}