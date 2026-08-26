; RUN: %dxa %s -o %t
; RUN: %dxa %t -dumpreflection | FileCheck %s

; CHECK: ID3D12ShaderReflectionConstantBuffer:
; CHECK-NEXT:        D3D12_SHADER_BUFFER_DESC: Name: s
; CHECK-NEXT:          Type: D3D_CT_CBUFFER
; CHECK-NEXT:          Size: 288
; CHECK-NEXT:          uFlags: 0
; CHECK-NEXT:          Num Variables: 1
; CHECK-NEXT:        {
; CHECK-NEXT:          ID3D12ShaderReflectionVariable:
; CHECK-NEXT:            D3D12_SHADER_VARIABLE_DESC: Name: s
; CHECK-NEXT:              Size: 276
; CHECK-NEXT:              StartOffset: 0
; CHECK-NEXT:              uFlags: (D3D_SVF_USED)
; CHECK-NEXT:              DefaultValue: <nullptr>
; CHECK-NEXT:            ID3D12ShaderReflectionType:
; CHECK-NEXT:              D3D12_SHADER_TYPE_DESC: Name: S
; CHECK-NEXT:                Class: D3D_SVC_STRUCT
; CHECK-NEXT:                Type: D3D_SVT_VOID
; CHECK-NEXT:                Elements: 0
; CHECK-NEXT:                Rows: 1
; CHECK-NEXT:                Columns: 42
; CHECK-NEXT:                Members: 6
; CHECK-NEXT:                Offset: 0
; CHECK-NEXT:              {
; CHECK-NEXT:                ID3D12ShaderReflectionType:
; CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int4x3
; CHECK-NEXT:                    Class: D3D_SVC_MATRIX_ROWS
; CHECK-NEXT:                    Type: D3D_SVT_INT
; CHECK-NEXT:                    Elements: 0
; CHECK-NEXT:                    Rows: 4
; CHECK-NEXT:                    Columns: 3
; CHECK-NEXT:                    Members: 0
; CHECK-NEXT:                    Offset: 0
; CHECK-NEXT:                ID3D12ShaderReflectionType:
; CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int1x3
; CHECK-NEXT:                    Class: D3D_SVC_MATRIX_COLUMNS
; CHECK-NEXT:                    Type: D3D_SVT_INT
; CHECK-NEXT:                    Elements: 2
; CHECK-NEXT:                    Rows: 1
; CHECK-NEXT:                    Columns: 3
; CHECK-NEXT:                    Members: 0
; CHECK-NEXT:                    Offset: 64
; CHECK-NEXT:                ID3D12ShaderReflectionType:
; CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int3x1
; CHECK-NEXT:                    Class: D3D_SVC_MATRIX_COLUMNS
; CHECK-NEXT:                    Type: D3D_SVT_INT
; CHECK-NEXT:                    Elements: 0
; CHECK-NEXT:                    Rows: 3
; CHECK-NEXT:                    Columns: 1
; CHECK-NEXT:                    Members: 0
; CHECK-NEXT:                    Offset: 148
; CHECK-NEXT:                ID3D12ShaderReflectionType:
; CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int4x3
; CHECK-NEXT:                    Class: D3D_SVC_MATRIX_COLUMNS
; CHECK-NEXT:                    Type: D3D_SVT_INT
; CHECK-NEXT:                    Elements: 0
; CHECK-NEXT:                    Rows: 4
; CHECK-NEXT:                    Columns: 3
; CHECK-NEXT:                    Members: 0
; CHECK-NEXT:                    Offset: 160
; CHECK-NEXT:                ID3D12ShaderReflectionType:
; CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int1x3
; CHECK-NEXT:                    Class: D3D_SVC_MATRIX_ROWS
; CHECK-NEXT:                    Type: D3D_SVT_INT
; CHECK-NEXT:                    Elements: 2
; CHECK-NEXT:                    Rows: 1
; CHECK-NEXT:                    Columns: 3
; CHECK-NEXT:                    Members: 0
; CHECK-NEXT:                    Offset: 208
; CHECK-NEXT:                ID3D12ShaderReflectionType:
; CHECK-NEXT:                  D3D12_SHADER_TYPE_DESC: Name: int3x1
; CHECK-NEXT:                    Class: D3D_SVC_MATRIX_ROWS
; CHECK-NEXT:                    Type: D3D_SVT_INT
; CHECK-NEXT:                    Elements: 0
; CHECK-NEXT:                    Rows: 3
; CHECK-NEXT:                    Columns: 1
; CHECK-NEXT:                    Members: 0
; CHECK-NEXT:                    Offset: 240
; CHECK-NEXT:              }
; CHECK-NEXT:            CBuffer: s


target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%s = type { %struct.S }

;struct S {
;    row_major int4x3 m;
;    int1x3 m1[2];
;    int3x1 m2;
;    int4x3 m3;
;    row_major int1x3 m4[2];
;    row_major int3x1 m5;
;};

; Make sure cbuffer reflection is correct with
; High level struct stripped: [rows x <cols x float>] and
;High level struct stripped, one row: <cols x float> for m1.

%struct.S = type { [4 x <3 x i32>], [2 x <3 x i32>], [3 x i32], [4 x <3 x i32>], [2 x <3 x i32>], [3 x <1 x i32>] }

@s = external constant %dx.types.Handle

; Function Attrs: nounwind readonly
define float @"\01?foo@@YAMXZ"() #0 {
; force use cb.
  %1 = load %dx.types.Handle, %dx.types.Handle* @s, align 4
  ret float 1.000000e+00
}

attributes #0 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.typeAnnotations = !{!7, !23}
!dx.entryPoints = !{!28}

!0 = !{!"dxc"}
!1 = !{i32 1, i32 8}
!2 = !{i32 0, i32 0}
!3 = !{!"lib", i32 6, i32 15}
!4 = !{null, null, !5, null}
!5 = !{!6}
!6 = !{i32 0, %s* bitcast (%dx.types.Handle* @s to %s*), !"s", i32 -1, i32 -1, i32 1, i32 276, null}
!7 = !{i32 0, %struct.S undef, !8, %s undef, !21}
!8 = !{i32 276, !9, !11, !13, !15, !17, !19}
!9 = !{i32 6, !"m", i32 2, !10, i32 3, i32 0, i32 7, i32 4}
!10 = !{i32 4, i32 3, i32 1}
!11 = !{i32 6, !"m1", i32 2, !12, i32 3, i32 64, i32 7, i32 4}
!12 = !{i32 1, i32 3, i32 2}
!13 = !{i32 6, !"m2", i32 2, !14, i32 3, i32 148, i32 7, i32 4}
!14 = !{i32 3, i32 1, i32 2}
!15 = !{i32 6, !"m3", i32 2, !16, i32 3, i32 160, i32 7, i32 4}
!16 = !{i32 4, i32 3, i32 2}
!17 = !{i32 6, !"m4", i32 2, !18, i32 3, i32 208, i32 7, i32 4}
!18 = !{i32 1, i32 3, i32 1}
!19 = !{i32 6, !"m5", i32 2, !20, i32 3, i32 240, i32 7, i32 4}
!20 = !{i32 3, i32 1, i32 1}
!21 = !{i32 276, !22}
!22 = !{i32 6, !"s", i32 3, i32 0}
!23 = !{i32 1, float ()* @"\01?foo@@YAMXZ", !24}
!24 = !{!25}
!25 = !{i32 1, !26, !27}
!26 = !{i32 7, i32 9}
!27 = !{}
!28 = !{null, !"", null, !4, null}
