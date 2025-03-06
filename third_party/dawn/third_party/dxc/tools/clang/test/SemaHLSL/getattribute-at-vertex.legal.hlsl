// RUN: %dxc -T ps_6_2 -E main %s -verify

// expected-no-diagnostics
struct S1 {
  nointerpolation float3 f1;
};

struct S2 {
  float3 f1;
};

struct S3 {
  float3 f1[3];
};

struct S4 {
  S1 f1;
};

struct S5 {
  S1 f1[2];
};

float compute(float3 value) {
  return GetAttributeAtVertex(value, 0)[0];
}

float4 main(nointerpolation float3 a : A,
            S1 s1 : B,
            nointerpolation S2 s2 : C,
            nointerpolation S3 s3 : D,
            S4 s4 : E,
            S5 s5 : F
            ) : SV_Target
{
  float v1 = GetAttributeAtVertex(a, 0)[0];
  float v2 = GetAttributeAtVertex(a.x, 0);
  float v3 = GetAttributeAtVertex(s1.f1, 0)[0];
  float v4 = GetAttributeAtVertex(s2.f1, 0)[0];
  float v5 = GetAttributeAtVertex(s3.f1[1], 0)[0];
  float v6 = GetAttributeAtVertex(s4.f1.f1, 0)[0];
  float v7 = GetAttributeAtVertex(s5.f1[1].f1, 0)[0];
  float v8 = compute(s1.f1);

  return float4(v1, v2, v3, v4) + float4(v5, v6, v7, v8);
}
