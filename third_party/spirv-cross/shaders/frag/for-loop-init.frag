#version 310 es
precision mediump float;
layout(location = 0) out int FragColor;

void main()
{
   FragColor = 16;

   // Basic loop variable.
   for (int i = 0; i < 25; i++)
      FragColor += 10;

   // Multiple loop variables.
   for (int i = 1, j = 4; i < 30; i++, j += 4)
      FragColor += 11;

   // A potential loop variables, but we access it outside the loop,
   // so cannot be one.
   int k = 0;
   for (; k < 20; k++)
      FragColor += 12;
   k += 3;
   FragColor += k;

   // Potential loop variables, but the dominator is not trivial.
   int l;
   if (k == 40)
   {
      for (l = 0; l < 40; l++)
         FragColor += 13;
      return;
   }
   else
   {
      l = k;
      FragColor += l;
   }

   // Vectors cannot be loop variables
   for (ivec2 i = ivec2(0); i.x < 10; i.x += 4)
   {
      FragColor += i.y;
   }

   // Check that static expressions can be used before the loop header.
   int m = 0;
   m = k;
   int o = m;
   for (; m < 40; m++)
      FragColor += m;
   FragColor += o;
}
