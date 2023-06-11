#version 450 core
#extension GL_EXT_device_group : require
#extension GL_EXT_multiview : require

void main()
{
	gl_Position = vec4(gl_DeviceIndex, gl_ViewIndex, 0.0, 1.0);
}
