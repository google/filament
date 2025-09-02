; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-9

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?input_vector_buffer@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@"\01?opa_input_buffer@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@"\01?matrix_buffer@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@"\01?bias_buffer@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4
@"\01?output_vector_buffer@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4

; Function Attrs: nounwind
define void @cs_main() #0 {
entry:
  ;CHECK-DAG: %[[MLD:[^ ]+]] = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?matrix_buffer@@3UByteAddressBuffer@@A"
  ;CHECK-DAG: %[[BLD:[^ ]+]] = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?bias_buffer@@3UByteAddressBuffer@@A"
  ;CHECK-DAG: %[[RWMLD0:[^ ]+]] = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A"
  %output_vector = alloca <4 x float>, align 4
  %tmp = bitcast <4 x float>* %output_vector to i8*, !dbg !21 ; line:14 col:5
  call void @llvm.lifetime.start(i64 16, i8* %tmp) #0, !dbg !21 ; line:14 col:5
  %tmp1 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?input_vector_buffer@@3UByteAddressBuffer@@A", !dbg !25 ; line:17 col:37
  %tmp2 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %tmp1), !dbg !25 ; line:17 col:37
  %tmp3 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp2, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer zeroinitializer), !dbg !25 ; line:17 col:37
  %tmp4 = call <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp3, i32 0), !dbg !25 ; line:17 col:37
  %tmp5 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?matrix_buffer@@3UByteAddressBuffer@@A", !dbg !26 ; line:33 col:5
  %tmp6 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %tmp5), !dbg !26 ; line:33 col:5
  %tmp7 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp6, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer zeroinitializer), !dbg !26 ; line:33 col:5

  ;CHECK: %[[MCH0:[^ ]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %[[MLD]]
  ;CHECK: %[[MAH0:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[MCH0]]
  ;CHECK: call <4 x float> @dx.op.matVecMul.v4f32.v4f32(i32 305, <4 x float> %{{[^ ]+}}, i1 false, i32 9, %dx.types.Handle %[[MAH0]], i32 0, i32 9, i32 4, i32 4, i32 0, i1 false, i32 64, i1 false) 
  call void @"dx.hl.op..void (i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32)"(i32 390, <4 x float>* %output_vector, i1 false, <4 x float> %tmp4, i1 false, i32 9, %dx.types.Handle %tmp7, i32 0, i32 9, i32 4, i32 4, i32 0, i1 false, i32 64), !dbg !26 ; line:33 col:5

  %tmp8 = load <4 x float>, <4 x float>* %output_vector, align 4, !dbg !27, !tbaa !28 ; line:37 col:35
  %tmp9 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?output_vector_buffer@@3URWByteAddressBuffer@@A", !dbg !31 ; line:37 col:5
  %tmp10 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp9), !dbg !31 ; line:37 col:5
  %tmp11 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp10, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !31 ; line:37 col:5
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32 277, %dx.types.Handle %tmp11, i32 0, <4 x float> %tmp8), !dbg !31 ; line:37 col:5
  %tmp12 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?matrix_buffer@@3UByteAddressBuffer@@A", !dbg !32 ; line:49 col:5
  %tmp13 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %tmp12), !dbg !32 ; line:49 col:5
  %tmp14 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp13, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer zeroinitializer), !dbg !32 ; line:49 col:5
  %tmp15 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?bias_buffer@@3UByteAddressBuffer@@A", !dbg !32 ; line:49 col:5
  %tmp16 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %tmp15), !dbg !32 ; line:49 col:5
  %tmp17 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp16, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer zeroinitializer), !dbg !32 ; line:49 col:5

  ;CHECK: %[[MCH1:[^ ]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %[[MLD]]
  ;CHECK: %[[MAH1:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[MCH1]]
  ;CHECK: %[[BCH1:[^ ]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %[[BLD]]
  ;CHECK: %[[BAH1:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[BCH1]]
  ;CHECK: call <4 x float> @dx.op.matVecMulAdd.v4f32.v4f32(i32 306, <4 x float> %{{[^ ]+}}, i1 false, i32 9, %dx.types.Handle %[[MAH1]], i32 0, i32 9, i32 4, i32 4, i32 0, i1 false, i32 64, %dx.types.Handle %[[BAH1]], i32 0, i32 9, i1 false)
  call void @"dx.hl.op..void (i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32, %dx.types.Handle, i32, i32)"(i32 391, <4 x float>* %output_vector, i1 false, <4 x float> %tmp4, i1 false, i32 9, %dx.types.Handle %tmp14, i32 0, i32 9, i32 4, i32 4, i32 0, i1 false, i32 64, %dx.types.Handle %tmp17, i32 0, i32 9), !dbg !32 ; line:49 col:5
  
  %tmp18 = load <4 x float>, <4 x float>* %output_vector, align 4, !dbg !33, !tbaa !28 ; line:54 col:38
  %tmp19 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?output_vector_buffer@@3URWByteAddressBuffer@@A", !dbg !34 ; line:54 col:5
  %tmp20 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp19), !dbg !34 ; line:54 col:5
  %tmp21 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp20, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !34 ; line:54 col:5
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32 277, %dx.types.Handle %tmp21, i32 1024, <4 x float> %tmp18), !dbg !34 ; line:54 col:5
  %tmp22 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?opa_input_buffer@@3UByteAddressBuffer@@A", !dbg !35 ; line:56 col:37
  %tmp23 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %tmp22), !dbg !35 ; line:56 col:37
  %tmp24 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp23, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer zeroinitializer), !dbg !35 ; line:56 col:37
  %tmp25 = call <8 x i32> @"dx.hl.op.ro.<8 x i32> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp24, i32 0), !dbg !35 ; line:56 col:37
  %tmp26 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?opa_input_buffer@@3UByteAddressBuffer@@A", !dbg !36 ; line:57 col:37
  %tmp27 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %tmp26), !dbg !36 ; line:57 col:37
  %tmp28 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp27, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer zeroinitializer), !dbg !36 ; line:57 col:37
  %tmp29 = call <8 x i32> @"dx.hl.op.ro.<8 x i32> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %tmp28, i32 128), !dbg !36 ; line:57 col:37
  %tmp30 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A", !dbg !37 ; line:67 col:5
  %tmp31 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp30), !dbg !37 ; line:67 col:5
  %tmp32 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp31, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !37 ; line:67 col:5

  ;CHECK: %[[RWMCH0:[^ ]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %[[RWMLD0]]
  ;CHECK: %[[RWMAH0:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[RWMCH0]]
  ;CHECK: call void @dx.op.outerProductAccumulate.v8i32.v8i32(i32 307, <8 x i32> %{{[^ ]+}}, <8 x i32> %{{[^ ]+}}, %dx.types.Handle %[[RWMAH0]], i32 0, i32 5, i32 3, i32 0)
  call void @"dx.hl.op..void (i32, <8 x i32>, <8 x i32>, %dx.types.Handle, i32, i32, i32, i32)"(i32 392, <8 x i32> %tmp25, <8 x i32> %tmp29, %dx.types.Handle %tmp32, i32 0, i32 5, i32 3, i32 0), !dbg !37 ; line:67 col:5

  
  %tmp33 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A", !dbg !38 ; line:77 col:5
  %tmp34 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %tmp33), !dbg !38 ; line:77 col:5
  %tmp35 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %tmp34, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !38 ; line:77 col:5

  ;CHECK: %[[RWMCH1:[^ ]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer(i32 160, %struct.RWByteAddressBuffer %[[RWMLD0]]
  ;CHECK: %[[RWMAH1:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[RWMCH1]]
  ;CHECK: call void @dx.op.vectorAccumulate.v8i32(i32 308, <8 x i32> %{{[^ ]+}}, %dx.types.Handle %[[RWMAH1]], i32 0)
  call void @"dx.hl.op..void (i32, <8 x i32>, %dx.types.Handle, i32)"(i32 393, <8 x i32> %tmp25, %dx.types.Handle %tmp35, i32 0), !dbg !38 ; line:77 col:5

  %tmp36 = bitcast <4 x float>* %output_vector to i8*, !dbg !39 ; line:79 col:1
  call void @llvm.lifetime.end(i64 16, i8* %tmp36) #0, !dbg !39 ; line:79 col:1
  ret void, !dbg !39 ; line:79 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind readonly
declare <4 x float> @"dx.hl.op.ro.<4 x float> (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32, %struct.ByteAddressBuffer) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer) #2

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32)"(i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <4 x float>)"(i32, %dx.types.Handle, i32, <4 x float>) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #2

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32, %dx.types.Handle, i32, i32)"(i32, <4 x float>*, i1, <4 x float>, i1, i32, %dx.types.Handle, i32, i32, i32, i32, i32, i1, i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readonly
declare <8 x i32> @"dx.hl.op.ro.<8 x i32> (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, <8 x i32>, <8 x i32>, %dx.types.Handle, i32, i32, i32, i32)"(i32, <8 x i32>, <8 x i32>, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, <8 x i32>, %dx.types.Handle, i32)"(i32, <8 x i32>, %dx.types.Handle, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4}
!dx.entryPoints = !{!8}
!dx.fnprops = !{!18}
!dx.options = !{!19, !20}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{i32 1, i32 9}
!3 = !{!"cs", i32 6, i32 9}
!4 = !{i32 1, void ()* @cs_main, !5}
!5 = !{!6}
!6 = !{i32 1, !7, !7}
!7 = !{}
!8 = !{void ()* @cs_main, !"cs_main", null, !9, null}
!9 = !{!10, !15, null, null}
!10 = !{!11, !12, !13, !14}
!11 = !{i32 0, %struct.ByteAddressBuffer* @"\01?input_vector_buffer@@3UByteAddressBuffer@@A", !"input_vector_buffer", i32 -1, i32 -1, i32 1, i32 11, i32 0, null}
!12 = !{i32 1, %struct.ByteAddressBuffer* @"\01?opa_input_buffer@@3UByteAddressBuffer@@A", !"opa_input_buffer", i32 -1, i32 -1, i32 1, i32 11, i32 0, null}
!13 = !{i32 2, %struct.ByteAddressBuffer* @"\01?matrix_buffer@@3UByteAddressBuffer@@A", !"matrix_buffer", i32 -1, i32 -1, i32 1, i32 11, i32 0, null}
!14 = !{i32 3, %struct.ByteAddressBuffer* @"\01?bias_buffer@@3UByteAddressBuffer@@A", !"bias_buffer", i32 -1, i32 -1, i32 1, i32 11, i32 0, null}
!15 = !{!16, !17}
!16 = !{i32 0, %struct.RWByteAddressBuffer* @"\01?rw_matrix_buffer@@3URWByteAddressBuffer@@A", !"rw_matrix_buffer", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!17 = !{i32 1, %struct.RWByteAddressBuffer* @"\01?output_vector_buffer@@3URWByteAddressBuffer@@A", !"output_vector_buffer", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!18 = !{void ()* @cs_main, i32 5, i32 1, i32 1, i32 1}
!19 = !{i32 -2147483584}
!20 = !{i32 -1}
!21 = !DILocation(line: 14, column: 5, scope: !22)
!22 = !DISubprogram(name: "cs_main", scope: !23, file: !23, line: 12, type: !24, isLocal: false, isDefinition: true, scopeLine: 13, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @cs_main)
!23 = !DIFile(filename: "DirectXShaderCompiler\5Ctools\5Cclang\5Ctest\5CCodeGenDXIL\5Chlsl\5Cintrinsics\5Clinalg_builtins\5Clinalg-builtins.hlsl", directory: "")
!24 = !DISubroutineType(types: !7)
!25 = !DILocation(line: 17, column: 37, scope: !22)
!26 = !DILocation(line: 33, column: 5, scope: !22)
!27 = !DILocation(line: 37, column: 35, scope: !22)
!28 = !{!29, !29, i64 0}
!29 = !{!"omnipotent char", !30, i64 0}
!30 = !{!"Simple C/C++ TBAA"}
!31 = !DILocation(line: 37, column: 5, scope: !22)
!32 = !DILocation(line: 49, column: 5, scope: !22)
!33 = !DILocation(line: 54, column: 38, scope: !22)
!34 = !DILocation(line: 54, column: 5, scope: !22)
!35 = !DILocation(line: 56, column: 37, scope: !22)
!36 = !DILocation(line: 57, column: 37, scope: !22)
!37 = !DILocation(line: 67, column: 5, scope: !22)
!38 = !DILocation(line: 77, column: 5, scope: !22)
!39 = !DILocation(line: 79, column: 1, scope: !22)
