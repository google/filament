groupshared uint g_Data[32];
groupshared uint g_Data2[32];

float t;
[numthreads(64, 1, 1)]
void main(uint idx : SV_DispatchThreadId) {
  uint orig;
  if (t > 1)
  InterlockedAdd(g_Data[idx], 1, orig);
  else
  InterlockedAdd(g_Data2[idx], 1, orig);
}
