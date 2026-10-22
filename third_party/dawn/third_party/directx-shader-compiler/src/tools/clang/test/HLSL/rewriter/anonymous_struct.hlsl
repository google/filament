
SamplerState ss;

static const struct {
  float a;
  SamplerState s;
} A = {1.2, ss};

float4 main() : SV_Target {
    return A.a;
}