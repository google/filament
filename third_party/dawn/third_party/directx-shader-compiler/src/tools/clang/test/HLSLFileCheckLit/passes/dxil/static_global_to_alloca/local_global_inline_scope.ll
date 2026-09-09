; RUN: opt %s -hlsl-passes-resume -static-global-to-alloca -S | FileCheck %s

; Check that the debug info for a global variable is present for every inlined scope

; CHECK: @main
; CHECK: %[[alloca:.+]] = alloca i32

; CHECK-DAG: ![[main_scope:[0-9]+]] = !DISubprogram(name: "main"
; CHECK-DAG: ![[main_var:[0-9]+]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.g_cond", arg: {{[0-9]+}}, scope: ![[main_scope]],
; CHECK-DAG: call void @llvm.dbg.declare(metadata i32* %[[alloca]], metadata ![[main_var]],

; CHECK-DAG: ![[foo_scope:[0-9]+]] = !DISubprogram(name: "foo"
; CHECK-DAG: ![[foo_var:[0-9]+]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.g_cond", arg: {{[0-9]+}}, scope: ![[foo_scope]],
; CHECK-DAG: call void @llvm.dbg.declare(metadata i32* %[[alloca]], metadata ![[foo_var]],

; CHECK-DAG: ![[bar_scope:[0-9]+]] = !DISubprogram(name: "bar"
; CHECK-DAG: ![[bar_var:[0-9]+]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.g_cond", arg: {{[0-9]+}}, scope: ![[bar_scope]],
; CHECK-DAG: call void @llvm.dbg.declare(metadata i32* %[[alloca]], metadata ![[bar_var]],

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%ConstantBuffer = type opaque
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?tex0@@3V?$Texture2D@V?$vector@M$03@@@@A" = external global %"class.Texture2D<vector<float, 4> >", align 4
@"\01?tex1@@3V?$Texture2D@V?$vector@M$03@@@@A" = external global %"class.Texture2D<vector<float, 4> >", align 4
@g_cond = internal global i32 0, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define <4 x float> @main(i32 %a) #0 {
entry:
  %retval.i.i = alloca <4 x float>, align 4, !dx.temp !2
  %retval.i = alloca <4 x float>, align 4, !dx.temp !2
  %ret.i = alloca <4 x float>, align 4
  %retval = alloca <4 x float>, align 4, !dx.temp !2
  %a.addr = alloca i32, align 4, !dx.temp !2
  store i32 %a, i32* %a.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %a.addr, metadata !61, metadata !62), !dbg !63
  %0 = load i32, i32* %a.addr, align 4, !dbg !64
  %cmp = icmp ne i32 %0, 0, !dbg !65
  %tobool = icmp ne i1 %cmp, false, !dbg !65
  %frombool = zext i1 %tobool to i32, !dbg !66
  call void @dx.noop(), !dbg !66
  store i32 %frombool, i32* @g_cond, align 4, !dbg !66
  call void @dx.noop(), !dbg !67
  call void @dx.noop() #0, !dbg !68
  %1 = load i32, i32* @g_cond, align 4, !dbg !68
  %tobool.i = icmp ne i32 %1, 0, !dbg !68
  br i1 %tobool.i, label %cond.true.i, label %cond.false.i, !dbg !68

cond.true.i:                                      ; preds = %entry
  %2 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* @"\01?tex0@@3V?$Texture2D@V?$vector@M$03@@@@A", !dbg !70
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 4> >\22)"(i32 0, %"class.Texture2D<vector<float, 4> >" %2) #0, !dbg !70
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 4> >\22)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 2, i32 1033 }, %"class.Texture2D<vector<float, 4> >" undef) #0, !dbg !70
  %5 = call <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32 231, %dx.types.Handle %4, <3 x i32> zeroinitializer) #0, !dbg !70
  br label %cond.end.i, !dbg !68

cond.false.i:                                     ; preds = %entry
  %6 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* @"\01?tex1@@3V?$Texture2D@V?$vector@M$03@@@@A", !dbg !71
  %7 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 4> >\22)"(i32 0, %"class.Texture2D<vector<float, 4> >" %6) #0, !dbg !71
  %8 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 4> >\22)"(i32 14, %dx.types.Handle %7, %dx.types.ResourceProperties { i32 2, i32 1033 }, %"class.Texture2D<vector<float, 4> >" undef) #0, !dbg !71
  %9 = call <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32 231, %dx.types.Handle %8, <3 x i32> zeroinitializer) #0, !dbg !71
  br label %cond.end.i, !dbg !68

