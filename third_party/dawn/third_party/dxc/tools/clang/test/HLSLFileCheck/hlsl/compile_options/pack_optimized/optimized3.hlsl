// RUN: %dxilver 1.5 | %dxc -E main -T vs_6_0 -pack_optimized %s | FileCheck %s

// CHECK:      ; Output signature:
// CHECK:      ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK-NEXT: ; -------------------- ----- ------ -------- -------- ------- ------
// CHECK-NEXT: ; First                    0   xyz         0     NONE   float   xyz
// CHECK-NEXT: ; WithFirst                0      w        0     NONE   float      w
// CHECK-NEXT: ; SV_CullDistance          0      w        1  CULLDST   float      w
// CHECK-NEXT: ; SV_CullDistance          1     z         1  CULLDST   float     z
// CHECK-NEXT: ; SV_ClipDistance          1    y          1  CLIPDST   float    y
// CHECK-NEXT: ; BeforeClipCull           0   x           1     NONE   float   x
// CHECK-NEXT: ; SV_CullDistance          2     z         2  CULLDST   float     z
// CHECK-NEXT: ; SV_ClipDistance          0   x           2  CLIPDST   float   x
// CHECK-NEXT: ; SV_ClipDistance          2    y          2  CLIPDST   float    y
// CHECK-NEXT: ; SV_ClipDistance          3      w        2  CLIPDST   float      w

struct VS_OUT {
  float3 first : First;
  float cull0 : SV_CullDistance0;
  float clip0 : SV_ClipDistance0;
  float clip1[2] : SV_ClipDistance1;
  float cull1[2] : SV_CullDistance1;
  float clip3 : SV_ClipDistance3;
  float withFirst : WithFirst;          // packs with First
  float afterClipCull : BeforeClipCull; // packed before clip/cull in same row
};


VS_OUT main() {
	return (VS_OUT)1.0F;
}