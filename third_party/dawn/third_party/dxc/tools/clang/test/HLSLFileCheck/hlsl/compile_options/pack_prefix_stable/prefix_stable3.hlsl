// RUN: %dxilver 1.5 | %dxc -E main -T vs_6_0 -pack_prefix_stable %s | FileCheck %s

// CHECK:      ; Output signature:
// CHECK:      ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK-NEXT: ; -------------------- ----- ------ -------- -------- ------- ------
// CHECK-NEXT: ; First                    0   xyz         0     NONE   float   xyz
// CHECK-NEXT: ; WithFirst                0      w        0     NONE   float      w
// CHECK-NEXT: ; SV_ClipDistance          0   x           1  CLIPDST   float   x
// CHECK-NEXT: ; SV_ClipDistance          1      w        1  CLIPDST   float      w
// CHECK-NEXT: ; SV_CullDistance          1    yz         1  CULLDST   float    yz
// CHECK-NEXT: ; SV_ClipDistance          2      w        2  CLIPDST   float      w
// CHECK-NEXT: ; SV_CullDistance          2    yz         2  CULLDST   float    yz
// CHECK-NEXT: ; AfterClipCull            0   x           3     NONE   float   x

struct VS_OUT {
  float3 first : First;
  // Clip/Cull reserves row
  float clip0 : SV_ClipDistance0;
  float2 cull1[2] : SV_CullDistance1;   // array works
  float clip1[2] : SV_ClipDistance1;    // second array works without conflict
  float withFirst : WithFirst;          // packs with First
  // AfterClipCull looks like it could pack before clip/cull elements on row 2,
  // but prefix-stable reserves rows for clip/cull, so it will not pack there.
  float afterClipCull : AfterClipCull;
};


VS_OUT main() {
	return (VS_OUT)1.0F;
}