cond.end.i:                                       ; preds = %cond.false.i, %cond.true.i
  %cond.i = phi <4 x float> [ %5, %cond.true.i ], [ %9, %cond.false.i ], !dbg !68
  call void @dx.noop() #0, !dbg !72
  store <4 x float> %cond.i, <4 x float>* %ret.i, align 4, !dbg !72
  call void @dx.noop() #0, !dbg !73
  call void @dx.noop() #0, !dbg !74
  %10 = load i32, i32* @g_cond, align 4, !dbg !74
  %tobool.i.i = icmp ne i32 %10, 0, !dbg !74
  br i1 %tobool.i.i, label %if.then.i.i, label %if.end.i.i, !dbg !77

if.then.i.i:                                      ; preds = %cond.end.i
  %11 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* @"\01?tex0@@3V?$Texture2D@V?$vector@M$03@@@@A", !dbg !78
  %12 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 4> >\22)"(i32 0, %"class.Texture2D<vector<float, 4> >" %11) #0, !dbg !78
  %13 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 4> >\22)"(i32 14, %dx.types.Handle %12, %dx.types.ResourceProperties { i32 2, i32 1033 }, %"class.Texture2D<vector<float, 4> >" undef) #0, !dbg !78
  %14 = call <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32 231, %dx.types.Handle %13, <3 x i32> <i32 1, i32 1, i32 1>) #0, !dbg !78
  store <4 x float> %14, <4 x float>* %retval.i.i, !dbg !79
  br label %"\01?foo@@YA?AV?$vector@M$03@@XZ.exit", !dbg !79

if.end.i.i:                                       ; preds = %cond.end.i
  store <4 x float> zeroinitializer, <4 x float>* %retval.i.i, !dbg !80
  br label %"\01?foo@@YA?AV?$vector@M$03@@XZ.exit", !dbg !80

"\01?foo@@YA?AV?$vector@M$03@@XZ.exit":           ; preds = %if.then.i.i, %if.end.i.i
  %15 = load <4 x float>, <4 x float>* %retval.i.i, !dbg !81
  call void @dx.noop() #0, !dbg !81
  %16 = load <4 x float>, <4 x float>* %ret.i, align 4, !dbg !82
  %add.i = fadd <4 x float> %16, %15, !dbg !82
  call void @dx.noop() #0, !dbg !82
  store <4 x float> %add.i, <4 x float>* %ret.i, align 4, !dbg !82
  %17 = load <4 x float>, <4 x float>* %ret.i, align 4, !dbg !83
  store <4 x float> %17, <4 x float>* %retval.i, !dbg !84
  %18 = load <4 x float>, <4 x float>* %retval.i, !dbg !84
  call void @dx.noop() #0, !dbg !84
  store <4 x float> %18, <4 x float>* %retval, !dbg !85
  %19 = load <4 x float>, <4 x float>* %retval, !dbg !85
  call void @dx.noop(), !dbg !85
  call void @llvm.dbg.declare(metadata <4 x float>* %ret.i, metadata !86, metadata !62), !dbg !72
  ret <4 x float> %19, !dbg !85
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readonly
declare <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 4> >\22)"(i32, %"class.Texture2D<vector<float, 4> >") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 4> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture2D<vector<float, 4> >") #1

