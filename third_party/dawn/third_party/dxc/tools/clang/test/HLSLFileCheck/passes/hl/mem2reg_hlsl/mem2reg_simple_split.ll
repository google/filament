; RUN: %opt %s -dxil-cond-mem2reg -S | FileCheck %s

; CHECK: @main

; CHECK-NOT: alloca
; CHECK-NOT: getelementptr
; CHECK-NOT: store
; CHECK-NOT: load

; CHECK-DAG: !DIExpression(DW_OP_bit_piece, 0, 32)
; CHECK-DAG: !DIExpression(DW_OP_bit_piece, 32, 32)

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%cb = type { [10 x float] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@cb = external constant %cb

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: nounwind readnone
declare %cb* @"dx.hl.subscript.cb.%cb* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32, %cb*, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb) #0

; Function Attrs: convergent
declare void @dx.noop() #1

; Function Attrs: nounwind
define void @main(float* noalias, i32, i32) #2 {
entry:
  %tmp2.0 = alloca float
  %tmp.0 = alloca float
  %arr.0 = alloca [2 x float]
  %number.addr.i.11 = alloca i32, align 4, !dx.temp !2
  %number.addr.i = alloca i32, align 4, !dx.temp !2
  %retval = alloca float, align 4, !dx.temp !2
  %j.addr = alloca i32, align 4, !dx.temp !2
  %i.addr = alloca i32, align 4, !dx.temp !2
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32 0, %cb* @cb, i32 0)
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32 11, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 13, i32 148 }, %cb undef)
  %5 = call %cb* @"dx.hl.subscript.cb.%cb* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %4, i32 0)
  store i32 %2, i32* %j.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %j.addr, metadata !30, metadata !31), !dbg !32
  store i32 %1, i32* %i.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %i.addr, metadata !33, metadata !31), !dbg !34
  call void @llvm.dbg.declare(metadata [2 x float]* %arr.0, metadata !35, metadata !31), !dbg !39
  %6 = load i32, i32* %i.addr, align 4, !dbg !40
  call void @dx.noop(), !dbg !41
  store i32 %6, i32* %number.addr.i, align 4, !dbg !41, !noalias !42
  call void @llvm.dbg.declare(metadata float* %tmp.0, metadata !45, metadata !31), !dbg !46
  %7 = load i32, i32* %number.addr.i, align 4, !dbg !48, !noalias !42
  %conv.i = sitofp i32 %7 to float, !dbg !48
  call void @dx.noop() #2, !dbg !49, !noalias !42
  store float %conv.i, float* %tmp.0, align 4, !dbg !49, !alias.scope !42
  call void @dx.noop() #2, !dbg !50, !noalias !42
  call void @dx.noop(), !dbg !41
  %8 = getelementptr inbounds [2 x float], [2 x float]* %arr.0, i32 0, i32 0, !dbg !41
  %9 = load float, float* %tmp.0, !dbg !41
  store float %9, float* %8, !dbg !41
  %10 = load i32, i32* %j.addr, align 4, !dbg !51
  call void @dx.noop(), !dbg !52
  store i32 %10, i32* %number.addr.i.11, align 4, !dbg !52, !noalias !53
  call void @llvm.dbg.declare(metadata float* %tmp2.0, metadata !45, metadata !31), !dbg !56
  %11 = load i32, i32* %number.addr.i.11, align 4, !dbg !58, !noalias !53
  %conv.i.12 = sitofp i32 %11 to float, !dbg !58
  call void @dx.noop() #2, !dbg !59, !noalias !53
  store float %conv.i.12, float* %tmp2.0, align 4, !dbg !59, !alias.scope !53
  call void @dx.noop() #2, !dbg !60, !noalias !53
  call void @dx.noop(), !dbg !52
  %12 = getelementptr inbounds [2 x float], [2 x float]* %arr.0, i32 0, i32 1, !dbg !52
  %13 = load float, float* %tmp2.0, !dbg !52
  store float %13, float* %12, !dbg !52
  %member2 = getelementptr inbounds [2 x float], [2 x float]* %arr.0, i32 0, i32 0, !dbg !61
  %14 = load float, float* %member2, align 4, !dbg !61
  %conv = fptoui float %14 to i32, !dbg !62
  %arrayidx49 = getelementptr inbounds %cb, %cb* %5, i32 0, i32 0, i32 %conv, !dbg !63
  %15 = load float, float* %arrayidx49, align 4, !dbg !63
  %member61 = getelementptr inbounds [2 x float], [2 x float]* %arr.0, i32 0, i32 1, !dbg !64
  %16 = load float, float* %member61, align 4, !dbg !64
  %conv7 = fptoui float %16 to i32, !dbg !65
  %arrayidx810 = getelementptr inbounds %cb, %cb* %5, i32 0, i32 0, i32 %conv7, !dbg !66
  %17 = load float, float* %arrayidx810, align 4, !dbg !66
  %add = fadd float %15, %17, !dbg !67
  store float %add, float* %retval, !dbg !68
  %18 = load float, float* %retval, !dbg !68
  call void @dx.noop(), !dbg !68
  call void @llvm.dbg.declare(metadata i32* %number.addr.i, metadata !69, metadata !31), !dbg !70
  call void @llvm.dbg.declare(metadata i32* %number.addr.i.11, metadata !69, metadata !31), !dbg !71
  store float %18, float* %0, !dbg !68
  ret void, !dbg !68
}

