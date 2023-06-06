#version 310 es

uniform vec4 UBO[14];
layout(location = 0) out vec4 oA;
layout(location = 1) out vec4 oB;
layout(location = 2) out vec4 oC;
layout(location = 3) out vec4 oD;
layout(location = 4) out vec4 oE;

void main()
{
    gl_Position = vec4(0.0);
    oA = UBO[1];
    oB = vec4(UBO[4].y, UBO[5].y, UBO[6].y, UBO[7].y);
    oC = UBO[9];
    oD = vec4(UBO[10].x, UBO[11].x, UBO[12].x, UBO[13].x);
    oE = vec4(UBO[1].z, UBO[6].y, UBO[9].z, UBO[12].y);
}