; Function Attrs: convergent
declare void @dx.noop() #3

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }
attributes #3 = { convergent }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!33, !34}
!pauseresume = !{!35}
!llvm.ident = !{!36}
!dx.source.contents = !{!37}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!38}
!dx.source.args = !{!39}
!dx.version = !{!40}
!dx.valver = !{!41}
!dx.shaderModel = !{!42}
!dx.typeAnnotations = !{!43}
!dx.entryPoints = !{!49}
!dx.fnprops = !{!57}
!dx.options = !{!58, !59}
!dx.rootSignature = !{!60}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "dxc(private) 1.8.0.5063 (local_global_var_debug_info, 3464c49d3-dirty)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !25)
!1 = !DIFile(filename: "C:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cdxil\5Cdebug\5Clocal_global_inline_scope.hlsl", directory: "")
!2 = !{}
!3 = !{!4, !21, !24}
!4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 37, type: !5, isLocal: false, isDefinition: true, scopeLine: 37, flags: DIFlagPrototyped, isOptimized: false, function: <4 x float> (i32)* @main)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !19}
!7 = !DIDerivedType(tag: DW_TAG_typedef, name: "float4", file: !1, baseType: !8)
!8 = !DICompositeType(tag: DW_TAG_class_type, name: "vector<float, 4>", file: !1, size: 128, align: 32, elements: !9, templateParams: !15)
!9 = !{!10, !12, !13, !14}
!10 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !8, file: !1, baseType: !11, size: 32, align: 32, flags: DIFlagPublic)
!11 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!12 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !8, file: !1, baseType: !11, size: 32, align: 32, offset: 32, flags: DIFlagPublic)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !8, file: !1, baseType: !11, size: 32, align: 32, offset: 64, flags: DIFlagPublic)
!14 = !DIDerivedType(tag: DW_TAG_member, name: "w", scope: !8, file: !1, baseType: !11, size: 32, align: 32, offset: 96, flags: DIFlagPublic)
!15 = !{!16, !17}
!16 = !DITemplateTypeParameter(name: "element", type: !11)
!17 = !DITemplateValueParameter(name: "element_count", type: !18, value: i32 4)
!18 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!19 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !1, baseType: !20)
!20 = !DIBasicType(name: "unsigned int", size: 32, align: 32, encoding: DW_ATE_unsigned)
!21 = !DISubprogram(name: "foo", linkageName: "\01?foo@@YA?AV?$vector@M$03@@XZ", scope: !1, file: !1, line: 30, type: !22, isLocal: false, isDefinition: true, scopeLine: 30, flags: DIFlagPrototyped, isOptimized: false)
!22 = !DISubroutineType(types: !23)
!23 = !{!7}
!24 = !DISubprogram(name: "bar", linkageName: "\01?bar@@YA?AV?$vector@M$03@@XZ", scope: !1, file: !1, line: 24, type: !22, isLocal: false, isDefinition: true, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: false)
!25 = !{!26, !30, !31}
!26 = !DIGlobalVariable(name: "tex0", linkageName: "\01?tex0@@3V?$Texture2D@V?$vector@M$03@@@@A", scope: !0, file: !1, line: 21, type: !27, isLocal: false, isDefinition: true, variable: %"class.Texture2D<vector<float, 4> >"* @"\01?tex0@@3V?$Texture2D@V?$vector@M$03@@@@A")
!27 = !DICompositeType(tag: DW_TAG_class_type, name: "Texture2D<vector<float, 4> >", file: !1, line: 21, size: 160, align: 32, elements: !2, templateParams: !28)
!28 = !{!29}
!29 = !DITemplateTypeParameter(name: "element", type: !8)
!30 = !DIGlobalVariable(name: "tex1", linkageName: "\01?tex1@@3V?$Texture2D@V?$vector@M$03@@@@A", scope: !0, file: !1, line: 22, type: !27, isLocal: false, isDefinition: true, variable: %"class.Texture2D<vector<float, 4> >"* @"\01?tex1@@3V?$Texture2D@V?$vector@M$03@@@@A")
!31 = !DIGlobalVariable(name: "g_cond", scope: !0, file: !1, line: 19, type: !32, isLocal: true, isDefinition: true, variable: i32* @g_cond)
!32 = !DIBasicType(name: "bool", size: 32, align: 32, encoding: DW_ATE_boolean)
!33 = !{i32 2, !"Dwarf Version", i32 4}
!34 = !{i32 2, !"Debug Info Version", i32 3}
!35 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!36 = !{!"dxc(private) 1.8.0.5063 (local_global_var_debug_info, 3464c49d3-dirty)"}
!37 = !{!"C:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cdxil\5Cdebug\5Clocal_global_inline_scope.hlsl", !""}
!38 = !{!"C:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cdxil\5Cdebug\5Clocal_global_inline_scope.hlsl"}
!39 = !{!"-E", !"main", !"-T", !"ps_6_0", !"/Zi", !"/Od", !"-Qembed_debug"}
!40 = !{i32 1, i32 0}
!41 = !{i32 1, i32 9}
!42 = !{!"ps", i32 6, i32 0}
!43 = !{i32 1, <4 x float> (i32)* @main, !44}
!44 = !{!45, !47}
!45 = !{i32 1, !46, !2}
!46 = !{i32 4, !"sv_target", i32 7, i32 9}
!47 = !{i32 0, !48, !2}
!48 = !{i32 4, !"A", i32 7, i32 5}
!49 = !{<4 x float> (i32)* @main, !"main", null, !50, null}
!50 = !{!51, null, !55, null}
!51 = !{!52, !54}
!52 = !{i32 0, %"class.Texture2D<vector<float, 4> >"* @"\01?tex0@@3V?$Texture2D@V?$vector@M$03@@@@A", !"tex0", i32 0, i32 0, i32 1, i32 2, i32 0, !53}
!53 = !{i32 0, i32 9}
!54 = !{i32 1, %"class.Texture2D<vector<float, 4> >"* @"\01?tex1@@3V?$Texture2D@V?$vector@M$03@@@@A", !"tex1", i32 0, i32 1, i32 1, i32 2, i32 0, !53}
!55 = !{!56}
!56 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!57 = !{<4 x float> (i32)* @main, i32 0, i1 false}
!58 = !{i32 -2147483576}
!59 = !{i32 -1}
!60 = !{[68 x i8] c"\02\00\00\00\01\00\00\00\18\00\00\00\00\00\00\00D\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00$\00\00\00\01\00\00\00,\00\00\00\00\00\00\00\02\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\FF\FF\FF\FF"}
!61 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "a", arg: 1, scope: !4, file: !1, line: 37, type: !19)
!62 = !DIExpression()
!63 = !DILocation(line: 37, column: 18, scope: !4)
!64 = !DILocation(line: 38, column: 12, scope: !4)
!65 = !DILocation(line: 38, column: 14, scope: !4)
!66 = !DILocation(line: 38, column: 10, scope: !4)
!67 = !DILocation(line: 39, column: 10, scope: !4)
!68 = !DILocation(line: 31, column: 16, scope: !21, inlinedAt: !69)
!69 = distinct !DILocation(line: 39, column: 10, scope: !4)
!70 = !DILocation(line: 31, column: 25, scope: !21, inlinedAt: !69)
!71 = !DILocation(line: 31, column: 40, scope: !21, inlinedAt: !69)
!72 = !DILocation(line: 31, column: 10, scope: !21, inlinedAt: !69)
!73 = !DILocation(line: 32, column: 10, scope: !21, inlinedAt: !69)
!74 = !DILocation(line: 25, column: 7, scope: !75, inlinedAt: !76)
!75 = distinct !DILexicalBlock(scope: !24, file: !1, line: 25, column: 7)
!76 = distinct !DILocation(line: 32, column: 10, scope: !21, inlinedAt: !69)
!77 = !DILocation(line: 25, column: 7, scope: !24, inlinedAt: !76)
!78 = !DILocation(line: 26, column: 12, scope: !75, inlinedAt: !76)
!79 = !DILocation(line: 26, column: 5, scope: !75, inlinedAt: !76)
!80 = !DILocation(line: 27, column: 3, scope: !24, inlinedAt: !76)
!81 = !DILocation(line: 28, column: 1, scope: !24, inlinedAt: !76)
!82 = !DILocation(line: 32, column: 7, scope: !21, inlinedAt: !69)
!83 = !DILocation(line: 33, column: 10, scope: !21, inlinedAt: !69)
!84 = !DILocation(line: 33, column: 3, scope: !21, inlinedAt: !69)
!85 = !DILocation(line: 39, column: 3, scope: !4)
!86 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "ret", scope: !21, file: !1, line: 31, type: !7)
