// RUN: %dxr -E main -extract-entry-uniforms -flegacy-macro-expansion %s | FileCheck %s

#define SLOT 0
#define TEXDECL2D(texname, slot) uniform Texture2D texname : register (t##slot)

struct PS_INPUT {
  float4 pos : SV_Position;
};

// CHECK: uniform Texture2D texDiffuse : register(t0);
// CHECK: float4 main(PS_INPUT I)

float4 main(PS_INPUT I,
    TEXDECL2D(texDiffuse, SLOT)
) : SV_Target
{
    return 0;
}
