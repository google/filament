; RUN: %opt %s -dxil-cond-mem2reg -S | FileCheck %s

; CHECK: @main
; CHECK-NOT: alloca [4 x float]
; CHECK-NOT: !dx.dbg.varlayout

; CHECK-DAG: !DIExpression(DW_OP_bit_piece, 0, 32)
; CHECK-DAG: !DIExpression(DW_OP_bit_piece, 32, 32)
; CHECK-DAG: !DIExpression(DW_OP_bit_piece, 64, 32)
; CHECK-DAG: !DIExpression(DW_OP_bit_piece, 96, 32)

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: convergent
declare void @dx.noop() #1

; Function Attrs: nounwind
define void @main(<2 x float>* noalias, i32) #2 {
entry:
  %values.0 = alloca [4 x float]
  %values.1 = alloca [4 x float]
  %retval = alloca <2 x float>, align 4, !dx.temp !2
  call void @llvm.dbg.declare(metadata [4 x float]* %values.0, metadata !25, metadata !29), !dbg !30, !dx.dbg.varlayout !31
  call void @llvm.dbg.declare(metadata [4 x float]* %values.1, metadata !25, metadata !32), !dbg !30, !dx.dbg.varlayout !33
  %2 = getelementptr [4 x float], [4 x float]* %values.0, i32 0, i32 0, !dbg !34
  %3 = getelementptr [4 x float], [4 x float]* %values.1, i32 0, i32 0, !dbg !34
  call void @dx.noop(), !dbg !34
  store float 1.000000e+00, float* %2, !dbg !34
  store float 2.000000e+00, float* %3, !dbg !34
  %4 = getelementptr [4 x float], [4 x float]* %values.0, i32 0, i32 1, !dbg !34
  %5 = getelementptr [4 x float], [4 x float]* %values.1, i32 0, i32 1, !dbg !34
  call void @dx.noop(), !dbg !34
  store float 3.000000e+00, float* %4, !dbg !34
  store float 4.000000e+00, float* %5, !dbg !34
  %6 = getelementptr [4 x float], [4 x float]* %values.0, i32 0, i32 2, !dbg !34
  %7 = getelementptr [4 x float], [4 x float]* %values.1, i32 0, i32 2, !dbg !34
  call void @dx.noop(), !dbg !34
  store float 5.000000e+00, float* %6, !dbg !34
  store float 6.000000e+00, float* %7, !dbg !34
  %8 = getelementptr [4 x float], [4 x float]* %values.0, i32 0, i32 3, !dbg !34
  %9 = getelementptr [4 x float], [4 x float]* %values.1, i32 0, i32 3, !dbg !34
  call void @dx.noop(), !dbg !34
  store float 7.000000e+00, float* %8, !dbg !34
  store float 8.000000e+00, float* %9, !dbg !34
  %10 = getelementptr [4 x float], [4 x float]* %values.0, i32 0, i32 3, !dbg !35
  %11 = getelementptr [4 x float], [4 x float]* %values.1, i32 0, i32 3, !dbg !35
  %load = load float, float* %10, !dbg !35
  %insert = insertelement <2 x float> undef, float %load, i64 0, !dbg !35
  %load6 = load float, float* %11, !dbg !35
  %insert7 = insertelement <2 x float> %insert, float %load6, i64 1, !dbg !35
  store <2 x float> %insert7, <2 x float>* %retval, !dbg !36
  %12 = load <2 x float>, <2 x float>* %retval, !dbg !36
  call void @dx.noop(), !dbg !36
  store <2 x float> %12, <2 x float>* %0, !dbg !36
  ret void, !dbg !36
}

attributes #0 = { nounwind readnone }
attributes #1 = { convergent }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!18, !19}
!pauseresume = !{!20}
!llvm.ident = !{!21}
!dx.source.contents = !{!22}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!23}
!dx.source.args = !{!24}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 3.7 (tags/RELEASE_370/final)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, retainedTypes: !3, subprograms: !14)
!1 = !DIFile(filename: "F:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cdxil\5Cdebug\5Cno_fold_vec_array.hlsl", directory: "")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_typedef, name: "float2", file: !1, baseType: !5)
!5 = !DICompositeType(tag: DW_TAG_class_type, name: "vector<float, 2>", file: !1, size: 64, align: 32, elements: !6, templateParams: !10)
!6 = !{!7, !9}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !5, file: !1, baseType: !8, size: 32, align: 32, flags: DIFlagPublic)
!8 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!9 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !5, file: !1, baseType: !8, size: 32, align: 32, offset: 32, flags: DIFlagPublic)
!10 = !{!11, !12}
!11 = !DITemplateTypeParameter(name: "element", type: !8)
!12 = !DITemplateValueParameter(name: "element_count", type: !13, value: i32 2)
!13 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!14 = !{!15}
!15 = !DISubprogram(name: "main", scope: !1, file: !1, line: 7, type: !16, isLocal: false, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, function: void (<2 x float>*, i32)* @main)
!16 = !DISubroutineType(types: !17)
!17 = !{!4, !13}
!18 = !{i32 2, !"Dwarf Version", i32 4}
!19 = !{i32 2, !"Debug Info Version", i32 3}
!20 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!21 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!22 = !{!"F:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cdxil\5Cdebug\5Cno_fold_vec_array.hlsl", !"// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s\0D\0A\0D\0A// Check that arrays of vectors still work with -Od\0D\0A// without all the inst-simplify\0D\0A\0D\0A[RootSignature(\22\22)]\0D\0Afloat2 main(int index : INDEX) : SV_Target {\0D\0A\0D\0A  float2 values[4] = {\0D\0A    float2(1,2),\0D\0A    float2(3,4),\0D\0A    float2(5,6),\0D\0A    float2(7,8),\0D\0A  };\0D\0A\0D\0A  // CHECK: alloca [4 x float]\0D\0A  // CHECK: alloca [4 x float]\0D\0A\0D\0A  // CHECK: store\0D\0A  // CHECK: store\0D\0A  // CHECK: store\0D\0A  // CHECK: store\0D\0A\0D\0A  // CHECK: store\0D\0A  // CHECK: store\0D\0A  // CHECK: store\0D\0A  // CHECK: store\0D\0A\0D\0A  // CHECK: load\0D\0A  // CHECK: load\0D\0A\0D\0A  return values[3];\0D\0A}\0D\0A\0D\0A"}
!23 = !{!"F:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cdxil\5Cdebug\5Cno_fold_vec_array.hlsl"}
!24 = !{!"-E", !"main", !"-T", !"ps_6_0", !"-Od", !"-Zi", !"-Qembed_debug"}
!25 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "values", scope: !15, file: !1, line: 9, type: !26)
!26 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 256, align: 32, elements: !27)
!27 = !{!28}
!28 = !DISubrange(count: 4)
!29 = !DIExpression(DW_OP_bit_piece, 0, 32)
!30 = !DILocation(line: 9, column: 10, scope: !15)
!31 = !{i32 0, i32 64, i32 4}
!32 = !DIExpression(DW_OP_bit_piece, 32, 32)
!33 = !{i32 32, i32 64, i32 4}
!34 = !DILocation(line: 9, column: 22, scope: !15)
!35 = !DILocation(line: 32, column: 10, scope: !15)
!36 = !DILocation(line: 32, column: 3, scope: !15)