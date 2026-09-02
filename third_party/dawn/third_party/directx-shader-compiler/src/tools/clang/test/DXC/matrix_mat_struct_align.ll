; RUN: %dxopt %s -hlsl-passes-resume -hlmatrixlower -S | FileCheck %s

; Ensure that groupshared matrix global in struct gets proper alignment
; Generated using groupshared-member-matrix-subscript-align.hlsl

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.StructuredBuffer<Data>" = type { %struct.Data }
%struct.Data = type { %class.matrix.float.4.4 }
%class.matrix.float.4.4 = type { [4 x <4 x float>] }
%"class.RWStructuredBuffer<Data>" = type { %struct.Data }
%ConstantBuffer = type opaque
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?input@@3V?$StructuredBuffer@UData@@@@A" = external global %"class.StructuredBuffer<Data>", align 4
@"\01?output@@3V?$RWStructuredBuffer@UData@@@@A" = external global %"class.RWStructuredBuffer<Data>", align 4
@"$Globals" = external constant %ConstantBuffer
; CHECK: GData{{.*}} = addrspace(3) global <16 x float> undef, align 16
@"\01?GData@@3UData@@A.0" = addrspace(3) global %class.matrix.float.4.4 undef, align 4

; Function Attrs: nounwind
define void @main(i32 %Id, i32 %g) #0 {
  %1 = alloca i32, align 4, !dx.temp !15
  store i32 %Id, i32* %1, align 4, !tbaa !33
  %2 = load %"class.StructuredBuffer<Data>", %"class.StructuredBuffer<Data>"* @"\01?input@@3V?$StructuredBuffer@UData@@@@A", !dbg !37 ; line:67 col:11
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<Data>\22)"(i32 0, %"class.StructuredBuffer<Data>" %2), !dbg !37 ; line:67 col:11
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<Data>\22)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 524, i32 64 }, %"class.StructuredBuffer<Data>" undef), !dbg !37 ; line:67 col:11
  %5 = call %struct.Data* @"dx.hl.subscript.[].rn.%struct.Data* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %4, i32 0), !dbg !37 ; line:67 col:11
  %6 = getelementptr inbounds %struct.Data, %struct.Data* %5, i32 0, i32 0, !dbg !37 ; line:67 col:11
  %7 = call %class.matrix.float.4.4 @"dx.hl.matldst.rowLoad.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4*)"(i32 2, %class.matrix.float.4.4* %6), !dbg !37 ; line:67 col:11
  %8 = call %class.matrix.float.4.4 @"dx.hl.matldst.rowStore.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4 addrspace(3)*, %class.matrix.float.4.4)"(i32 3, %class.matrix.float.4.4 addrspace(3)* @"\01?GData@@3UData@@A.0", %class.matrix.float.4.4 %7), !dbg !37 ; line:67 col:11
  call void @"dx.hl.op.nd.void (i32)"(i32 24), !dbg !41 ; line:68 col:3
  %9 = load i32, i32* %1, align 4, !dbg !42, !tbaa !33 ; line:88 col:10
  %10 = load %"class.RWStructuredBuffer<Data>", %"class.RWStructuredBuffer<Data>"* @"\01?output@@3V?$RWStructuredBuffer@UData@@@@A", !dbg !43 ; line:88 col:3
  %11 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Data>\22)"(i32 0, %"class.RWStructuredBuffer<Data>" %10), !dbg !43 ; line:88 col:3
  %12 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Data>\22)"(i32 14, %dx.types.Handle %11, %dx.types.ResourceProperties { i32 4620, i32 64 }, %"class.RWStructuredBuffer<Data>" undef), !dbg !43 ; line:88 col:3
  %13 = call %struct.Data* @"dx.hl.subscript.[].rn.%struct.Data* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %12, i32 %9), !dbg !43 ; line:88 col:3
  %14 = getelementptr inbounds %struct.Data, %struct.Data* %13, i32 0, i32 0, !dbg !44 ; line:88 col:16
  %15 = call %class.matrix.float.4.4 @"dx.hl.matldst.rowLoad.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4 addrspace(3)*)"(i32 2, %class.matrix.float.4.4 addrspace(3)* @"\01?GData@@3UData@@A.0"), !dbg !44 ; line:88 col:16
  %16 = call %class.matrix.float.4.4 @"dx.hl.matldst.rowStore.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4*, %class.matrix.float.4.4)"(i32 3, %class.matrix.float.4.4* %14, %class.matrix.float.4.4 %15), !dbg !44 ; line:88 col:16
  ret void, !dbg !45 ; line:90 col:1
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p3i8.p0i8.i64(i8 addrspace(3)* nocapture, i8* nocapture readonly, i64, i32, i1) #0

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p3i8.i64(i8* nocapture, i8 addrspace(3)* nocapture readonly, i64, i32, i1) #0

