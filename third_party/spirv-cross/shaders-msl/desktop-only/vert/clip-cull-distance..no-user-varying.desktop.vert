#version 450

void main()
{
   gl_Position = vec4(10.0);
   gl_ClipDistance[0] = 1.0;
   gl_ClipDistance[1] = 4.0;
   //gl_CullDistance[0] = 4.0;
   //gl_CullDistance[1] = 9.0;
}
