; RUN: %opt %s -hlsl-passes-resume -dxil-remove-dead-blocks -S | FileCheck %s

; Run the pass, to make sure that %val.0 is deleted since its only use is multiplied by 0.
;  %val.0 = phi float [ 1.000000e+00, %if.then ], [ 0.000000e+00, %entry ]
;  %mul = fmul fast float %val.0, 0.000000e+00, !dbg !28

; CHECK: @main
; CHECK-NOT: phi float

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%cb = type { float }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.CBufRet.f32 = type { float, float, float, float }

@cb = external constant %cb
@llvm.used = appending global [1 x i8*] [i8* bitcast (%cb* @cb to i8*)], section "llvm.metadata"

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32, %cb*, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb) #0

; Function Attrs: convergent
declare void @dx.noop() #1

; Function Attrs: nounwind
define void @main(float* noalias) #2 {
entry:
  %1 = load %cb, %cb* @cb
  %cb = call %dx.types.Handle @dx.op.createHandleForLib.cb(i32 160, %cb %1)  ; CreateHandleForLib(Resource)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %cb, %dx.types.ResourceProperties { i32 13, i32 4 })  ; AnnotateHandle(res,props)  resource: CBuffer
  call void @dx.noop(), !dbg !38 ; line:19 col:9
  call void @llvm.dbg.value(metadata float 0.000000e+00, i64 0, metadata !39, metadata !40), !dbg !38 ; var:"val" !DIExpression() func:"main"
  %3 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %2, i32 0), !dbg !41 ; line:20 col:7  ; CBufferLoadLegacy(handle,regIndex)
  %4 = extractvalue %dx.types.CBufRet.f32 %3, 0, !dbg !41 ; line:20 col:7
  %tobool = fcmp fast une float %4, 0.000000e+00, !dbg !41 ; line:20 col:7
  br i1 %tobool, label %if.then, label %if.end, !dbg !43 ; line:20 col:7

if.then:                                          ; preds = %entry
  call void @dx.noop(), !dbg !44 ; line:21 col:9
  call void @llvm.dbg.value(metadata float 1.000000e+00, i64 0, metadata !39, metadata !40), !dbg !38 ; var:"val" !DIExpression() func:"main"
  br label %if.end, !dbg !45 ; line:21 col:5

if.end:                                           ; preds = %if.then, %entry
  %val.0 = phi float [ 1.000000e+00, %if.then ], [ 0.000000e+00, %entry ]
  call void @llvm.dbg.value(metadata float %val.0, i64 0, metadata !39, metadata !40), !dbg !38 ; var:"val" !DIExpression() func:"main"
  call void @dx.noop(), !dbg !46 ; line:23 col:9
  call void @llvm.dbg.value(metadata float 0.000000e+00, i64 0, metadata !47, metadata !40), !dbg !46 ; var:"zero" !DIExpression() func:"main"
  %mul = fmul fast float %val.0, 0.000000e+00, !dbg !48 ; line:24 col:19
  call void @dx.noop(), !dbg !49 ; line:24 col:9
  call void @llvm.dbg.value(metadata float %mul, i64 0, metadata !50, metadata !40), !dbg !49 ; var:"ret" !DIExpression() func:"main"
  call void @dx.noop(), !dbg !51 ; line:26 col:3
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %mul), !dbg !51 ; line:26 col:3  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void, !dbg !51 ; line:26 col:3
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.cb(i32, %cb) #3

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