attributes #0 = { nounwind readnone }
attributes #1 = { convergent }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!23, !24}
!pauseresume = !{!25}
!llvm.ident = !{!26}
!dx.source.contents = !{!27}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!28}
!dx.source.args = !{!29}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 3.7 (tags/RELEASE_370/final)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !17)
!1 = !DIFile(filename: "F:\5Ctest\5Cepic_od\5Cminimal.hlsl", directory: "")
!2 = !{}
!3 = !{!4, !10}
!4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !5, isLocal: false, isDefinition: true, scopeLine: 17, flags: DIFlagPrototyped, isOptimized: false, function: void (float*, i32, i32)* @main)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8, !8}
!7 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!8 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !1, line: 13, baseType: !9)
!9 = !DIBasicType(name: "unsigned int", size: 32, align: 32, encoding: DW_ATE_unsigned)
!10 = !DISubprogram(name: "make_struct", linkageName: "\01?make_struct@@YA?AUMyStruct@@H@Z", scope: !1, file: !1, line: 6, type: !11, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !16}
!13 = !DICompositeType(tag: DW_TAG_structure_type, name: "MyStruct", file: !1, line: 2, size: 32, align: 32, elements: !14)
!14 = !{!15}
!15 = !DIDerivedType(tag: DW_TAG_member, name: "member", scope: !13, file: !1, line: 3, baseType: !7, size: 32, align: 32)
!16 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!17 = !{!18}
!18 = !DIGlobalVariable(name: "foo", linkageName: "\01?foo@cb@@3QBMB", scope: !0, file: !1, line: 13, type: !19, isLocal: false, isDefinition: true)
!19 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 320, align: 32, elements: !21)
!20 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !7)
!21 = !{!22}
!22 = !DISubrange(count: 10)
!23 = !{i32 2, !"Dwarf Version", i32 4}
!24 = !{i32 2, !"Debug Info Version", i32 3}
!25 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!26 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!27 = !{!"F:\5Ctest\5Cepic_od\5Cminimal.hlsl", !""} ; NOTE: Manually deleted this content for the test.
!28 = !{!"F:\5Ctest\5Cepic_od\5Cminimal.hlsl"}
!29 = !{!"-E", !"main", !"-T", !"ps_6_0", !"/Od", !"/Zi", !"-Qembed_debug"}
!30 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "j", arg: 2, scope: !4, file: !1, line: 17, type: !8)
!31 = !DIExpression()
!32 = !DILocation(line: 17, column: 29, scope: !4)
!33 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "i", arg: 1, scope: !4, file: !1, line: 17, type: !8)
!34 = !DILocation(line: 17, column: 17, scope: !4)
!35 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "arr", scope: !4, file: !1, line: 18, type: !36)
!36 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, size: 64, align: 32, elements: !37)
!37 = !{!38}
!38 = !DISubrange(count: 2)
!39 = !DILocation(line: 18, column: 12, scope: !4)
!40 = !DILocation(line: 19, column: 24, scope: !4)
!41 = !DILocation(line: 19, column: 12, scope: !4)
!42 = !{!43}
!43 = distinct !{!43, !44, !"\01?make_struct@@YA?AUMyStruct@@H@Z: %agg.result"}
!44 = distinct !{!44, !"\01?make_struct@@YA?AUMyStruct@@H@Z"}
!45 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "ret", arg: 0, scope: !10, file: !1, line: 7, type: !13)
!46 = !DILocation(line: 7, column: 12, scope: !10, inlinedAt: !47)
!47 = distinct !DILocation(line: 19, column: 12, scope: !4)
!48 = !DILocation(line: 8, column: 16, scope: !10, inlinedAt: !47)
!49 = !DILocation(line: 8, column: 14, scope: !10, inlinedAt: !47)
!50 = !DILocation(line: 9, column: 3, scope: !10, inlinedAt: !47)
!51 = !DILocation(line: 20, column: 24, scope: !4)
!52 = !DILocation(line: 20, column: 12, scope: !4)
!53 = !{!54}
!54 = distinct !{!54, !55, !"\01?make_struct@@YA?AUMyStruct@@H@Z: %agg.result"}
!55 = distinct !{!55, !"\01?make_struct@@YA?AUMyStruct@@H@Z"}
!56 = !DILocation(line: 7, column: 12, scope: !10, inlinedAt: !57)
!57 = distinct !DILocation(line: 20, column: 12, scope: !4)
!58 = !DILocation(line: 8, column: 16, scope: !10, inlinedAt: !57)
!59 = !DILocation(line: 8, column: 14, scope: !10, inlinedAt: !57)
!60 = !DILocation(line: 9, column: 3, scope: !10, inlinedAt: !57)
!61 = !DILocation(line: 22, column: 21, scope: !4)
!62 = !DILocation(line: 22, column: 14, scope: !4)
!63 = !DILocation(line: 22, column: 10, scope: !4)
!64 = !DILocation(line: 22, column: 42, scope: !4)
!65 = !DILocation(line: 22, column: 35, scope: !4)
!66 = !DILocation(line: 22, column: 31, scope: !4)
!67 = !DILocation(line: 22, column: 29, scope: !4)
!68 = !DILocation(line: 22, column: 3, scope: !4)
!69 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "number", arg: 1, scope: !10, file: !1, line: 6, type: !16)
!70 = !DILocation(line: 6, column: 26, scope: !10, inlinedAt: !47)
!71 = !DILocation(line: 6, column: 26, scope: !10, inlinedAt: !57)