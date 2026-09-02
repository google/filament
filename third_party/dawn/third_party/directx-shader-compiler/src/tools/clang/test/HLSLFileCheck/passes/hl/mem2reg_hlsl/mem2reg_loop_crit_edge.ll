; RUN: %opt %s -simplifycfg -reg2mem_hlsl -S | FileCheck %s

; Make sure loop variable isn't rewritten in the loop basic block
; CHECK-LABEL: loop.inner
; CHECK: load i32, i32* [[Addr:%.*]]
; CHECK-NOT: store i32 {{.*}}, i32* [[Addr]]
; CHECK-NOT: br{{.*}}%loop.inner
; CHECK: br
; CHECK: store i32 {{.*}}, i32* [[Addr]]
; CHECK: br label %loop.inner

; ModuleID = 'MyModule'
target triple = "dxil-ms-dx"

define void @main() {
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %2 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)
  %3 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 undef)
  %5 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 1, i32 undef)
  %7 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 2, i32 undef)
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 3, i32 undef)
  %9 = bitcast float %5 to i32
  %10 = bitcast float %6 to i32
  %11 = bitcast float %7 to i32
  %12 = bitcast float %8 to i32
  %13 = bitcast i32 0 to float
  br label %loop.outer

loop.outer:                                      ; preds = %49, %0
  %14 = phi i32 [ 0, %0 ], [ %45, %49 ]
  %15 = phi i32 [ %9, %0 ], [ %36, %49 ]
  %16 = phi i32 [ %10, %0 ], [ %37, %49 ]
  %17 = phi i32 [ %11, %0 ], [ %38, %49 ]
  %18 = phi i32 [ %12, %0 ], [ %39, %49 ]
  %19 = bitcast i32 0 to float
  br label %loop.inner

loop.inner:                                      ; preds = %43, %loop.outer
  %20 = phi i32 [ %15, %loop.outer ], [ %36, %43 ]
  %21 = phi i32 [ %16, %loop.outer ], [ %37, %43 ]
  %22 = phi i32 [ %17, %loop.outer ], [ %38, %43 ]
  %23 = phi i32 [ %18, %loop.outer ], [ %39, %43 ]
  %24 = bitcast i32 %21 to float
  %25 = bitcast i32 1065353216 to float
  %26 = fadd fast float %24, %25
  %27 = bitcast i32 %22 to float
  %28 = bitcast i32 1065353216 to float
  %29 = fadd fast float %27, %28
  %30 = bitcast i32 %23 to float
  %31 = bitcast i32 1065353216 to float
  %32 = fadd fast float %30, %31
  %33 = bitcast i32 %20 to float
  %34 = bitcast i32 1065353216 to float
  %35 = fadd fast float %33, %34
  %36 = bitcast float %26 to i32
  %37 = bitcast float %29 to i32
  %38 = bitcast float %32 to i32
  %39 = bitcast float %35 to i32
  %40 = icmp slt i32 %14, 3
  br i1 %40, label %41, label %42

; <label>:41                                      ; preds = %loop.inner
  br label %44

; <label>:42                                      ; preds = %loop.inner
  br label %43

; <label>:43                                      ; preds = %42
  br label %loop.inner

; <label>:44                                      ; preds = %41
  %45 = add i32 %14, 1
  %46 = icmp sge i32 %45, 3
  br i1 %46, label %47, label %48

; <label>:47                                      ; preds = %44
  br label %50

; <label>:48                                      ; preds = %44
  br label %49

; <label>:49                                      ; preds = %48
  br label %loop.outer

; <label>:50                                      ; preds = %47
  %51 = bitcast i32 -1073741824 to float
  %52 = bitcast i32 %21 to float
  %53 = fadd fast float %51, %52
  %54 = bitcast i32 -1073741824 to float
  %55 = bitcast i32 %22 to float
  %56 = fadd fast float %54, %55
  %57 = bitcast i32 -1073741824 to float
  %58 = bitcast i32 %23 to float
  %59 = fadd fast float %57, %58
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %53)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %56)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %59)
  %60 = bitcast i32 -1082130432 to float
  %61 = fmul fast float %2, %60
  call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float %1)
  call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float %61)
  call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2, float %3)
  call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3, float %4)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7}

!0 = !{!"Mesa version 23.2.0-devel (git-e2603275dc)"}
!1 = !{i32 1, i32 7}
!2 = !{!"vs", i32 6, i32 7}
!3 = !{i32 1, void ()* @main, !4}
!4 = !{!5}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{void ()* @main, !"main", !8, null, null}
!8 = !{!9, !15, null}
!9 = !{!10, !13}
!10 = !{i32 0, !"TEXCOORD", i8 9, i8 0, !11, i8 0, i32 1, i8 4, i32 0, i8 0, !12}
!11 = !{i32 0}
!12 = !{i32 3, i8 1}
!13 = !{i32 1, !"TEXCOORD", i8 9, i8 0, !14, i8 0, i32 1, i8 4, i32 1, i8 0, !12}
!14 = !{i32 1}
!15 = !{!16, !18}
!16 = !{i32 0, !"TEXCOORD", i8 9, i8 0, !11, i8 2, i32 1, i8 3, i32 0, i8 0, !17}
!17 = !{i32 3, i8 7}
!18 = !{i32 1, !"SV_Position", i8 9, i8 3, !11, i8 4, i32 1, i8 4, i32 1, i8 0, !19}
!19 = !{i32 3, i8 15}