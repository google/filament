; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: Function: mainVS: error: Thread Group Shared Memory not supported from non-compute entry points.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromMemory
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixAccumulate not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulate
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixLength not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLength
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgFillMatrix not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgFillMatrix
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixLength not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLength
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixGetCoordinate not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixGetCoordinate
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixGetElement not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixGetElement
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixSetElement not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixSetElement
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixLoadFromMemory not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromMemory
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixMultiply not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixMultiplyAccumulate not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiplyAccumulate
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixStoreToDescriptor not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixStoreToMemory not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToMemory
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgCopyConvertMatrix not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix
; CHECK-NEXT: Function: mainVS: error: Opcode LinAlgMatrixAccumulateToMemory not valid in shader model vs_6_10.
; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToMemory
; CHECK-NEXT: Function: mainVS: error: Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.
; CHECK-NEXT: Function: mainVS: error: Function uses features incompatible with the shader stage (vs) of the entry function.
; CHECK-NEXT: Function: mainVS: error: Function requires a visible group, but is called from a shader without one.
; CHECK-NEXT: Validation failed.

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M16N16U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U0S1 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U2S1 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U1S1 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

@"\01?GSMemory@@3PA$ui8_4pk@A" = external addrspace(3) global [1024 x i32], align 4

define void @mainVS() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %4 = call %dx.types.LinAlgMatrixC8M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S0(i32 -2147483634, %dx.types.Handle %3, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %5 = call <16 x half> @dx.op.linAlgMatVecMul.v16f16.mC8M16N16U0S0.v16f16(i32 -2147483623, %dx.types.LinAlgMatrixC8M16N16U0S0 %4, i1 true, <16 x half> zeroinitializer, i32 8)  ; LinAlgMatVecMul(matrix,isOutputSigned,inputVector,interpretation)
  %6 = call <16 x half> @dx.op.linAlgMatVecMulAdd.v16f16.mC8M16N16U0S0.v16f16.v16f16(i32 -2147483622, %dx.types.LinAlgMatrixC8M16N16U0S0 %4, i1 true, <16 x half> %5, i32 8, <16 x half> %5)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)
  %7 = call <4 x i32> @dx.op.linAlgConvert.v4i32.v16i32(i32 -2147483618, <16 x i32> zeroinitializer, i32 5, i32 21)  ; LinAlgConvert(inputVector,inputInterpretation,outputInterpretation)
  %8 = call %dx.types.LinAlgMatrixC8M16N16U2S0 @dx.op.linAlgMatrixOuterProduct.mC8M16N16U2S0.v16f16.v16f16(i32 -2147483619, <16 x half> %6, <16 x half> %6)  ; LinAlgMatrixOuterProduct(vectorA,vectorB)
  %9 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M16N16U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M16N16U2S0 %8, %dx.types.Handle %9, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)
  %10 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgVectorAccumulateToDescriptor.v16f16(i32 -2147483617, %dx.types.Handle %10, i32 0, i32 64, <16 x half> %6)  ; LinAlgVectorAccumulateToDescriptor(handle,offset,align,vector)
  %11 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %12 = call %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S1(i32 -2147483634, %dx.types.Handle %11, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %13 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %14 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U2S1(i32 -2147483634, %dx.types.Handle %13, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %15 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixAccumulate.mC8M16N16U2S1.mC8M16N16U2S1.mC8M16N16U0S1(i32 -2147483624, %dx.types.LinAlgMatrixC8M16N16U2S1 %14, %dx.types.LinAlgMatrixC8M16N16U0S1 %12)  ; LinAlgMatrixAccumulate(matrixLHS,matrixRHS)
  %16 = call i32 @dx.op.linAlgMatrixLength.mC8M16N16U2S1(i32 -2147483632, %dx.types.LinAlgMatrixC8M16N16U2S1 %15)  ; LinAlgMatrixLength(matrix)
  %17 = call i32 @dx.op.linAlgMatrixQueryAccumulatorLayout(i32 -2147483626)  ; LinAlgMatrixQueryAccumulatorLayout()
  %18 = call %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgFillMatrix.mC8M16N16U0S1.f32(i32 -2147483636, float 0x40091EB860000000)  ; LinAlgFillMatrix(value)
  %19 = call i32 @dx.op.linAlgMatrixLength.mC8M16N16U0S1(i32 -2147483632, %dx.types.LinAlgMatrixC8M16N16U0S1 %18)  ; LinAlgMatrixLength(matrix)
  %20 = call <2 x i32> @dx.op.linAlgMatrixGetCoordinate.mC8M16N16U0S1(i32 -2147483631, %dx.types.LinAlgMatrixC8M16N16U0S1 %18, i32 %19)  ; LinAlgMatrixGetCoordinate(matrix,threadLocalIndex)
  %21 = call half @dx.op.linAlgMatrixGetElement.f16.mC8M16N16U0S1(i32 -2147483630, %dx.types.LinAlgMatrixC8M16N16U0S1 %18, i32 %19)  ; LinAlgMatrixGetElement(matrix,threadLocalIndex)
  %22 = fadd fast half %21, 0xH3CEC
  %23 = call %dx.types.LinAlgMatrixC8M16N16U0S1 @dx.op.linAlgMatrixSetElement.mC8M16N16U0S1.mC8M16N16U0S1.f16(i32 -2147483629, %dx.types.LinAlgMatrixC8M16N16U0S1 %18, i32 %19, half %22)  ; LinAlgMatrixSetElement(matrix,threadLocalIndex,value)
  %24 = call %dx.types.LinAlgMatrixC8M16N16U1S1 @dx.op.linAlgMatrixLoadFromMemory.mC8M16N16U1S1.i32(i32 -2147483633, i32 addrspace(3)* getelementptr inbounds ([1024 x i32], [1024 x i32] addrspace(3)* @"\01?GSMemory@@3PA$ui8_4pk@A", i32 0, i32 0), i32 0, i32 64, i32 0)  ; LinAlgMatrixLoadFromMemory(memory,offset,stride,layout)
  %25 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixMultiply.mC8M16N16U2S1.mC8M16N16U0S1.mC8M16N16U1S1(i32 -2147483625, %dx.types.LinAlgMatrixC8M16N16U0S1 %23, %dx.types.LinAlgMatrixC8M16N16U1S1 %24)  ; LinAlgMatrixMultiply(matrixA,matrixB)
  %26 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgMatrixMultiplyAccumulate.mC8M16N16U2S1.mC8M16N16U0S1.mC8M16N16U1S1.mC8M16N16U2S1(i32 -2147483637, %dx.types.LinAlgMatrixC8M16N16U0S1 %23, %dx.types.LinAlgMatrixC8M16N16U1S1 %24, %dx.types.LinAlgMatrixC8M16N16U2S1 %25)  ; LinAlgMatrixMultiplyAccumulate(matrixA,matrixB,matrixC)
  %27 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M16N16U2S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M16N16U2S1 %26, %dx.types.Handle %27, i32 0, i32 64, i32 0, i32 128)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)
  call void @dx.op.linAlgMatrixStoreToMemory.mC8M16N16U2S1.i32(i32 -2147483627, %dx.types.LinAlgMatrixC8M16N16U2S1 %26, i32 addrspace(3)* getelementptr inbounds ([1024 x i32], [1024 x i32] addrspace(3)* @"\01?GSMemory@@3PA$ui8_4pk@A", i32 0, i32 0), i32 0, i32 64, i32 0)  ; LinAlgMatrixStoreToMemory(matrix,memory,offset,stride,layout)
  %28 = call %dx.types.LinAlgMatrixC8M16N16U2S1 @dx.op.linAlgCopyConvertMatrix.mC8M16N16U2S1.mC8M16N16U1S1(i32 -2147483635, %dx.types.LinAlgMatrixC8M16N16U1S1 %24, i1 false)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)
  call void @dx.op.linAlgMatrixAccumulateToMemory.mC8M16N16U2S1.i32(i32 -2147483620, %dx.types.LinAlgMatrixC8M16N16U2S1 %28, i32 addrspace(3)* getelementptr inbounds ([1024 x i32], [1024 x i32] addrspace(3)* @"\01?GSMemory@@3PA$ui8_4pk@A", i32 0, i32 0), i32 8, i32 0, i32 64, i32 0)  ; LinAlgMatrixAccumulateToMemory(matrix,memory,targetType,offset,stride,layout)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 1.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

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
declare i32 @dx.op.linAlgMatrixQueryAccumulatorLayout(i32) #0

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

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.targetTypes = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}
!dx.version = !{!6}
!dx.valver = !{!6}
!dx.shaderModel = !{!7}
!dx.resources = !{!8}
!dx.viewIdState = !{!13}
!dx.entryPoints = !{!14}

