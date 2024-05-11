#version 310 es
precision mediump float;

layout(location = 0) in vec4 PLSIn0;
layout(location = 1) in vec4 PLSIn1;
layout(location = 2) in vec4 PLSIn2;
layout(location = 3) in vec4 PLSIn3;

layout(location = 0) out vec4 PLSOut0;
layout(location = 1) out vec4 PLSOut1;
layout(location = 2) out vec4 PLSOut2;
layout(location = 3) out vec4 PLSOut3;

void main()
{
    PLSOut0 = 2.0 * PLSIn0;
    PLSOut1 = 6.0 * PLSIn1;
    PLSOut2 = 7.0 * PLSIn2;
    PLSOut3 = 4.0 * PLSIn3;
}
