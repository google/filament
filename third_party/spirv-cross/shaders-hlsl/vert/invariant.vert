#version 310 es

invariant gl_Position;
layout(location = 0) invariant out vec4 vColor;
layout(location = 0) in vec4 vInput0;
layout(location = 1) in vec4 vInput1;
layout(location = 2) in vec4 vInput2;

void main()
{
	gl_Position = vInput0 + vInput1 * vInput2;
	vColor = (vInput0 - vInput1) * vInput2;
}
