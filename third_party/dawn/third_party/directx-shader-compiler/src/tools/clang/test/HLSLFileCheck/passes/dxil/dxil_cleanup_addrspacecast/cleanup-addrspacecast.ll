; RUN: %opt %s -hlsl-dxil-cleanup-addrspacecast -S | FileCheck %s

; Make sure addrspacecast is removed
; CHECK-NOT: addrspacecast


; ModuleID = 'MyModule'
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWStructuredBuffer<Derived>" = type { %struct.Derived }
%struct.Derived = type { %struct.Base, float }
%struct.Base = type { i32 }
%"$Globals" = type { [2 x %struct.Derived], i32 }
%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.CBufRet.f32 = type { float, float, float, float }
%"class.RWStructuredBuffer<Base>" = type { %struct.Base }

@"\01?sb_Derived@@3PAV?$RWStructuredBuffer@UDerived@@@@A" = external constant [2 x %"class.RWStructuredBuffer<Derived>"], align 4
@"$Globals" = external constant %"$Globals"
@"\01?gs_Derived0@@3UDerived@@A.1" = addrspace(3) global float undef
@"\01?gs_Derived0@@3UDerived@@A.0.0" = addrspace(3) global i32 undef
@"\01?gs_Derived1@@3UDerived@@A.1" = addrspace(3) global float undef
@"\01?gs_Derived1@@3UDerived@@A.0.0" = addrspace(3) global i32 undef
@"\01?gs_Derived@@3PAUDerived@@A.1" = addrspace(3) global [2 x float] undef
@"\01?gs_Derived@@3PAUDerived@@A.0.0" = addrspace(3) global [2 x i32] undef
@"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim" = addrspace(3) global [8 x float] undef
@"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim" = addrspace(3) global [16 x float] undef

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %"$Globals_cbuffer" = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %0 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 0)
  %1 = extractvalue %dx.types.CBufRet.i32 %0, 0
  store i32 %1, i32 addrspace(3)* @"\01?gs_Derived0@@3UDerived@@A.0.0", align 4
  %2 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 0)
  %3 = extractvalue %dx.types.CBufRet.f32 %2, 1
  store float %3, float addrspace(3)* @"\01?gs_Derived0@@3UDerived@@A.1", align 4
  %4 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %5 = extractvalue %dx.types.CBufRet.i32 %4, 2
  %sub = sub nsw i32 1, %5
  %6 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %sub
  %7 = getelementptr [2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 %sub
  %8 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 %5)
  %9 = extractvalue %dx.types.CBufRet.i32 %8, 0
  store i32 %9, i32 addrspace(3)* %6, align 4
  %10 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 %5)
  %11 = extractvalue %dx.types.CBufRet.f32 %10, 1
  store float %11, float addrspace(3)* %7, align 4
  %12 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %13 = extractvalue %dx.types.CBufRet.i32 %12, 2
  %14 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %13
  store i32 1, i32 addrspace(3)* %14, align 4, !tbaa !30
  %15 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %16 = extractvalue %dx.types.CBufRet.i32 %15, 2
  %y10 = getelementptr inbounds [2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 %16
  store float 2.000000e+00, float addrspace(3)* %y10, align 4, !tbaa !34
  %tobool = icmp eq i32 %1, 0
  br i1 %tobool, label %if.then, label %if.else, !dx.controlflow.hints !36

if.then:                                          ; preds = %entry
  store i32 0, i32 addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.0.0", align 4
  store float %3, float addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.1", align 4

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; add addrspacecast inst
  %new.asc.0 = addrspacecast i32 addrspace(3)* bitcast (float addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.1" to i32 addrspace(3)*) to i32*
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  br label %if.end.11

if.else:                                          ; preds = %entry
  %17 = load i32, i32* addrspacecast (i32 addrspace(3)* getelementptr inbounds ([2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 0) to i32*), align 4, !tbaa !30
  %tobool7 = icmp eq i32 %17, 0
  br i1 %tobool7, label %if.else.10, label %if.then.8

if.then.8:                                        ; preds = %if.else
  %18 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %19 = extractvalue %dx.types.CBufRet.i32 %18, 2
  %20 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %19

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; add constant addrspacecast expr inside gep inst
  %new.gep.asc.1 = getelementptr [2 x i32], [2 x i32]* addrspacecast ([2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0" to [2 x i32]*), i32 0, i32 %19
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  %21 = getelementptr [2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 %19
  %22 = load i32, i32 addrspace(3)* %20, align 4
  %23 = load float, float addrspace(3)* %21, align 4
  store i32 %22, i32 addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.0.0", align 4
  store float %23, float addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.1", align 4
  %phitmp26 = sitofp i32 %22 to float
  br label %if.end.11

if.else.10:                                       ; preds = %if.else
  %24 = load float, float addrspace(3)* getelementptr inbounds ([2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 0), align 4
  store i32 0, i32 addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.0.0", align 4
  store float %24, float addrspace(3)* @"\01?gs_Derived1@@3UDerived@@A.1", align 4
  br label %if.end.11

if.end.11:                                        ; preds = %if.then.8, %if.else.10, %if.then
  %25 = phi float [ %phitmp26, %if.then.8 ], [ 0.000000e+00, %if.else.10 ], [ 0.000000e+00, %if.then ]
  %26 = phi float [ %23, %if.then.8 ], [ %24, %if.else.10 ], [ %3, %if.then ]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; add phi mixing incoming addrspace cast scenarios and incoming global pointers
  %new.phi.0 = phi i32* [ %new.gep.asc.1, %if.then.8 ], [ addrspacecast (i32 addrspace(3)* getelementptr inbounds ([2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 0) to i32*), %if.else.10 ], [ %new.asc.0, %if.then ]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  %27 = fmul fast float %25, %25
  %mul2.i = fmul fast float %27, %26
  %28 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %29 = extractvalue %dx.types.CBufRet.i32 %28, 2
  %30 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %29
  %31 = addrspacecast i32 addrspace(3)* %30 to i32*
  %32 = getelementptr [2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 %29
  %33 = addrspacecast float addrspace(3)* %32 to float*
  store i32 5, i32* %31, align 4, !tbaa !30

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; Store to new phi ptr
  store i32 13, i32* %new.phi.0, align 4, !tbaa !30
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  store float 6.000000e+00, float* %33, align 4, !tbaa !34
  %34 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %35 = extractvalue %dx.types.CBufRet.i32 %34, 2
  %36 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %35
  %37 = addrspacecast i32 addrspace(3)* %36 to i32*
  %conv = fptosi float %mul2.i to i32
  store i32 %conv, i32* %37, align 4, !tbaa !30
  %38 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %39 = extractvalue %dx.types.CBufRet.i32 %38, 2
  %40 = add i32 %39, 0
  %sb_Derived_UAV_structbuf29 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 %40, i1 false)
  %41 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %42 = extractvalue %dx.types.CBufRet.i32 %41, 2
  %43 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %42
  %44 = getelementptr [2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 %42
  %45 = load i32, i32 addrspace(3)* %43, align 4
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf29, i32 0, i32 0, i32 %45, i32 undef, i32 undef, i32 undef, i8 1)
  %46 = load float, float addrspace(3)* %44, align 4
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf29, i32 0, i32 4, float %46, float undef, float undef, float undef, i8 1)
  %47 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %48 = extractvalue %dx.types.CBufRet.i32 %47, 2
  %49 = add i32 %48, 0
  %sb_Derived_UAV_structbuf28 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 %49, i1 false)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf28, i32 1, i32 0, i32 7, i32 undef, i32 undef, i32 undef, i8 1)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf28, i32 1, i32 0, i32 7, i32 undef, i32 undef, i32 undef, i8 1)
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf28, i32 1, i32 4, float 8.000000e+00, float undef, float undef, float undef, i8 1)
  %50 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %51 = extractvalue %dx.types.CBufRet.i32 %50, 2
  %cmp.23 = icmp slt i32 %51, 4
  br i1 %cmp.23, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %if.end.11
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %j.025 = phi i32 [ %inc, %for.body ], [ %51, %for.body.preheader ]
  %k.024 = phi i32 [ %sub46, %for.body ], [ 4, %for.body.preheader ]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; add phi mixing incoming scenarios
  %new.phi.1 = phi i32* [ %new.ptr.1, %for.body ], [ %new.phi.0, %for.body.preheader ]
  %new.load.1 = load i32, i32* %new.phi.1, align 4, !tbaa !30
  store i32 %new.load.1, i32* bitcast (float* getelementptr inbounds ([8 x float], [8 x float]* addrspacecast ([8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim" to [8 x float]*), i32 0, i32 0) to i32*), align 4, !tbaa !30

  ; If desired, for additional testing of function case:
  ; use same constant in a function where we will be unable to replace it:
  ;%new.unused.0 = call void @FunctionConsumingPtr(i32* bitcast (float* getelementptr inbounds ([8 x float], [8 x float]* addrspacecast ([8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim" to [8 x float]*), i32 0, i32 0) to i32*))

  ; If desired, for additional testing of function case:
  ; use %new.phi.1 in a function where we will be unable to replace it:
  ;%new.unused.0 = call void @FunctionConsumingPtr(i32* %new.phi.1)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


  %rem = srem i32 %j.025, 2
  %52 = getelementptr [2 x i32], [2 x i32] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.0.0", i32 0, i32 %rem
  %53 = load i32, i32 addrspace(3)* %52, align 4, !tbaa !30

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; use the value
  %new.add.1 = add i32 %new.load.1, %53
  %conv26 = sitofp i32 %new.add.1 to float
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  %sub27 = sub nsw i32 1, %j.025
  %rem28 = srem i32 %sub27, 2
  %y309 = getelementptr inbounds [2 x float], [2 x float] addrspace(3)* @"\01?gs_Derived@@3PAUDerived@@A.1", i32 0, i32 %rem28
  %54 = load float, float addrspace(3)* %y309, align 4, !tbaa !34
  %div = sdiv i32 %j.025, 2
  %55 = mul i32 %div, 2
  %56 = add i32 %rem, %55
  %57 = mul i32 %56, 2
  %58 = add i32 0, %57
  %59 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %58

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; set pointer for loop to bitcast of float pointer
  %new.ptr.0 = addrspacecast float addrspace(3)* %59 to float*
  %new.ptr.1 = bitcast float* %new.ptr.0 to i32*
  ; new.ptr.1 is used in phi at beginning of this block
  ; If desired, for additional testing of function case:
  ; also use it in a function where we will be unable to replace it:
  ;%new.unused.0 = call void @FunctionConsumingPtr(i32* %new.ptr.1)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  store float %conv26, float addrspace(3)* %59, align 8
  %60 = mul i32 %div, 2
  %61 = add i32 %rem, %60
  %62 = mul i32 %61, 2
  %63 = add i32 1, %62
  %64 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %63
  store float %54, float addrspace(3)* %64, align 4
  %rem38 = srem i32 %k.024, 2
  %div39 = sdiv i32 %k.024, 2
  %65 = mul i32 %div39, 2
  %66 = add i32 %rem38, %65
  %67 = mul i32 %66, 2
  %68 = add i32 0, %67
  %69 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %68
  %70 = load float, float addrspace(3)* %69, align 8
  %71 = mul i32 %div39, 2
  %72 = add i32 %rem38, %71
  %73 = mul i32 %72, 2
  %74 = add i32 1, %73
  %75 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %74
  %76 = load float, float addrspace(3)* %75, align 4
  %77 = mul i32 %rem38, 2
  %78 = add i32 %div39, %77
  %79 = mul i32 %78, 2
  %80 = add i32 0, %79
  %81 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %80
  %82 = load float, float addrspace(3)* %81, align 8
  %83 = mul i32 %rem38, 2
  %84 = add i32 %div39, %83
  %85 = mul i32 %84, 2
  %86 = add i32 1, %85
  %87 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %86
  %88 = load float, float addrspace(3)* %87, align 4
  %89 = mul i32 %div, 2
  %90 = add i32 %rem, %89
  %91 = mul i32 %90, 4
  %92 = add i32 0, %91
  %93 = getelementptr [16 x float], [16 x float] addrspace(3)* @"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim", i32 0, i32 %92
  store float %70, float addrspace(3)* %93, align 16
  %94 = mul i32 %div, 2
  %95 = add i32 %rem, %94
  %96 = mul i32 %95, 4
  %97 = add i32 1, %96
  %98 = getelementptr [16 x float], [16 x float] addrspace(3)* @"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim", i32 0, i32 %97
  store float %82, float addrspace(3)* %98, align 4
  %99 = mul i32 %div, 2
  %100 = add i32 %rem, %99
  %101 = mul i32 %100, 4
  %102 = add i32 2, %101
  %103 = getelementptr [16 x float], [16 x float] addrspace(3)* @"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim", i32 0, i32 %102
  store float %76, float addrspace(3)* %103, align 8
  %104 = mul i32 %div, 2
  %105 = add i32 %rem, %104
  %106 = mul i32 %105, 4
  %107 = add i32 3, %106
  %108 = getelementptr [16 x float], [16 x float] addrspace(3)* @"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim", i32 0, i32 %107
  store float %88, float addrspace(3)* %108, align 4
  %sub46 = add nsw i32 %k.024, -1
  %inc = add nsw i32 %j.025, 1
  %exitcond = icmp eq i32 %inc, 4
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %phitmp = srem i32 %51, 2
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %if.end.11
  %k.0.lcssa = phi i32 [ 0, %if.end.11 ], [ %phitmp, %for.end.loopexit ]
  %109 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %110 = extractvalue %dx.types.CBufRet.i32 %109, 2
  %sub47 = sub nsw i32 1, %110
  %111 = add i32 %110, 2
  %112 = mul i32 %sub47, 2
  %113 = add i32 %110, %112
  %114 = mul i32 %113, 4
  %115 = add i32 %110, %114
  %116 = getelementptr [16 x float], [16 x float] addrspace(3)* @"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim", i32 0, i32 %115
  %117 = load float, float addrspace(3)* %116, align 4
  %118 = mul i32 %sub47, 2
  %119 = add i32 %110, %118
  %120 = mul i32 %119, 4
  %121 = add i32 %111, %120
  %122 = getelementptr [16 x float], [16 x float] addrspace(3)* @"\01?gs_matArray@@3PAY01$$CAV?$matrix@M$01$01@@A.v.v.1dim", i32 0, i32 %121
  %123 = load float, float addrspace(3)* %122, align 4
  %124 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %125 = extractvalue %dx.types.CBufRet.i32 %124, 2
  %sub50 = sub nsw i32 1, %125
  %126 = mul i32 %125, 2
  %127 = add i32 %sub50, %126
  %128 = mul i32 %127, 2
  %129 = add i32 0, %128
  %130 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %129
  store float %117, float addrspace(3)* %130, align 8
  %131 = mul i32 %125, 2
  %132 = add i32 %sub50, %131
  %133 = mul i32 %132, 2
  %134 = add i32 1, %133
  %135 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %134
  store float %123, float addrspace(3)* %135, align 4
  %136 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %137 = extractvalue %dx.types.CBufRet.i32 %136, 2
  %sub54 = sub nsw i32 1, %137
  %138 = mul i32 %sub54, 2
  %139 = add i32 %k.0.lcssa, %138
  %140 = mul i32 %139, 2
  %141 = add i32 0, %140
  %142 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %141
  %143 = load float, float addrspace(3)* %142, align 8
  %conv57 = fptosi float %143 to i32
  %144 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %145 = extractvalue %dx.types.CBufRet.i32 %144, 2
  %146 = add i32 %145, 0
  %sb_Derived_UAV_structbuf27 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 %146, i1 false)
  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf27, i32 1, i32 0, i32 %conv57, i32 undef, i32 undef, i32 undef, i8 1)
  %147 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %148 = extractvalue %dx.types.CBufRet.i32 %147, 2
  %sub61 = sub nsw i32 1, %148
  %149 = mul i32 %sub61, 2
  %150 = add i32 %148, %149
  %151 = mul i32 %150, 2
  %152 = add i32 1, %151
  %153 = getelementptr [8 x float], [8 x float] addrspace(3)* @"\01?gs_vecArray@@3PAY01$$CAV?$vector@M$01@@A.v.1dim", i32 0, i32 %152
  %154 = load float, float addrspace(3)* %153, align 4
  %155 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 1)
  %156 = extractvalue %dx.types.CBufRet.i32 %155, 2
  %157 = add i32 %156, 0
  %sb_Derived_UAV_structbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 %157, i1 false)
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %sb_Derived_UAV_structbuf, i32 1, i32 4, float %154, float undef, float undef, float undef, i8 1)
  ret void
}

; Function Attrs: nounwind
declare void @FunctionConsumingPtr(i32*) #2


; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #1

; Function Attrs: nounwind
declare void @dx.op.bufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8) #2

; Function Attrs: nounwind
declare void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind }

!pauseresume = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.resources = !{!5}
!dx.typeAnnotations = !{!11, !23}
!dx.entryPoints = !{!27}

!0 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!1 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 4}
!4 = !{!"cs", i32 6, i32 0}
!5 = !{null, !6, !9, null}
!6 = !{!7}
!7 = !{i32 0, [2 x %"class.RWStructuredBuffer<Derived>"]* undef, !"sb_Derived", i32 0, i32 0, i32 2, i32 12, i1 false, i1 false, i1 false, !8}
!8 = !{i32 1, i32 8}
!9 = !{!10}
!10 = !{i32 0, %"$Globals"* undef, !"$Globals", i32 0, i32 0, i32 1, i32 28, null}
!11 = !{i32 0, %struct.Derived undef, !12, %struct.Base undef, !15, %"class.RWStructuredBuffer<Derived>" undef, !17, %"class.RWStructuredBuffer<Base>" undef, !19, %"$Globals" undef, !20}
!12 = !{i32 8, !13, !14}
!13 = !{i32 6, !"Base", i32 3, i32 0}
!14 = !{i32 6, !"y", i32 3, i32 4, i32 7, i32 9}
!15 = !{i32 4, !16}
!16 = !{i32 6, !"x", i32 3, i32 0, i32 7, i32 4}
!17 = !{i32 8, !18}
!18 = !{i32 6, !"h", i32 3, i32 0}
!19 = !{i32 4, !18}
!20 = !{i32 28, !21, !22}
!21 = !{i32 6, !"c_Derived", i32 3, i32 0}
!22 = !{i32 6, !"i", i32 3, i32 24, i32 7, i32 4}
!23 = !{i32 1, void ()* @main, !24}
!24 = !{!25}
!25 = !{i32 1, !26, !26}
!26 = !{}
!27 = !{void ()* @main, !"main", null, !5, !28}
!28 = !{i32 4, !29}
!29 = !{i32 1, i32 1, i32 1}
!30 = !{!31, !31, i64 0}
!31 = !{!"int", !32, i64 0}
!32 = !{!"omnipotent char", !33, i64 0}
!33 = !{!"Simple C/C++ TBAA"}
!34 = !{!35, !35, i64 0}
!35 = !{!"float", !32, i64 0}
!36 = distinct !{!36, !"dx.controlflow.hints", i32 1}
