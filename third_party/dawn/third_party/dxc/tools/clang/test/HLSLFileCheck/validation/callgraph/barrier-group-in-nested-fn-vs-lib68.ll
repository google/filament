; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; Make sure function compatibility checking is done for called functions.

; CHECK: Function: main: error: Entry function calls one or more functions using incompatible features.  See other errors for details.
; CHECK: Function: {{.*}}barrier_group{{.*}}: error: Function uses features incompatible with the shader stage (vs) of the entry function.
; CHECK: Function: {{.*}}barrier_group{{.*}}: error: Function requires a visible group, but is called from a shader without one.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RWBuffer<unsigned int>" = type { i32 }

@"\01?Buf@@3V?$RWBuffer@I@@A" = external constant %dx.types.Handle, align 4

; Function to call that does nothing, so is not to blame for conflict.
; Function Attrs: noinline nounwind
define void @"\01?write_value@@YAXI@Z"(i32 %value) #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?Buf@@3V?$RWBuffer@I@@A", align 4
  %2 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 4106, i32 261 })
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %3, i32 %value, i32 undef, i32 %value, i32 %value, i32 %value, i32 %value, i8 15)
  ret void
}

; Function that uses a barrier requiring visible group.
; Also calls write_value, but this function is to blame for group requirement.
; Function Attrs: noinline nounwind
define void @"\01?barrier_group@@YAXXZ"() #0 {
  call void @"\01?write_value@@YAXI@Z"(i32 1)
  call void @dx.op.barrierByMemoryType(i32 244, i32 2, i32 2)
  call void @"\01?write_value@@YAXI@Z"(i32 2)
  ret void
}

; Intermediate function that is not directly to blame for conflict.
; Function calls barrier_group, and write_value.
; Function Attrs: noinline nounwind
define void @"\01?intermediate@@YAXXZ"() #0 {
  call void @"\01?write_value@@YAXI@Z"(i32 3)
  call void @"\01?barrier_group@@YAXXZ"()
  call void @"\01?write_value@@YAXI@Z"(i32 4)
  ret void
}

; Function Attrs: nounwind
define void @main() #1 {
  call void @"\01?intermediate@@YAXXZ"()
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) #1

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryType(i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #4

attributes #0 = { noinline nounwind }
attributes #1 = { nounwind }
attributes #2 = { noduplicate nounwind }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!7}
!dx.entryPoints = !{!14, !16}

!0 = !{!"dxc(private) 1.8.0.4482 (val-compat-calls, 055b660e4)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %"class.RWBuffer<unsigned int>"* bitcast (%dx.types.Handle* @"\01?Buf@@3V?$RWBuffer@I@@A" to %"class.RWBuffer<unsigned int>"*), !"Buf", i32 0, i32 0, i32 1, i32 10, i1 false, i1 false, i1 false, !6}
!6 = !{i32 0, i32 5}
!7 = !{i32 1, void (i32)* @"\01?write_value@@YAXI@Z", !8, void ()* @"\01?barrier_group@@YAXXZ", !13, void ()* @"\01?intermediate@@YAXXZ", !13, void ()* @main, !13}
!8 = !{!9, !11}
!9 = !{i32 1, !10, !10}
!10 = !{}
!11 = !{i32 0, !12, !10}
!12 = !{i32 7, i32 5}
!13 = !{!9}
!14 = !{null, !"", null, !3, !15}
!15 = !{i32 0, i64 8589934592}
!16 = !{void ()* @main, !"main", null, null, !17}
!17 = !{i32 8, i32 1, i32 5, !18}
!18 = !{i32 0}