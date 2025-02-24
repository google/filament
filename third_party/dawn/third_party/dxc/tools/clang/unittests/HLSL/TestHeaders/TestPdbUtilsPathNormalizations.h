#pragma once

static const llvm::StringRef kTestPdbUtilsPathNormalizationsIR = R"x(
  target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
  target triple = "dxil-ms-dx"

  define void @main() {
  entry:
    call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00), !dbg !30 ; line:2 col:28  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
    ret void, !dbg !30 ; line:2 col:28
  }

  ; Function Attrs: nounwind
  declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

  attributes #0 = { nounwind }

  !llvm.dbg.cu = !{!0}
  !llvm.module.flags = !{!10, !11}
  !llvm.ident = !{!12}
  !dx.source.contents = !{!13, !14}
  !dx.source.defines = !{!2}
  !dx.source.mainFileName = !{!15}
  !dx.source.args = !{!16}
  !dx.version = !{!17}
  !dx.valver = !{!18}
  !dx.shaderModel = !{!19}
  !dx.typeAnnotations = !{!20}
  !dx.viewIdState = !{!23}
  !dx.entryPoints = !{!24}

  !0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "dxc(private) 1.7.0.4135 (pdb_header_fix, 24cf4a146)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3)
  !1 = !DIFile(filename: "<MAIN_FILE>", directory: "")
  !2 = !{}
  !3 = !{!4, !8}
  !4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 2, type: !5, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
  !5 = !DISubroutineType(types: !6)
  !6 = !{!7}
  !7 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
  !8 = !DISubprogram(name: "foo", linkageName: "\01?foo@@YAMXZ", scope: !9, file: !9, line: 1, type: !5, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false)
  !9 = !DIFile(filename: "<INCLUDE_FILE>", directory: "")
  !10 = !{i32 2, !"Dwarf Version", i32 4}
  !11 = !{i32 2, !"Debug Info Version", i32 3}
  !12 = !{!"dxc(private) 1.7.0.4135 (pdb_header_fix, 24cf4a146)"}
  !13 = !{!"<MAIN_FILE>", !"#include \22include.h\22\0D\0Afloat main() : SV_Target { return foo(); }\0D\0A"}
  !14 = !{!"<INCLUDE_FILE>", !"float foo() {\0D\0A  return 0;\0D\0A}\0D\0A"}
  !15 = !{!"<MAIN_FILE>"}
  !16 = !{!"-E", !"main", !"-T", !"ps_6_0", !"/Zi", !"-Qembed_debug"}
  !17 = !{i32 1, i32 0}
  !18 = !{i32 1, i32 8}
  !19 = !{!"ps", i32 6, i32 0}
  !20 = !{i32 1, void ()* @main, !21}
  !21 = !{!22}
  !22 = !{i32 0, !2, !2}
  !23 = !{[2 x i32] [i32 0, i32 1]}
  !24 = !{void ()* @main, !"main", !25, null, null}
  !25 = !{null, !26, null}
  !26 = !{!27}
  !27 = !{i32 0, !"SV_Target", i8 9, i8 16, !28, i8 0, i32 1, i8 1, i32 0, i8 0, !29}
  !28 = !{i32 0}
  !29 = !{i32 3, i32 1}
  !30 = !DILocation(line: 2, column: 28, scope: !4)
)x";
