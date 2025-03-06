// Verify that we can find an include header in the original directory
#include "inc2.hlsli"

int f1(int g)
{
  return f2(g);
}
