#version 450
#extension GL_ARB_shader_stencil_export : require

layout(location = 0) out vec4 buf0;
layout(location = 1) out vec4 buf1;
layout(location = 2) out vec4 buf2;
layout(location = 3) out vec4 buf3;
layout(location = 4) out vec4 buf4;
layout(location = 5) out vec4 buf5;
layout(location = 6) out vec4 buf6;
layout(location = 7) out vec4 buf7;

void main() {
	buf0 = vec4(0, 0, 0, 1);
	buf1 = vec4(1, 0, 0, 1);
	buf2 = vec4(0, 1, 0, 1);
	buf3 = vec4(0, 0, 1, 1);
	buf4 = vec4(1, 0, 1, 0.5);
	buf5 = vec4(0.25, 0.25, 0.25, 0.25);
	buf6 = vec4(0.75, 0.75, 0.75, 0.75);
	buf7 = vec4(1, 1, 1, 1);
	gl_FragDepth = 0.9;
	gl_FragStencilRefARB = 127;
}

