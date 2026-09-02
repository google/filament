; RUN: %dxopt %s -hlsl-passes-resume -hlsl-passes-pause -S | FileCheck %s

; Test round-trip metadata serialization for ResourceProperties type annotation.

; CHECK: %dx.types.ResourceProperties = type { i32, i32 }
; CHECK: !dx.typeAnnotations = !{![[TYPE_ANNOTATIONS:[0-9]+]]}
; CHECK: ![[TYPE_ANNOTATIONS]] = !{
; CHECK-SAME: void (%struct.RWByteAddressBuffer*)* @"\01?foo@@YAXURWByteAddressBuffer@@@Z", ![[FN_ANN:[0-9]+]]
; CHECK: ![[FN_ANN]] = !{!{{[0-9]+}}, ![[PARAM_ANN:[0-9]+]]}
; CHECK: ![[PARAM_ANN]] = !{i32 0, ![[FIELD_ANN:[0-9]+]], !{{[0-9]+}}}
; CHECK: ![[FIELD_ANN]] = !{
; CHECK-SAME: i32 10, %dx.types.ResourceProperties { i32 4107, i32 0 }

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RWByteAddressBuffer = type { i32 }
%ConstantBuffer = type opaque
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?OutBuff@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main() #0 {
entry:
  call void @"\01?foo@@YAXURWByteAddressBuffer@@@Z"(%struct.RWByteAddressBuffer* @"\01?OutBuff@@3URWByteAddressBuffer@@A")
  ret void
}

; Function Attrs: alwaysinline nounwind
define internal void @"\01?foo@@YAXURWByteAddressBuffer@@@Z"(%struct.RWByteAddressBuffer* %Buf) #1 {
entry:
  %0 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* %Buf
  %1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %0)
  %2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %2, i32 0, i32 42)
  ret void
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #2

attributes #0 = { nounwind }
attributes #1 = { alwaysinline nounwind }
attributes #2 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!13}
!dx.fnprops = !{!19}
!dx.options = !{!20, !21}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.9.0.15350 (main, 348fc7832)"}
!3 = !{i32 1, i32 8}
!4 = !{i32 1, i32 10}
!5 = !{!"cs", i32 6, i32 8}
!6 = !{i32 1, void ()* @main, !7, void (%struct.RWByteAddressBuffer*)* @"\01?foo@@YAXURWByteAddressBuffer@@@Z", !10}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{!8, !11}
!11 = !{i32 0, !12, !9}
!12 = !{i32 10, %dx.types.ResourceProperties { i32 4107, i32 0 }}
!13 = !{void ()* @main, !"main", null, !14, null}
!14 = !{null, !15, !17, null}
!15 = !{!16}
!16 = !{i32 0, %struct.RWByteAddressBuffer* @"\01?OutBuff@@3URWByteAddressBuffer@@A", !"OutBuff", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!17 = !{!18}
!18 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!19 = !{void ()* @main, i32 5, i32 8, i32 1, i32 1}
!20 = !{i32 64}
!21 = !{i32 -1}