attributes #0 = { nounwind readnone }
attributes #1 = { convergent }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!11, !12}
!pauseresume = !{!13}
!llvm.ident = !{!14}
!dx.source.contents = !{!15}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!16}
!dx.source.args = !{!17}
!dx.version = !{!18}
!dx.valver = !{!19}
!dx.shaderModel = !{!20}
!dx.resources = !{!21}
!dx.typeAnnotations = !{!24, !27}
!dx.entryPoints = !{!33}
!dx.rootSignature = !{!37}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "dxc(private) 1.7.0.3848 (dxil-op-cache-init, 44ba911a0)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !8)
!1 = !DIFile(filename: "delete_constant_dce.hlsl", directory: "")
!2 = !{}
!3 = !{!4}
!4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 18, type: !5, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagPrototyped, isOptimized: false, function: void (float*)* @main)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!8 = !{!9}
!9 = !DIGlobalVariable(name: "foo", linkageName: "\01?foo@cb@@3MB", scope: !0, file: !1, line: 14, type: !10, isLocal: false, isDefinition: true)
!10 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !7)
!11 = !{i32 2, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!14 = !{!"dxc(private) 1.7.0.3848 (dxil-op-cache-init, 44ba911a0)"}
!15 = !{!"delete_constant_dce.hlsl", !"// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s\0D\0A\0D\0A// This test verifies the fix for a deficiency in RemoveDeadBlocks where:\0D\0A//\0D\0A// - Value 'ret' that can be reduced to constant by DxilValueCache is removed\0D\0A// - It held on uses for a PHI 'val', but 'val' was not removed\0D\0A// - 'val' is not used, but also not DCE'ed until after DeleteDeadRegion is run\0D\0A// - DeleteDeadRegion cannot delete 'if (foo)' because 'val' still exists.\0D\0A\0D\0A// CHECK: @main\0D\0A// CHECK-NOT: phi\0D\0A\0D\0Acbuffer cb : register(b0) {\0D\0A  float foo;\0D\0A}\0D\0A\0D\0A[RootSignature(\22\22)]\0D\0Afloat main() : SV_Target {\0D\0A  float val = 0;\0D\0A  if (foo)\0D\0A    val = 1;\0D\0A\0D\0A  float zero = 0;\0D\0A  float ret = val * zero;\0D\0A\0D\0A  return ret;\0D\0A}\0D\0A"}
!16 = !{!"delete_constant_dce.hlsl"}
!17 = !{!"-E", !"main", !"-T", !"ps_6_0", !"-fcgl", !"-Od", !"-Zi", !"-Qembed_debug"}
!18 = !{i32 1, i32 0}
!19 = !{i32 1, i32 7}
!20 = !{!"ps", i32 6, i32 0}
!21 = !{null, null, !22, null}
!22 = !{!23}
!23 = !{i32 0, %cb* @cb, !"cb", i32 0, i32 0, i32 1, i32 4, null}
!24 = !{i32 0, %cb undef, !25}
!25 = !{i32 4, !26}
!26 = !{i32 6, !"foo", i32 3, i32 0, i32 7, i32 9}
!27 = !{i32 1, void (float*)* @main, !28}
!28 = !{!29, !30}
!29 = !{i32 0, !2, !2}
!30 = !{i32 1, !31, !32}
!31 = !{i32 4, !"SV_Target", i32 7, i32 9}
!32 = !{i32 0}
!33 = !{void (float*)* @main, !"main", !34, !21, null}
!34 = !{null, !35, null}
!35 = !{!36}
!36 = !{i32 0, !"SV_Target", i8 9, i8 16, !32, i8 0, i32 1, i8 1, i32 0, i8 0, null}
!37 = !{[24 x i8] c"\02\00\00\00\00\00\00\00\18\00\00\00\00\00\00\00\18\00\00\00\00\00\00\00"}
!38 = !DILocation(line: 19, column: 9, scope: !4)
!39 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "val", scope: !4, file: !1, line: 19, type: !7)
!40 = !DIExpression()
!41 = !DILocation(line: 20, column: 7, scope: !42)
!42 = distinct !DILexicalBlock(scope: !4, file: !1, line: 20, column: 7)
!43 = !DILocation(line: 20, column: 7, scope: !4)
!44 = !DILocation(line: 21, column: 9, scope: !42)
!45 = !DILocation(line: 21, column: 5, scope: !42)
!46 = !DILocation(line: 23, column: 9, scope: !4)
!47 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "zero", scope: !4, file: !1, line: 23, type: !7)
!48 = !DILocation(line: 24, column: 19, scope: !4)
!49 = !DILocation(line: 24, column: 9, scope: !4)
!50 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "ret", scope: !4, file: !1, line: 24, type: !7)
!51 = !DILocation(line: 26, column: 3, scope: !4)
