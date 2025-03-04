; RUN: %dxopt %s -hlsl-passes-resume -dxil-fix-array-init -S | FileCheck %s

; The pass should not perform an out of bounds access when trying to determine
; which elements of an array are coverd by stores.
; The store instructions may have out of bounds accesses, including to negative
; indices.  In these cases, ignore those stores.  They are undefined behaviour
; anyway, and the best thing to do with them in this pass is nothing.
; If the original HLSL code used a literal -1 for the array index, the program
; is rejected at an earlier stage of compilation.

; Issue: #6824

; Original HLSL:

; groupshared float4 w;
;
; [numthreads(1, 1, 1)]
; void b() {
;   int i = -1;
;   w[i] = 0;
; }

; Check that the store instruction remains, and the compiler does not crash.

; CHECK: define void @b
; CHECK: store float 0
; CHECK-SAME: , i32 0, i32 -1)
; CHECK-NEXT: ret void

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

@"\01?w@@3V?$vector@M$03@@A.v" = addrspace(3) global [4 x float] undef, align 4

; Function Attrs: nounwind
define void @b() #0 {
entry:
  store float 0.000000e+00, float addrspace(3)* getelementptr inbounds ([4 x float], [4 x float] addrspace(3)* @"\01?w@@3V?$vector@M$03@@A.v", i32 0, i32 -1), !dbg !13, !tbaa !17 ; line:6 col:8
  ret void, !dbg !21 ; line:7 col:1
}

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.4640 (issue-785, 45018c752d)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{i32 1, void ()* @b, !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{void ()* @b, !"b", null, null, !11}
!11 = !{i32 4, !12}
!12 = !{i32 1, i32 1, i32 1}
!13 = !DILocation(line: 6, column: 8, scope: !14)
!14 = !DISubprogram(name: "b", scope: !15, file: !15, line: 4, type: !16, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @b)
!15 = !DIFile(filename: "a.hlsl", directory: "")
!16 = !DISubroutineType(types: !9)
!17 = !{!18, !18, i64 0}
!18 = !{!"float", !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = !DILocation(line: 7, column: 1, scope: !14)
