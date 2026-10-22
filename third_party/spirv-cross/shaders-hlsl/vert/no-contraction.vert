#version 450

layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;
layout(location = 2) in vec4 vC;

void main()
{
	precise vec4 mul = vA * vB;
	precise vec4 add = vA + vB;
	precise vec4 sub = vA - vB;
	precise vec4 mad = vA * vB + vC;
	precise vec4 summed = mul + add + sub + mad;
	gl_Position = summed;
}
