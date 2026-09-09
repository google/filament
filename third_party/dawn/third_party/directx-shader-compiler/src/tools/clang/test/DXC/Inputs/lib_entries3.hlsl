// Make sure include works for lib share compile.
#include "lib_inc.hlsl"

// PS
[shader("pixel")]
float4 ps_main() : SV_TARGET
{
  return a + b + c + d + e + M;
}
