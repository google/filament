; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: Function: mainNS: error: Opcode LinAlgMatrixAccumulate not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixLength not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLength
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgFillMatrix not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgFillMatrix
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixLength not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLength
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixGetCoordinate not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixGetCoordinate
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixGetElement not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixGetElement
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixSetElement not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixSetElement
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixLoadFromMemory not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromMemory
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixMultiply not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixMultiplyAccumulate not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiplyAccumulate
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixStoreToDescriptor not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixStoreToMemory not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToMemory
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgCopyConvertMatrix not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix
; CHECK-NEXT: Function: mainNS: error: Opcode LinAlgMatrixAccumulateToMemory not valid in shader model lib_6_10(node).
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToMemory
; CHECK-NEXT: Function: mainNS: error: Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.
; CHECK-NEXT: Function: mainNS: error: Function uses features incompatible with the shader stage (node) of the entry function.
; CHECK-NEXT: Validation failed.

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M16N16U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U0S1 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U2S1 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U1S1 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

@"\01?InBuf@@3UByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4
@"\01?OutBuf@@3URWByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4
@"\01?GSMemory@@3PA$ui8_4pk@A" = external addrspace(3) global [1024 x i32], align 4

; Function Attrs: nounwind
define void @mainNS() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?InBuf@@3UByteAddressBuffer@@A", align 4, !noalias !21
  %2 = load %dx.types.Handle, %dx.types.Handle* @"\01?OutBuf@@3URWByteAddressBuffer@@A", align 4
  %3 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %5 = call %dx.types.LinAlgMatrixC8M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S0(i32 -2147483634, %dx.types.Handle %4, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %6 = call <16 x half> @dx.op.linAlgMatVecMul.v16f16.mC8M16N16U0S0.v16f16(i32 -2147483623, %dx.types.LinAlgMatrixC8M16N16U0S0 %5, i1 true, <16 x half> zeroinitializer, i32 8)  ; LinAlgMatVecMul(matrix,isOutputSigned,inputVector,interpretation)
  %7 = call <16 x half> @dx.op.linAlgMatVecMulAdd.v16f16.mC8M16N16U0S0.v16f16.v16f16(i32 -2147483622, %dx.types.LinAlgMatrixC8M16N16U0S0 %5, i1 true, <16 x half> %6, i32 8, <16 x half> %6)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)
  %8 = call <4 x i32> @dx.op.linAlgConvert.v4i32.v16i32(i32 -2147483618, <16 x i32> zeroinitializer, i32 5, i32 21)  ; LinAlgConvert(inputVector,inputInterpretation,outputInterpretation)
  %9 = call %dx.types.LinAlgMatrixC8M16N16U2S0 @dx.op.linAlgMatrixOuterProduct.mC8M16N16U2S0.v16f16.v16f16(i32 -2147483619, <16 x half> %7, <16 x half> %7)  ; LinAlgMatrixOuterProduct(vectorA,vectorB)
  %10 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %2)  ; CreateHandleForLib(Resource)
  %11 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %10, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M16N16U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M16N16U2S0 %9, %dx.types.Handle %11, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)
  %12 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %2)  ; CreateHandleForLib(Resource)
  %13 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %12, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgVectorAccumulateToDescriptor.v16f16(i32 -2147483617, %dx.types.Handle %13, i32 0, i32 64, <16 x half> %7)  ; LinAlgVectorAccumulateToDescriptor(handle,offset,align,vector)
  %14 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %15 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %14, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %16 = call %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S1(i32 -2147483634, %dx.types.Handle %15, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %17 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %18 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %17, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %19 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U2S1(i32 -2147483634, %dx.types.Handle %18, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %20 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixAccumulate.mC8M16N16U2S1.mC8M16N16U2S1.mC8M16N16U0S1(i32 -2147483624, %dx.types.LinAlgMatrixC8M16N16U2S1 %19, %dx.types.LinAlgMatrixC8M16N16U0S1 %16)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)
  %21 = call i32 @dx.op.linAlgMatrixLength.mC8M16N16U2S1(i32 -2147483632, %dx.types.LinAlgMatrixC8M16N16U2S1 %20)  ; LinAlgMatrixLength(matrix)
  %22 = call i32 @dx.op.linAlgMatrixQueryAccumulatorLayout(i32 -2147483626)  ; LinAlgMatrixQueryAccumulatorLayout()
  %23 = call %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgFillMatrix.mC8M16N16U0S1.f32(i32 -2147483636, float 0x40091EB860000000)  ; LinAlgFillMatrix(value)
  %24 = call i32 @dx.op.linAlgMatrixLength.mC8M16N16U0S1(i32 -2147483632, %dx.types.LinAlgMatrixC8M16N16U0S1 %23)  ; LinAlgMatrixLength(matrix)
  %25 = call <2 x i32> @dx.op.linAlgMatrixGetCoordinate.mC8M16N16U0S1(i32 -2147483631, %dx.types.LinAlgMatrixC8M16N16U0S1 %23, i32 %24)  ; LinAlgMatrixGetCoordinate(matrix,threadLocalIndex)
  %26 = call half @dx.op.linAlgMatrixGetElement.f16.mC8M16N16U0S1(i32 -2147483630, %dx.types.LinAlgMatrixC8M16N16U0S1 %23, i32 %24)  ; LinAlgMatrixGetElement(matrix,threadLocalIndex)
  %27 = fadd fast half %26, 0xH3CEC
  %28 = call %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgMatrixSetElement.mC8M16N16U0S1.mC8M16N16U0S1.f16(i32 -2147483629, %dx.types.LinAlgMatrixC8M16N16U0S1 %23, i32 %24, half %27)  ; LinAlgMatrixSetElement(matrix,threadLocalIndex,value)
  %29 = call %dx.types.LinAlgMatrixC8M16N16U1S1 @dx.op.linAlgMatrixLoadFromMemory.mC8M16N16U1S1.i32(i32 -2147483633, i32 addrspace(3)* getelementptr inbounds ([1024 x i32], [1024 x i32] addrspace(3)* @"\01?GSMemory@@3PA$ui8_4pk@A", i32 0, i32 0), i32 0, i32 64, i32 0)  ; LinAlgMatrixLoadFromMemory(memory,offset,stride,layout)
  %30 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixMultiply.mC8M16N16U2S1.mC8M16N16U0S1.mC8M16N16U1S1(i32 -2147483625, %dx.types.LinAlgMatrixC8M16N16U0S1 %28, %dx.types.LinAlgMatrixC8M16N16U1S1 %29)  ; LinAlgMatrixMultiply(matrixA,matrixB)
  %31 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixMultiplyAccumulate.mC8M16N16U2S1.mC8M16N16U0S1.mC8M16N16U1S1.mC8M16N16U2S1(i32 -2147483637, %dx.types.LinAlgMatrixC8M16N16U0S1 %28, %dx.types.LinAlgMatrixC8M16N16U1S1 %29, %dx.types.LinAlgMatrixC8M16N16U2S1 %30)  ; LinAlgMatrixMultiplyAccumulate(matrixA,matrixB,matrixC)
  %32 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %2)  ; CreateHandleForLib(Resource)
  %33 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %32, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M16N16U2S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M16N16U2S1 %31, %dx.types.Handle %33, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)
  call void @dx.op.linAlgMatrixStoreToMemory.mC8M16N16U2S1.i32(i32 -2147483627, %dx.types.LinAlgMatrixC8M16N16U2S1 %31, i32 addrspace(3)* getelementptr inbounds ([1024 x i32], [1024 x i32] addrspace(3)* @"\01?GSMemory@@3PA$ui8_4pk@A", i32 0, i32 0), i32 0, i32 64, i32 0)  ; LinAlgMatrixStoreToMemory(matrix,memory,offset,stride,layout)
  %34 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgCopyConvertMatrix.mC8M16N16U2S1.mC8M16N16U1S1(i32 -2147483635, %dx.types.LinAlgMatrixC8M16N16U1S1 %29, i1 false)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)
  call void @dx.op.linAlgMatrixAccumulateToMemory.mC8M16N16U2S1.i32(i32 -2147483620, %dx.types.LinAlgMatrixC8M16N16U2S1 %34, i32 addrspace(3)* getelementptr inbounds ([1024 x i32], [1024 x i32] addrspace(3)* @"\01?GSMemory@@3PA$ui8_4pk@A", i32 0, i32 0), i32 8, i32 0, i32 64, i32 0)  ; LinAlgMatrixAccumulateToMemory(matrix,memory,targetType,offset,stride,layout)
  ret void
}

