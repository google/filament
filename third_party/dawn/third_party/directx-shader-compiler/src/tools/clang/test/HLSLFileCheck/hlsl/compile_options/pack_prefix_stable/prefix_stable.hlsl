// RUN: %dxc -E main -T vs_6_0 -pack_prefix_stable %s | FileCheck %s

// CHECK:      ; Output signature:
// CHECK:      ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK-NEXT: ; -------------------- ----- ------ -------- -------- ------- ------
// CHECK-NEXT: ; First                    0   xyz         0     NONE   float   xyz
// CHECK-NEXT: ; WithFirst                0      w        0     NONE   float      w
// CHECK-NEXT: ; SV_ClipDistance          0   x           1  CLIPDST   float   x
// CHECK-NEXT: ; SV_CullDistance          1    yzw        1  CULLDST   float    yzw
// CHECK-NEXT: ; SV_ClipDistance          1    yz         2  CLIPDST   float    yz
// CHECK-NEXT: ; SV_CullDistance          0   x           2  CULLDST   float   x
// CHECK-NEXT: ; AfterClipCull            0   x           3     NONE   float   x

struct VS_OUT {
  float3 first : First;
  // Clip/Cull reserves row
  float clip0 : SV_ClipDistance0;
  float3 cull1 : SV_CullDistance1;
  float cull0 : SV_CullDistance0;
  float2 clip1 : SV_ClipDistance1;
  float withFirst : WithFirst;          // packs with First
  float afterClipCull : AfterClipCull;  // cannot pack after clip/cull in same row
};


VS_OUT main() {
	return (VS_OUT)1.0F;
}