!0 = !{%dx.types.LinAlgMatrixC8M16N16U0S0 undef, i32 8, i32 16, i32 16, i32 0, i32 0}
!1 = !{%dx.types.LinAlgMatrixC8M16N16U2S0 undef, i32 8, i32 16, i32 16, i32 2, i32 0}
!2 = !{%dx.types.LinAlgMatrixC8M16N16U0S1 undef, i32 8, i32 16, i32 16, i32 0, i32 1}
!3 = !{%dx.types.LinAlgMatrixC8M16N16U2S1 undef, i32 8, i32 16, i32 16, i32 2, i32 1}
!4 = !{%dx.types.LinAlgMatrixC8M16N16U1S1 undef, i32 8, i32 16, i32 16, i32 1, i32 1}
!5 = !{!"dxc(private) 1.9.0.15449 (main, 4b537fd7a-dirty)"}
!6 = !{i32 1, i32 10}
!7 = !{!"vs", i32 6, i32 10}
!8 = !{!9, !11, null, null}
!9 = !{!10}
!10 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!11 = !{!12}
!12 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!13 = !{[3 x i32] [i32 1, i32 4, i32 0]}
!14 = !{void ()* @mainVS, !"mainVS", !15, !8, !22}
!15 = !{!16, !19, null}
!16 = !{!17}
!17 = !{i32 0, !"SV_VertexID", i8 5, i8 1, !18, i8 0, i32 1, i8 1, i32 0, i8 0, null}
!18 = !{i32 0}
!19 = !{!20}
!20 = !{i32 0, !"OUT", i8 9, i8 0, !18, i8 2, i32 1, i8 4, i32 0, i8 0, !21}
!21 = !{i32 3, i32 15}
!22 = !{i32 0, i64 8598388784}