; Function Attrs: nounwind
declare i32 @dx.op.linAlgMatrixQueryAccumulatorLayout(i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare <16 x half> @dx.op.linAlgMatVecMul.v16f16.mC8M16N16U0S0.v16f16(i32, %dx.types.LinAlgMatrixC8M16N16U0S0, i1, <16 x half>, i32) #0

; Function Attrs: nounwind
declare <16 x half> @dx.op.linAlgMatVecMulAdd.v16f16.mC8M16N16U0S0.v16f16.v16f16(i32, %dx.types.LinAlgMatrixC8M16N16U0S0, i1, <16 x half>, i32, <16 x half>) #0

; Function Attrs: nounwind
declare <4 x i32> @dx.op.linAlgConvert.v4i32.v16i32(i32, <16 x i32>, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S0 @dx.op.linAlgMatrixOuterProduct.mC8M16N16U2S0.v16f16.v16f16(i32, <16 x half>, <16 x half>) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M16N16U2S0(i32, %dx.types.LinAlgMatrixC8M16N16U2S0, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgVectorAccumulateToDescriptor.v16f16(i32, %dx.types.Handle, i32, i32, <16 x half>) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S1(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U2S1(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixAccumulate.mC8M16N16U2S1.mC8M16N16U2S1.mC8M16N16U0S1(i32, %dx.types.LinAlgMatrixC8M16N16U2S1, %dx.types.LinAlgMatrixC8M16N16U0S1) #0

; Function Attrs: nounwind
declare i32 @dx.op.linAlgMatrixLength.mC8M16N16U2S1(i32, %dx.types.LinAlgMatrixC8M16N16U2S1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgFillMatrix.mC8M16N16U0S1.f32(i32, float) #0

; Function Attrs: nounwind
declare i32 @dx.op.linAlgMatrixLength.mC8M16N16U0S1(i32, %dx.types.LinAlgMatrixC8M16N16U0S1) #0

; Function Attrs: nounwind
declare <2 x i32> @dx.op.linAlgMatrixGetCoordinate.mC8M16N16U0S1(i32, %dx.types.LinAlgMatrixC8M16N16U0S1, i32) #0

; Function Attrs: nounwind
declare half @dx.op.linAlgMatrixGetElement.f16.mC8M16N16U0S1(i32, %dx.types.LinAlgMatrixC8M16N16U0S1, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgMatrixSetElement.mC8M16N16U0S1.mC8M16N16U0S1.f16(i32, %dx.types.LinAlgMatrixC8M16N16U0S1, i32, half) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U1S1 @dx.op.linAlgMatrixLoadFromMemory.mC8M16N16U1S1.i32(i32, i32 addrspace(3)*, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixMultiply.mC8M16N16U2S1.mC8M16N16U0S1.mC8M16N16U1S1(i32, %dx.types.LinAlgMatrixC8M16N16U0S1, %dx.types.LinAlgMatrixC8M16N16U1S1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixMultiplyAccumulate.mC8M16N16U2S1.mC8M16N16U0S1.mC8M16N16U1S1.mC8M16N16U2S1(i32, %dx.types.LinAlgMatrixC8M16N16U0S1, %dx.types.LinAlgMatrixC8M16N16U1S1, %dx.types.LinAlgMatrixC8M16N16U2S1) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixStoreToDescriptor.mC8M16N16U2S1(i32, %dx.types.LinAlgMatrixC8M16N16U2S1, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixStoreToMemory.mC8M16N16U2S1.i32(i32, %dx.types.LinAlgMatrixC8M16N16U2S1, i32 addrspace(3)*, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgCopyConvertMatrix.mC8M16N16U2S1.mC8M16N16U1S1(i32, %dx.types.LinAlgMatrixC8M16N16U1S1, i1) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixAccumulateToMemory.mC8M16N16U2S1.i32(i32, %dx.types.LinAlgMatrixC8M16N16U2S1, i32 addrspace(3)*, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!dx.targetTypes = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}
!dx.version = !{!6}
!dx.valver = !{!6}
!dx.shaderModel = !{!7}
!dx.resources = !{!8}
!dx.entryPoints = !{!13, !15}

!0 = !{%dx.types.LinAlgMatrixC8M16N16U0S0 undef, i32 8, i32 16, i32 16, i32 0, i32 0}
!1 = !{%dx.types.LinAlgMatrixC8M16N16U2S0 undef, i32 8, i32 16, i32 16, i32 2, i32 0}
!2 = !{%dx.types.LinAlgMatrixC8M16N16U0S1 undef, i32 8, i32 16, i32 16, i32 0, i32 1}
!3 = !{%dx.types.LinAlgMatrixC8M16N16U2S1 undef, i32 8, i32 16, i32 16, i32 2, i32 1}
!4 = !{%dx.types.LinAlgMatrixC8M16N16U1S1 undef, i32 8, i32 16, i32 16, i32 1, i32 1}
!5 = !{!"dxc(private) 1.9.0.15449 (main, 4b537fd7a-dirty)"}
!6 = !{i32 1, i32 10}
!7 = !{!"lib", i32 6, i32 10}
!8 = !{!9, !11, null, null}
!9 = !{!10}
!10 = !{i32 0, %struct.ByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?InBuf@@3UByteAddressBuffer@@A" to %struct.ByteAddressBuffer*), !"InBuf", i32 -1, i32 -1, i32 1, i32 11, i32 0, null}
!11 = !{!12}
!12 = !{i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?OutBuf@@3URWByteAddressBuffer@@A" to %struct.RWByteAddressBuffer*), !"OutBuf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!13 = !{null, !"", null, !8, !14}
!14 = !{i32 0, i64 8598323248}
!15 = !{void ()* @mainNS, !"mainNS", null, null, !16}
!16 = !{i32 8, i32 15, i32 13, i32 1, i32 15, !17, i32 16, i32 -1, i32 18, !18, i32 4, !19, i32 5, !20}
!17 = !{!"mainNS", i32 0}
!18 = !{i32 8, i32 1, i32 1}
!19 = !{i32 64, i32 2, i32 2}
!20 = !{i32 0}
!21 = !{!22}
!22 = distinct !{!22, !23, !"\01??$Load@$0IA@@?$Matrix@$07$0BA@$0BA@$01$00@linalg@dx@@SA?AV012@UByteAddressBuffer@@IIW4MatrixLayoutEnum@MatrixLayout@12@@Z: %agg.result"}
!23 = distinct !{!23, !"\01??$Load@$0IA@@?$Matrix@$07$0BA@$0BA@$01$00@linalg@dx@@SA?AV012@UByteAddressBuffer@@IIW4MatrixLayoutEnum@MatrixLayout@12@@Z"}

