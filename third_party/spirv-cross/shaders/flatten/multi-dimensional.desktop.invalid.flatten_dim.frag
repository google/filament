#version 450

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D uTextures[2][3][1];
layout(location = 0) flat in int vIndex;
layout(location = 1) in vec2 vUV;

void main()
{
   vec4 values3[2][3][1];

   for (int z = 0; z < 2; z++)
      for (int y = 0; y < 3; y++)
         for (int x = 0; x < 1; x++)
            values3[z][y][x] = texture(uTextures[z][y][x], vUV);

   FragColor = values3[1][2][0] + values3[0][2][0] + values3[vIndex + 1][2][vIndex];
}
