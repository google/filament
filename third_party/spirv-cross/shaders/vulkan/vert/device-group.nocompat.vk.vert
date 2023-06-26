#version 450 core
#extension GL_EXT_device_group : require

void main()
{
	gl_Position = vec4(gl_DeviceIndex);
}
