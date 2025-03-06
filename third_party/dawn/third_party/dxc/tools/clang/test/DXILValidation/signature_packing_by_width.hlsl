// TODO: Update this file when we introduce i8

// Checking if signature elements are packed by interpolation mode and data width with up to 4 elements.

// CHECK: ; Name                 Index   Mask Register SysValue  Format   Used
// CHECK: ; -------------------- ----- ------ -------- -------- ------- ------
// CHECK: ; A                        0   xy          0     NONE    fp16
// CHECK: ; F                        0     zw        0     NONE    fp16
// CHECK: ; B                        0   xy          1     NONE   float
// CHECK: ; D                        0     zw        1     NONE   float
// CHECK: ; C                        0   xyz         2     NONE    fp16
// CHECK: ; G                        0      w        2     NONE    fp16
// CHECK: ; SV_PrimitiveID           0   x           3   PRIMID    uint
// CHECK: ; E                        0   x           4     NONE     int
// CHECK: ; I                        0    y          4     NONE     int
// CHECK: ; H                        0   x           5     NONE   int16
// CHECK: ; J                        0    yzw        5     NONE    fp16
// CHECK: ; K                        0   xy          6     NONE   int16
// CHECK: ; N                        0     z         6     NONE    fp16
// CHECK: ; O                        0      w        6     NONE  uint16
// CHECK: ; L                        0   xy          7     NONE    fp16
// CHECK: ; Q                        0     z         7     NONE    fp16
// CHECK: ; P                        0   xy          8     NONE  uint16
// CHECK: ; SV_SampleIndex           0    N/A  special   SAMPLE    uint     NO

// CHECK: !{i32 0, !"A", i8 8, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 2, i32 0, i8 0, null}
// CHECK: !{i32 1, !"B", i8 9, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 2, i32 1, i8 0, null}
// CHECK: !{i32 2, !"C", i8 8, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 3, i32 2, i8 0, null}
// CHECK: !{i32 3, !"SV_PrimitiveID", i8 5, i8 10, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 3, i8 0, null}
// CHECK: !{i32 4, !"D", i8 9, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 2, i32 1, i8 2, null}
// CHECK: !{i32 5, !"E", i8 4, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 4, i8 0, null}
// CHECK: !{i32 6, !"F", i8 8, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 2, i32 0, i8 2, null}
// CHECK: !{i32 7, !"G", i8 8, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 1, i32 2, i8 3, null}
// CHECK: !{i32 8, !"H", i8 2, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 5, i8 0, null}
// CHECK: !{i32 9, !"I", i8 4, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 4, i8 1, null}
// CHECK: !{i32 10, !"J", i8 8, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 3, i32 5, i8 1, null}
// CHECK: !{i32 11, !"K", i8 2, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 2, i32 6, i8 0, null}
// CHECK: !{i32 12, !"L", i8 8, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 2, i32 7, i8 0, null}
// CHECK: !{i32 13, !"N", i8 8, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 6, i8 2, null}
// CHECK: !{i32 14, !"SV_SampleIndex", i8 5, i8 12, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 -1, i8 -1, null}
// CHECK: !{i32 15, !"O", i8 3, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 6, i8 3, null}
// CHECK: !{i32 16, !"P", i8 3, i8 0, !{{[0-9]+}}, i8 1, i32 1, i8 2, i32 8, i8 0, null}
// CHECK: !{i32 17, !"Q", i8 8, i8 0, !{{[0-9]+}}, i8 2, i32 1, i8 1, i32 7, i8 2, null}

float4 main(min16float2 a : A, float2 b : B, half3 c : C, uint id : SV_PrimitiveID,
            float2 d : D, int e : E, half2 f : F, half g : G,
            min16int h : H, int i : I, nointerpolation min16float3 j : J,
            min16int2 k : K, half2 l : L, nointerpolation half n : N, uint sample_idx : SV_SampleIndex, uint16_t o : O,
            vector<uint16_t,2> p : P, half q : Q) : SV_Target {
  return 1;
}