; Function Attrs: nounwind readnone
declare %struct.Data* @"dx.hl.subscript.[].rn.%struct.Data* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<Data>\22)"(i32, %"class.StructuredBuffer<Data>") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<Data>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.StructuredBuffer<Data>") #1

; Function Attrs: noduplicate nounwind
declare void @"dx.hl.op.nd.void (i32)"(i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Data>\22)"(i32, %"class.RWStructuredBuffer<Data>") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Data>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<Data>") #1

; Function Attrs: nounwind readonly
declare %class.matrix.float.4.4 @"dx.hl.matldst.rowLoad.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4*)"(i32, %class.matrix.float.4.4*) #3

; Function Attrs: nounwind
declare %class.matrix.float.4.4 @"dx.hl.matldst.rowStore.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4 addrspace(3)*, %class.matrix.float.4.4)"(i32, %class.matrix.float.4.4 addrspace(3)*, %class.matrix.float.4.4) #0

; Function Attrs: nounwind readonly
declare %class.matrix.float.4.4 @"dx.hl.matldst.rowLoad.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4 addrspace(3)*)"(i32, %class.matrix.float.4.4 addrspace(3)*) #3

; Function Attrs: nounwind
declare %class.matrix.float.4.4 @"dx.hl.matldst.rowStore.%class.matrix.float.4.4 (i32, %class.matrix.float.4.4*, %class.matrix.float.4.4)"(i32, %class.matrix.float.4.4*, %class.matrix.float.4.4) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { noduplicate nounwind }
attributes #3 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !12}
!dx.entryPoints = !{!21}
!dx.fnprops = !{!30}
!dx.options = !{!31, !32}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4582 (gs_mat_ldst, 1d3f00bbf)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{i32 0, %struct.Data undef, !7, %"class.StructuredBuffer<Data>" undef, !10, %"class.RWStructuredBuffer<Data>" undef, !10}
!7 = !{i32 64, !8}
!8 = !{i32 6, !"m", i32 2, !9, i32 3, i32 0, i32 7, i32 9}
!9 = !{i32 4, i32 4, i32 2}
!10 = !{i32 64, !11}
!11 = !{i32 6, !"h", i32 3, i32 0}
!12 = !{i32 1, void (i32, i32)* @main, !13}
!13 = !{!14, !16, !19}
!14 = !{i32 1, !15, !15}
!15 = !{}
!16 = !{i32 0, !17, !18}
!17 = !{i32 4, !"SV_DispatchThreadId", i32 7, i32 5}
!18 = !{i32 0}
!19 = !{i32 0, !20, !18}
!20 = !{i32 4, !"SV_GroupID", i32 7, i32 5}
!21 = !{void (i32, i32)* @main, !"main", null, !22, null}
!22 = !{!23, !26, !28, null}
!23 = !{!24}
!24 = !{i32 0, %"class.StructuredBuffer<Data>"* @"\01?input@@3V?$StructuredBuffer@UData@@@@A", !"input", i32 0, i32 0, i32 1, i32 12, i32 0, !25}
!25 = !{i32 1, i32 64}
!26 = !{!27}
!27 = !{i32 0, %"class.RWStructuredBuffer<Data>"* @"\01?output@@3V?$RWStructuredBuffer@UData@@@@A", !"output", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !25}
!28 = !{!29}
!29 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!30 = !{void (i32, i32)* @main, i32 5, i32 128, i32 1, i32 1}
!31 = !{i32 -2147483584}
!32 = !{i32 -1}
!33 = !{!34, !34, i64 0}
!34 = !{!"int", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !DILocation(line: 67, column: 11, scope: !38)
!38 = !DISubprogram(name: "main", scope: !39, file: !39, line: 36, type: !40, isLocal: false, isDefinition: true, scopeLine: 37, flags: DIFlagPrototyped, isOptimized: false, function: void (i32, i32)* @main)
!39 = !DIFile(filename: "d:\5Cdxc\5CDirectXShaderCompiler\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Chlsl\5Ctypes\5Cmodifiers\5Cgroupshared\5Cgroupshared-member-matrix-subscript-align.hlsl", directory: "")
!40 = !DISubroutineType(types: !15)
!41 = !DILocation(line: 68, column: 3, scope: !38)
!42 = !DILocation(line: 88, column: 10, scope: !38)
!43 = !DILocation(line: 88, column: 3, scope: !38)
!44 = !DILocation(line: 88, column: 16, scope: !38)
!45 = !DILocation(line: 90, column: 1, scope: !38)
