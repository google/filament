// RUN: %dxc -E main -T vs_6_0 -pack_optimized %s | FileCheck %s

// CHECK:      ; Output signature:
// CHECK:      ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK-NEXT: ; -------------------- ----- ------ -------- -------- ------- ------
// CHECK-NEXT: ; First                    0   xyz         0     NONE   float   xyz
// CHECK-NEXT: ; WithFirst                0      w        0     NONE   float      w
// CHECK-NEXT: ; SV_ClipDistance          1    yz         1  CLIPDST   float    yz
// CHECK-NEXT: ; SV_CullDistance          0      w        1  CULLDST   float      w
// CHECK-NEXT: ; BeforeClipCull           0   x           1     NONE   float   x
// CHECK-NEXT: ; SV_ClipDistance          0      w        2  CLIPDST   float      w
// CHECK-NEXT: ; SV_CullDistance          1   xyz         2  CULLDST   float   xyz

struct VS_OUT {
  float3 first : First;
  float clip0 : SV_ClipDistance0;
  float3 cull1 : SV_CullDistance1;
  float cull0 : SV_CullDistance0;
  float2 clip1 : SV_ClipDistance1;
  float withFirst : WithFirst;          // packs with First
  float afterClipCull : BeforeClipCull; // packed before clip/cull in same row
};


VS_OUT main() {
	return (VS_OUT)1.0F;
}