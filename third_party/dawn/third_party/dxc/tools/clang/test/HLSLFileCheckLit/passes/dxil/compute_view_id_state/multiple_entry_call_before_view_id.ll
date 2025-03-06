; RUN: opt -viewid-state -analyze -S %s 2>&1 | FileCheck %s

; CHECK: ViewId state:
; CHECK: Number of inputs: 2, outputs: 1, patchconst: 0
; CHECK: Outputs dependent on ViewId: {  }
; CHECK: Inputs contributing to computation of Outputs:
; CHECK: output 0 depends on inputs: { 0, 1 }

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: noinline nounwind readnone
define internal fastcc float @"\01?foo@@YAMM@Z"(float %a) #0 {
entry:
  %add = fadd float %a, 2.000000e+00, !dbg !23 ; line:3 col:12
  ret float %add, !dbg !27 ; line:3 col:4
}

define void @main() {
entry:
  %0 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef), !dbg !28 ; line:7 col:14
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef), !dbg !28 ; line:7 col:14
  %call = call fastcc float @"\01?foo@@YAMM@Z"(float %0), !dbg !30 ; line:7 col:10
  %call1 = call fastcc float @"\01?foo@@YAMM@Z"(float %1), !dbg !31 ; line:7 col:21
  %add = fadd fast float %call1, %call, !dbg !32 ; line:7 col:19
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %add), !dbg !33 ; line:7 col:3
  ret void, !dbg !33 ; line:7 col:3
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #2

attributes #0 = { noinline nounwind readnone }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!14}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.7.0.3808 (multiple_entry_uses, 7c47685d8-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 7}
!5 = !{!"ps", i32 6, i32 0}
!6 = !{i32 1, float (float)* @"\01?foo@@YAMM@Z", !7, void ()* @main, !12}
!7 = !{!8, !11}
!8 = !{i32 1, !9, !10}
!9 = !{i32 7, i32 9}
!10 = !{}
!11 = !{i32 0, !9, !10}
!12 = !{!13}
!13 = !{i32 0, !10, !10}
!14 = !{void ()* @main, !"main", !15, null, null}
!15 = !{!16, !20, null}
!16 = !{!17}
!17 = !{i32 0, !"A", i8 9, i8 0, !18, i8 2, i32 1, i8 2, i32 0, i8 0, !19}
!18 = !{i32 0}
!19 = !{i32 3, i32 3}
!20 = !{!21}
!21 = !{i32 0, !"SV_Target", i8 9, i8 16, !18, i8 0, i32 1, i8 1, i32 0, i8 0, !22}
!22 = !{i32 3, i32 1}
!23 = !DILocation(line: 3, column: 12, scope: !24)
!24 = !DISubprogram(name: "foo", scope: !25, file: !25, line: 2, type: !26, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, function: float (float)* @"\01?foo@@YAMM@Z")
!25 = !DIFile(filename: "hlsl.hlsl", directory: "")
!26 = !DISubroutineType(types: !10)
!27 = !DILocation(line: 3, column: 4, scope: !24)
!28 = !DILocation(line: 7, column: 14, scope: !29)
!29 = !DISubprogram(name: "main", scope: !25, file: !25, line: 6, type: !26, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false)
!30 = !DILocation(line: 7, column: 10, scope: !29)
!31 = !DILocation(line: 7, column: 21, scope: !29)
!32 = !DILocation(line: 7, column: 19, scope: !29)
!33 = !DILocation(line: 7, column: 3, scope: !29)
