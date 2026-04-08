#version 450

layout(location = 5) out BlockIO
{
	float v;
} outBlock;

void func()
{
	outBlock.v = 1.0;
}

void main(void)
{
	func();
}
