#version 320 es

layout(location = 0) out mediump vec4 fragColor;
void main (void)
{
       for (int i = 0; i < gl_SampleMask.length(); ++i)
              gl_SampleMask[i] = int(0xAAAAAAAA);

       fragColor = vec4(0.0, 1.0, 0.0, 1.0);
}