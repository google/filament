// RUN: %dxc -E main -T ps_6_0 -rootsig-define RS %s
// Test root signature define empty string

#define RS ""

float main(float i : I) : SV_Target
{
  return i;
}

