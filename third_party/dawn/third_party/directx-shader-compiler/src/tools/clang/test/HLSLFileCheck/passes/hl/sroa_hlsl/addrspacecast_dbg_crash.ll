; RUN: %opt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Check that the global struct is completely gone
; CHECK-NOT: @"\01?foo@@3UStruct@@A

; CHECK: @main

; Regression test for a crash when there's addrspacecast going into a
; getelementptr, which goes into a dbg.decare. The pass was
; incorrectly calling dropAllReferences on the addrspacecast
; ConstExpr, which invalidates the getelementptr ConstExpr (sets the
; operand to nullptr).

; The test was made with the following original HLSL, and then manually
; reduced to just the lines that cause problems:

;; void GlobalSet(inout float x, float val) {
;;   x = 10;
;; }
;; struct Struct {
;;   int x;
;;   float y;
;;   void Set(float val) {
;;     GlobalSet(x, val);
;;     GlobalSet(y, val);
;;   }
;; };
;;
;; groupshared Struct foo;
;;
;; float main() : SV_Target {
;;   foo.Set(10);
;;   return 0;
;; }
;;

;
; Buffer Definitions:
;
; cbuffer $Globals
; {
;
;   [0 x i8] (type annotation not present)
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; $Globals                          cbuffer      NA          NA     CB0   cb4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.Struct = type { i32, float }
%ConstantBuffer = type opaque

@"\01?foo@@3UStruct@@A" = external addrspace(3) global %struct.Struct, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define float @main() #0 {
entry:
  call void @llvm.dbg.declare(metadata float* getelementptr (%struct.Struct, %struct.Struct* addrspacecast (%struct.Struct addrspace(3)* @"\01?foo@@3UStruct@@A" to %struct.Struct*), i32 0, i32 1), metadata !74, metadata !57), !dbg !75 ; var:"x" !DIExpression() func:"GlobalSet"
  store float 1.000000e+01, float addrspace(3)* getelementptr inbounds (%struct.Struct, %struct.Struct addrspace(3)* @"\01?foo@@3UStruct@@A", i32 0, i32 1), align 4, !dbg !77, !tbaa !51, !alias.scope !71 ; line:3 col:5
  ret float 0.000000e+00, !dbg !84 ; line:18 col:3
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!25, !26}
!pauseresume = !{!27}
!llvm.ident = !{!28}
!dx.source.contents = !{!29}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!30}
!dx.source.args = !{!31}
!dx.version = !{!32}
!dx.valver = !{!33}
!dx.shaderModel = !{!34}
!dx.typeAnnotations = !{!35, !39}
!dx.entryPoints = !{!43}
!dx.fnprops = !{!47}
!dx.options = !{!48, !49}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "dxc(private) 1.7.0.3788 (no_structurize_return_for_lifetime, deb91beeb)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !23)
!1 = !DIFile(filename: "f:\5C\5Caddrspacecast_crash.hlsl", directory: "")
!2 = !{}
!3 = !{!4, !8, !18}
!4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !5, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, function: float ()* @main)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!8 = !DISubprogram(name: "Set", linkageName: "\01?Set@Struct@@QAAXM@Z", scope: !9, file: !1, line: 8, type: !15, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false, declaration: !14)
!9 = !DICompositeType(tag: DW_TAG_structure_type, name: "Struct", file: !1, line: 5, size: 64, align: 32, elements: !10)
!10 = !{!11, !13, !14}
!11 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !9, file: !1, line: 6, baseType: !12, size: 32, align: 32)
!12 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !9, file: !1, line: 7, baseType: !7, size: 32, align: 32, offset: 32)
!14 = !DISubprogram(name: "Set", linkageName: "\01?Set@Struct@@QAAXM@Z", scope: !9, file: !1, line: 8, type: !15, isLocal: false, isDefinition: false, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false)
!15 = !DISubroutineType(types: !16)
!16 = !{null, !17, !7}
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 32, align: 32, flags: DIFlagArtificial | DIFlagObjectPointer)
!18 = !DISubprogram(name: "GlobalSet", linkageName: "\01?GlobalSet@@YAXAIAMM@Z", scope: !1, file: !1, line: 2, type: !19, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false)
!19 = !DISubroutineType(types: !20)
!20 = !{null, !21, !7}
!21 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !22)
!22 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !7)
!23 = !{!24}
!24 = !DIGlobalVariable(name: "foo", linkageName: "\01?foo@@3UStruct@@A", scope: !0, file: !1, line: 14, type: !9, isLocal: false, isDefinition: true, variable: %struct.Struct addrspace(3)* @"\01?foo@@3UStruct@@A")
!25 = !{i32 2, !"Dwarf Version", i32 4}
!26 = !{i32 2, !"Debug Info Version", i32 3}
!27 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!28 = !{!"dxc(private) 1.7.0.3788 (no_structurize_return_for_lifetime, deb91beeb)"}
!29 = !{!"f:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cpasses\5Chl\5Csroa_hlsl\5Caddrspacecast_crash.hlsl", !"\0D\0Avoid GlobalSet(inout float x, float val) {\0D\0A  x = 10;\0D\0A}\0D\0Astruct Struct {\0D\0A  int x;\0D\0A  float y;\0D\0A  void Set(float val) {\0D\0A    GlobalSet(x, val);\0D\0A    GlobalSet(y, val);\0D\0A  }\0D\0A};\0D\0A\0D\0Agroupshared Struct foo;\0D\0A\0D\0Afloat main() : SV_Target {\0D\0A  foo.Set(10);\0D\0A  return 0;\0D\0A}"}
!30 = !{!"f:\5Cdxc\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cpasses\5Chl\5Csroa_hlsl\5Caddrspacecast_crash.hlsl"}
!31 = !{!"-E", !"main", !"-T", !"ps_6_0", !"-fcgl", !"/Zi", !"-Qembed_debug"}
!32 = !{i32 1, i32 0}
!33 = !{i32 1, i32 5}
!34 = !{!"ps", i32 6, i32 0}
!35 = !{i32 0, %struct.Struct undef, !36}
!36 = !{i32 8, !37, !38}
!37 = !{i32 6, !"x", i32 3, i32 0, i32 7, i32 4}
!38 = !{i32 6, !"y", i32 3, i32 4, i32 7, i32 9}
!39 = !{i32 1, float ()* @main, !40}
!40 = !{!41}
!41 = !{i32 1, !42, !2}
!42 = !{i32 4, !"SV_Target", i32 7, i32 9}
!43 = !{float ()* @main, !"main", null, !44, null}
!44 = !{null, null, !45, null}
!45 = !{!46}
!46 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!47 = !{float ()* @main, i32 0, i1 false}
!48 = !{i32 144}
!49 = !{i32 -1}
!50 = !DILocation(line: 17, column: 3, scope: !4)
!51 = !{!52, !52, i64 0}
!52 = !{!"float", !53, i64 0}
!53 = !{!"omnipotent char", !54, i64 0}
!54 = !{!"Simple C/C++ TBAA"}
!55 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "this", arg: 1, scope: !8, type: !56, flags: DIFlagArtificial | DIFlagObjectPointer)
!56 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 32, align: 32)
!57 = !DIExpression()
!58 = !DILocation(line: 0, scope: !8, inlinedAt: !59)
!59 = distinct !DILocation(line: 17, column: 3, scope: !4)
!60 = !DILocation(line: 9, column: 5, scope: !8, inlinedAt: !59)
!61 = !{!62, !62, i64 0}
!62 = !{!"int", !53, i64 0}
!63 = !DILocation(line: 9, column: 18, scope: !8, inlinedAt: !59)
!64 = !{!65}
!65 = distinct !{!65, !66, !"\01?GlobalSet@@YAXAIAMM@Z: %x"}
!66 = distinct !{!66, !"\01?GlobalSet@@YAXAIAMM@Z"}
!67 = !DILocation(line: 3, column: 5, scope: !18, inlinedAt: !68)
!68 = distinct !DILocation(line: 9, column: 5, scope: !8, inlinedAt: !59)
!69 = !DILocation(line: 10, column: 18, scope: !8, inlinedAt: !59)
!70 = !DILocation(line: 10, column: 5, scope: !8, inlinedAt: !59)
!71 = !{!72}
!72 = distinct !{!72, !73, !"\01?GlobalSet@@YAXAIAMM@Z: %x"}
!73 = distinct !{!73, !"\01?GlobalSet@@YAXAIAMM@Z"}
!74 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "x", arg: 1, scope: !18, file: !1, line: 2, type: !7)
!75 = !DILocation(line: 2, column: 28, scope: !18, inlinedAt: !76)
!76 = distinct !DILocation(line: 10, column: 5, scope: !8, inlinedAt: !59)
!77 = !DILocation(line: 3, column: 5, scope: !18, inlinedAt: !76)
!78 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "val", arg: 2, scope: !18, file: !1, line: 2, type: !7)
!79 = !DILocation(line: 2, column: 37, scope: !18, inlinedAt: !76)
!80 = !DILocation(line: 2, column: 37, scope: !18, inlinedAt: !68)
!81 = !DILocation(line: 2, column: 28, scope: !18, inlinedAt: !68)
!82 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "val", arg: 2, scope: !8, file: !1, line: 8, type: !7)
!83 = !DILocation(line: 8, column: 18, scope: !8, inlinedAt: !59)
!84 = !DILocation(line: 18, column: 3, scope: !4)