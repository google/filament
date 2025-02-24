float4 tint_degrees(float4 param_0) {
  return param_0 * 57.29577951308232286465;
}

float3 tint_degrees_1(float3 param_0) {
  return param_0 * 57.29577951308232286465;
}

float2 tint_degrees_2(float2 param_0) {
  return param_0 * 57.29577951308232286465;
}

float tint_degrees_3(float param_0) {
  return param_0 * 57.29577951308232286465;
}

[numthreads(1, 1, 1)]
void main() {
  float4 va = (0.0f).xxxx;
  float4 a = tint_degrees(va);
  float4 vb = (1.0f).xxxx;
  float4 b = tint_degrees(vb);
  float4 vc = float4(1.0f, 2.0f, 3.0f, 4.0f);
  float4 c = tint_degrees(vc);
  float3 vd = (0.0f).xxx;
  float3 d = tint_degrees_1(vd);
  float3 ve = (1.0f).xxx;
  float3 e = tint_degrees_1(ve);
  float3 vf = float3(1.0f, 2.0f, 3.0f);
  float3 f = tint_degrees_1(vf);
  float2 vg = (0.0f).xx;
  float2 g = tint_degrees_2(vg);
  float2 vh = (1.0f).xx;
  float2 h = tint_degrees_2(vh);
  float2 vi = float2(1.0f, 2.0f);
  float2 i = tint_degrees_2(vi);
  float vj = 1.0f;
  float j = tint_degrees_3(vj);
  float vk = 2.0f;
  float k = tint_degrees_3(vk);
  float vl = 3.0f;
  float l = tint_degrees_3(vl);
  return;
}
