; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: Function: main: error: DXIL intrinsic overload must be valid.
; CHECK: note: at '%22 = call i64 @dx.op.tertiary.i64(i32 50, i64 %19, i64 %20, i64 %20)' in block '#0' of function 'main'.
; CHECK: Function: main: error: DXIL intrinsic overload must be valid.
; CHECK: note: at '%23 = call i64 @dx.op.tertiary.i64(i32 51, i64 %19, i64 %20, i64 %20)' in block '#0' of function 'main'.
; CHECK: Function: main: error: DXIL intrinsic overload must be valid.
; CHECK: note: at '%24 = call i64 @dx.op.tertiary.i64(i32 52, i64 %19, i64 %20, i64 %20)' in block '#0' of function 'main'.
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%"class.RWStructuredBuffer<SMsad4>" = type { %struct.SMsad4 }
%struct.SMsad4 = type { i32, <2 x i32>, <4 x i32>, <4 x i32> }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %2 = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)  ; FlattenedThreadIdInGroup()
  %3 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %1, i32 %2, i32 0)  ; BufferLoad(srv,index,wot)
  %4 = extractvalue %dx.types.ResRet.i32 %3, 0
  %5 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %1, i32 %2, i32 4)  ; BufferLoad(srv,index,wot)
  %6 = extractvalue %dx.types.ResRet.i32 %5, 0
  %7 = extractvalue %dx.types.ResRet.i32 %5, 1
  %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %1, i32 %2, i32 12)  ; BufferLoad(srv,index,wot)
  %9 = extractvalue %dx.types.ResRet.i32 %8, 0
  %10 = extractvalue %dx.types.ResRet.i32 %8, 1
  %11 = extractvalue %dx.types.ResRet.i32 %8, 2
  %12 = extractvalue %dx.types.ResRet.i32 %8, 3
  %13 = lshr i32 %6, 8
  %14 = call i32 @dx.op.quaternary.i32(i32 53, i32 8, i32 24, i32 %7, i32 %13)  ; Bfi(width,offset,value,replacedValue)
  %15 = lshr i32 %6, 16
  %16 = call i32 @dx.op.quaternary.i32(i32 53, i32 16, i32 16, i32 %7, i32 %15)  ; Bfi(width,offset,value,replacedValue)
  %17 = lshr i32 %6, 24
  %18 = call i32 @dx.op.quaternary.i32(i32 53, i32 24, i32 8, i32 %7, i32 %17)  ; Bfi(width,offset,value,replacedValue)
  %19 = zext i32 %4 to i64
  %20 = zext i32 %14 to i64
  %21 = zext i32 %10 to i64
  
  %22 = call i64 @dx.op.tertiary.i64(i32 50, i64 %19, i64 %20, i64 %20)  ; Msad(a,b,c)
  %23 = call i64 @dx.op.tertiary.i64(i32 51, i64 %19, i64 %20, i64 %20)  ; Ubfe(a,b,c)
  %24 = call i64 @dx.op.tertiary.i64(i32 52, i64 %19, i64 %20, i64 %20)  ; Ibfe(a,b,c)

  %25 = trunc i64 %22 to i32
  %26 = trunc i64 %23 to i32
  %27 = trunc i64 %24 to i32
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 %2, i32 0, i32 %25, i32 undef, i32 undef, i32 undef, i8 1)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 %2, i32 4, i32 %26, i32 %7, i32 undef, i32 undef, i8 3)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %1, i32 %2, i32 12, i32 %27, i32 %10, i32 %11, i32 %12, i8 15)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.quaternary.i32(i32, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare i64 @dx.op.tertiary.i64(i32, i64, i64, i64) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.entryPoints = !{!8}

!0 = !{!"dxc(private) 1.8.0.14883 (main, e50f599ff30-dirty)"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 9}
!3 = !{!"cs", i32 6, i32 0}
!4 = !{null, !5, null, null}
!5 = !{!6}
!6 = !{i32 0, %"class.RWStructuredBuffer<SMsad4>"* undef, !"", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !7}
!7 = !{i32 1, i32 44}
!8 = !{void ()* @main, !"main", null, !4, !9}
!9 = !{i32 0, i64 144, i32 4, !10}
!10 = !{i32 8, i32 8, i32 1}

