; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Test for SROA reduction of globals and allocas.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.VectRec1 = type { <1 x float> }
%struct.VectRec2 = type { <2 x float> }
%ConstantBuffer = type opaque

; Confirm that the dynamic globals are untouched and the statics are scalarized.
; DAG used to preserve the convenient ordering.

; Dynamic access preserves even vec1s in SROA.
; CHECK-DAG: @dyglob1 = internal global <1 x float> zeroinitializer, align 4
; CHECK-DAG: @dygar1 = internal global [2 x <1 x float>] zeroinitializer, align 4
; CHECK-DAG: @dygrec1.0 = internal global <1 x float> zeroinitializer, align 4
; CHECK-DAG: @dyglob2 = internal global <2 x float> zeroinitializer, align 4
; CHECK-DAG: @dygar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
; CHECK-DAG: @dygrec2.0 = internal global <2 x float> zeroinitializer, align 4

; Having >1 elements preserves even statically-accessed vec2s.
; CHECK-DAG: @stgar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
; CHECK-DAG: @stglob2 = internal global <2 x float> zeroinitializer, align 4
; CHECK-DAG: @stgrec2.0 = internal global <2 x float> zeroinitializer, align 4

; Statically-accessed vec1s should get scalarized.
; CHECK-DAG: @stgar1.0 = internal global [2 x float] zeroinitializer, align 4
; CHECK-DAG: @stglob1.0 = internal global float 0.000000e+00, align 4
; CHECK-DAG: @stgrec1.0.0 = internal global float 0.000000e+00, align 4

@dyglob2 = internal global <2 x float> zeroinitializer, align 4
@dygar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
@dygrec2 = internal global %struct.VectRec2 zeroinitializer, align 4
@dyglob1 = internal global <1 x float> zeroinitializer, align 4
@dygar1 = internal global [2 x <1 x float>] zeroinitializer, align 4
@dygrec1 = internal global %struct.VectRec1 zeroinitializer, align 4

@stglob2 = internal global <2 x float> zeroinitializer, align 4
@stgar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
@stgrec2 = internal global %struct.VectRec2 zeroinitializer, align 4

@stglob1 = internal global <1 x float> zeroinitializer, align 4
@stgar1 = internal global [2 x <1 x float>] zeroinitializer, align 4
@stgrec1 = internal global %struct.VectRec1 zeroinitializer, align 4

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define <4 x float> @"\01?tester@@YA?AV?$vector@M$03@@HY0M@M@Z"(i32 %ix, [12 x float]* %vals) #0 {
bb:
  ; Dynamic access preserves even vec1s in SROA.
  ; CHECK-DAG: %dylorc1.0 = alloca <1 x float>
  ; CHECK-DAG: %dylorc2.0 = alloca <2 x float>
  ; CHECK-DAG: %dylorc1.0 = alloca <1 x float>
  ; CHECK-DAG: %dylorc2.0 = alloca <2 x float>
  ; CHECK-DAG: %dylar1 = alloca [3 x <1 x float>]
  ; CHECK-DAG: %dylar2 = alloca [4 x <2 x float>]

  ; SROA doesn't reduce non-array allocas because scalarizer should get them.
  ; CHECK-DAG: %stlorc1.0 = alloca <1 x float>
  ; CHECK-DAG: %stlorc2.0 = alloca <2 x float>
  ; CHECK-DAG: %stloc1 = alloca <1 x float>, align 4
  ; CHECK-DAG: %stloc2 = alloca <2 x float>, align 4

  ; Statically-accessed arrays should get reduced.
  ; CHECK-DAG: %stlar2 = alloca [4 x <2 x float>]
  ; CHECK-DAG: %stlar1.0 = alloca [3 x float]

  %tmp = alloca i32, align 4, !dx.temp !14
  %dyloc1 = alloca <1 x float>, align 4
  %dyloc2 = alloca <2 x float>, align 4
  %dylar1 = alloca [3 x <1 x float>], align 4
  %dylar2 = alloca [4 x <2 x float>], align 4
  %dylorc1 = alloca %struct.VectRec1, align 4
  %dylorc2 = alloca %struct.VectRec2, align 4
  %stloc1 = alloca <1 x float>, align 4
  %stloc2 = alloca <2 x float>, align 4
  %stlar1 = alloca [3 x <1 x float>], align 4
  %stlar2 = alloca [4 x <2 x float>], align 4
  %stlorc1 = alloca %struct.VectRec1, align 4
  %stlorc2 = alloca %struct.VectRec2, align 4

  store i32 %ix, i32* %tmp, align 4
  %tmp13 = load i32, i32* %tmp, align 4 ; line:53 col:7
  %tmp14 = icmp sgt i32 %tmp13, 0 ; line:53 col:10
  %tmp15 = icmp ne i1 %tmp14, false ; line:53 col:10
  %tmp16 = icmp ne i1 %tmp15, false ; line:53 col:10
  br i1 %tmp16, label %bb17, label %bb86 ; line:53 col:7

bb17:                                             ; preds = %bb
  %tmp18 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 0 ; line:54 col:30
  %tmp19 = load float, float* %tmp18, align 4 ; line:54 col:30
  %tmp20 = load i32, i32* %tmp, align 4 ; line:54 col:24
  %tmp21 = getelementptr <1 x float>, <1 x float>* %dyloc1, i32 0, i32 %tmp20 ; line:54 col:17
  store float %tmp19, float* %tmp21 ; line:54 col:28
  %tmp22 = getelementptr <1 x float>, <1 x float>* %stloc1, i32 0, i32 0 ; line:54 col:5
  store float %tmp19, float* %tmp22 ; line:54 col:15
  %tmp23 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 1 ; line:55 col:30
  %tmp24 = load float, float* %tmp23, align 4 ; line:55 col:30
  %tmp25 = load i32, i32* %tmp, align 4 ; line:55 col:24
  %tmp26 = getelementptr <2 x float>, <2 x float>* %dyloc2, i32 0, i32 %tmp25 ; line:55 col:17
  store float %tmp24, float* %tmp26 ; line:55 col:28
  %tmp27 = getelementptr <2 x float>, <2 x float>* %stloc2, i32 0, i32 1 ; line:55 col:5
  store float %tmp24, float* %tmp27 ; line:55 col:15
  %tmp28 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 2 ; line:56 col:37
  %tmp29 = load float, float* %tmp28, align 4 ; line:56 col:37
  %tmp30 = load i32, i32* %tmp, align 4 ; line:56 col:27
  %tmp31 = getelementptr inbounds [3 x <1 x float>], [3 x <1 x float>]* %dylar1, i32 0, i32 %tmp30 ; line:56 col:20
  %tmp32 = load i32, i32* %tmp, align 4 ; line:56 col:31
  %tmp33 = getelementptr <1 x float>, <1 x float>* %tmp31, i32 0, i32 %tmp32 ; line:56 col:20
  store float %tmp29, float* %tmp33 ; line:56 col:35
  %tmp34 = getelementptr inbounds [3 x <1 x float>], [3 x <1 x float>]* %stlar1, i32 0, i32 1 ; line:56 col:5
  %tmp35 = getelementptr <1 x float>, <1 x float>* %tmp34, i32 0, i32 0 ; line:56 col:5
  store float %tmp29, float* %tmp35 ; line:56 col:18
  %tmp36 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 3 ; line:57 col:37
  %tmp37 = load float, float* %tmp36, align 4 ; line:57 col:37
  %tmp38 = load i32, i32* %tmp, align 4 ; line:57 col:27
  %tmp39 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %dylar2, i32 0, i32 %tmp38 ; line:57 col:20
  %tmp40 = load i32, i32* %tmp, align 4 ; line:57 col:31
  %tmp41 = getelementptr <2 x float>, <2 x float>* %tmp39, i32 0, i32 %tmp40 ; line:57 col:20
  store float %tmp37, float* %tmp41 ; line:57 col:35
  %tmp42 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %stlar2, i32 0, i32 1 ; line:57 col:5
  %tmp43 = getelementptr <2 x float>, <2 x float>* %tmp42, i32 0, i32 0 ; line:57 col:5
  store float %tmp37, float* %tmp43 ; line:57 col:18
  %tmp44 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 4 ; line:58 col:36
  %tmp45 = load float, float* %tmp44, align 4 ; line:58 col:36
  %tmp46 = getelementptr inbounds %struct.VectRec1, %struct.VectRec1* %dylorc1, i32 0, i32 0 ; line:58 col:28
  %tmp47 = load i32, i32* %tmp, align 4 ; line:58 col:30
  %tmp48 = getelementptr <1 x float>, <1 x float>* %tmp46, i32 0, i32 %tmp47 ; line:58 col:20
  store float %tmp45, float* %tmp48 ; line:58 col:34
  %tmp49 = getelementptr inbounds %struct.VectRec1, %struct.VectRec1* %stlorc1, i32 0, i32 0 ; line:58 col:13
  %tmp50 = getelementptr <1 x float>, <1 x float>* %tmp49, i32 0, i32 0 ; line:58 col:5
  store float %tmp45, float* %tmp50 ; line:58 col:18
  %tmp51 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 5 ; line:59 col:36
  %tmp52 = load float, float* %tmp51, align 4 ; line:59 col:36
  %tmp53 = getelementptr inbounds %struct.VectRec2, %struct.VectRec2* %dylorc2, i32 0, i32 0 ; line:59 col:28
  %tmp54 = load i32, i32* %tmp, align 4 ; line:59 col:30
  %tmp55 = getelementptr <2 x float>, <2 x float>* %tmp53, i32 0, i32 %tmp54 ; line:59 col:20
  store float %tmp52, float* %tmp55 ; line:59 col:34
  %tmp56 = getelementptr inbounds %struct.VectRec2, %struct.VectRec2* %stlorc2, i32 0, i32 0 ; line:59 col:13
  %tmp57 = getelementptr <2 x float>, <2 x float>* %tmp56, i32 0, i32 1 ; line:59 col:5
  store float %tmp52, float* %tmp57 ; line:59 col:18
  %tmp58 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 6 ; line:61 col:32
  %tmp59 = load float, float* %tmp58, align 4 ; line:61 col:32
  %tmp60 = load i32, i32* %tmp, align 4 ; line:61 col:26
  %tmp61 = getelementptr <1 x float>, <1 x float>* @dyglob1, i32 0, i32 %tmp60 ; line:61 col:18
  store float %tmp59, float* %tmp61 ; line:61 col:30
  store float %tmp59, float* getelementptr inbounds (<1 x float>, <1 x float>* @stglob1, i32 0, i32 0) ; line:61 col:16
  %tmp62 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 7 ; line:62 col:32
  %tmp63 = load float, float* %tmp62, align 4 ; line:62 col:32
  %tmp64 = load i32, i32* %tmp, align 4 ; line:62 col:26
  %tmp65 = getelementptr <2 x float>, <2 x float>* @dyglob2, i32 0, i32 %tmp64 ; line:62 col:18
  store float %tmp63, float* %tmp65 ; line:62 col:30
  store float %tmp63, float* getelementptr inbounds (<2 x float>, <2 x float>* @stglob2, i32 0, i32 1) ; line:62 col:16
  %tmp66 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 8 ; line:63 col:37
  %tmp67 = load float, float* %tmp66, align 4 ; line:63 col:37
  %tmp68 = load i32, i32* %tmp, align 4 ; line:63 col:27
  %tmp69 = getelementptr inbounds [2 x <1 x float>], [2 x <1 x float>]* @dygar1, i32 0, i32 %tmp68 ; line:63 col:20
  %tmp70 = load i32, i32* %tmp, align 4 ; line:63 col:31
  %tmp71 = getelementptr <1 x float>, <1 x float>* %tmp69, i32 0, i32 %tmp70 ; line:63 col:20
  store float %tmp67, float* %tmp71 ; line:63 col:35
  store float %tmp67, float* getelementptr inbounds ([2 x <1 x float>], [2 x <1 x float>]* @stgar1, i32 0, i32 1, i32 0) ; line:63 col:18
  %tmp72 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 9 ; line:64 col:37
  %tmp73 = load float, float* %tmp72, align 4 ; line:64 col:37
  %tmp74 = load i32, i32* %tmp, align 4 ; line:64 col:27
  %tmp75 = getelementptr inbounds [3 x <2 x float>], [3 x <2 x float>]* @dygar2, i32 0, i32 %tmp74 ; line:64 col:20
  %tmp76 = load i32, i32* %tmp, align 4 ; line:64 col:31
  %tmp77 = getelementptr <2 x float>, <2 x float>* %tmp75, i32 0, i32 %tmp76 ; line:64 col:20
  store float %tmp73, float* %tmp77 ; line:64 col:35
  store float %tmp73, float* getelementptr inbounds ([3 x <2 x float>], [3 x <2 x float>]* @stgar2, i32 0, i32 1, i32 1) ; line:64 col:18
  %tmp78 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 10 ; line:65 col:36
  %tmp79 = load float, float* %tmp78, align 4 ; line:65 col:36
  %tmp80 = load i32, i32* %tmp, align 4 ; line:65 col:30
  %tmp81 = getelementptr <1 x float>, <1 x float>* getelementptr inbounds (%struct.VectRec1, %struct.VectRec1* @dygrec1, i32 0, i32 0), i32 0, i32 %tmp80 ; line:65 col:20
  store float %tmp79, float* %tmp81 ; line:65 col:34
  store float %tmp79, float* getelementptr inbounds (%struct.VectRec1, %struct.VectRec1* @stgrec1, i32 0, i32 0, i32 0) ; line:65 col:18
  %tmp82 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 11 ; line:66 col:36
  %tmp83 = load float, float* %tmp82, align 4 ; line:66 col:36
  %tmp84 = load i32, i32* %tmp, align 4 ; line:66 col:30
  %tmp85 = getelementptr <2 x float>, <2 x float>* getelementptr inbounds (%struct.VectRec2, %struct.VectRec2* @dygrec2, i32 0, i32 0), i32 0, i32 %tmp84 ; line:66 col:20
  store float %tmp83, float* %tmp85 ; line:66 col:34
  store float %tmp83, float* getelementptr inbounds (%struct.VectRec2, %struct.VectRec2* @stgrec2, i32 0, i32 0, i32 1) ; line:66 col:18
  br label %bb86 ; line:67 col:3

bb86:                                             ; preds = %bb17, %bb
  %tmp87 = load <1 x float>, <1 x float>* %dyloc1, align 4 ; line:68 col:17
  %tmp88 = extractelement <1 x float> %tmp87, i32 0 ; line:68 col:17
  %tmp89 = load <2 x float>, <2 x float>* %dyloc2, align 4 ; line:68 col:27
  %tmp90 = extractelement <2 x float> %tmp89, i32 1 ; line:68 col:27
  %tmp91 = load <1 x float>, <1 x float>* %stloc1, align 4 ; line:68 col:37
  %tmp92 = extractelement <1 x float> %tmp91, i32 0 ; line:68 col:37
  %tmp93 = load <2 x float>, <2 x float>* %stloc2, align 4 ; line:68 col:47
  %tmp94 = extractelement <2 x float> %tmp93, i32 1 ; line:68 col:47
  %tmp95 = insertelement <4 x float> undef, float %tmp88, i64 0 ; line:68 col:16
  %tmp96 = insertelement <4 x float> %tmp95, float %tmp90, i64 1 ; line:68 col:16
  %tmp97 = insertelement <4 x float> %tmp96, float %tmp92, i64 2 ; line:68 col:16
  %tmp98 = insertelement <4 x float> %tmp97, float %tmp94, i64 3 ; line:68 col:16
  %tmp99 = load i32, i32* %tmp, align 4 ; line:68 col:73
  %tmp100 = getelementptr inbounds [3 x <1 x float>], [3 x <1 x float>]* %dylar1, i32 0, i32 %tmp99 ; line:68 col:66
  %tmp101 = load i32, i32* %tmp, align 4 ; line:68 col:77
  %tmp102 = getelementptr <1 x float>, <1 x float>* %tmp100, i32 0, i32 %tmp101 ; line:68 col:66
  %tmp103 = load float, float* %tmp102 ; line:68 col:66
  %tmp104 = load i32, i32* %tmp, align 4 ; line:68 col:89
  %tmp105 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %dylar2, i32 0, i32 %tmp104 ; line:68 col:82
  %tmp106 = load i32, i32* %tmp, align 4 ; line:68 col:93
  %tmp107 = getelementptr <2 x float>, <2 x float>* %tmp105, i32 0, i32 %tmp106 ; line:68 col:82
  %tmp108 = load float, float* %tmp107 ; line:68 col:82
  %tmp109 = getelementptr inbounds [3 x <1 x float>], [3 x <1 x float>]* %stlar1, i32 0, i32 0 ; line:68 col:98
  %tmp110 = load <1 x float>, <1 x float>* %tmp109, align 4 ; line:68 col:98
  %tmp111 = extractelement <1 x float> %tmp110, i32 0 ; line:68 col:98
  %tmp112 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %stlar2, i32 0, i32 0 ; line:68 col:111
  %tmp113 = load <2 x float>, <2 x float>* %tmp112, align 4 ; line:68 col:111
  %tmp114 = extractelement <2 x float> %tmp113, i32 1 ; line:68 col:111
  %tmp115 = insertelement <4 x float> undef, float %tmp103, i64 0 ; line:68 col:65
  %tmp116 = insertelement <4 x float> %tmp115, float %tmp108, i64 1 ; line:68 col:65
  %tmp117 = insertelement <4 x float> %tmp116, float %tmp111, i64 2 ; line:68 col:65
  %tmp118 = insertelement <4 x float> %tmp117, float %tmp114, i64 3 ; line:68 col:65
  %tmp119 = fadd <4 x float> %tmp98, %tmp118 ; line:68 col:57
  %tmp120 = load <1 x float>, <1 x float>* @dyglob1, align 4 ; line:69 col:10
  %tmp121 = extractelement <1 x float> %tmp120, i32 0 ; line:69 col:10
  %tmp122 = load <2 x float>, <2 x float>* @dyglob2, align 4 ; line:69 col:21
  %tmp123 = extractelement <2 x float> %tmp122, i32 1 ; line:69 col:21
  %tmp124 = load <1 x float>, <1 x float>* @stglob1, align 4 ; line:69 col:32
  %tmp125 = extractelement <1 x float> %tmp124, i32 0 ; line:69 col:32
  %tmp126 = load <2 x float>, <2 x float>* @stglob2, align 4 ; line:69 col:43
  %tmp127 = extractelement <2 x float> %tmp126, i32 1 ; line:69 col:43
  %tmp128 = insertelement <4 x float> undef, float %tmp121, i64 0 ; line:69 col:9
  %tmp129 = insertelement <4 x float> %tmp128, float %tmp123, i64 1 ; line:69 col:9
  %tmp130 = insertelement <4 x float> %tmp129, float %tmp125, i64 2 ; line:69 col:9
  %tmp131 = insertelement <4 x float> %tmp130, float %tmp127, i64 3 ; line:69 col:9
  %tmp132 = fadd <4 x float> %tmp119, %tmp131 ; line:68 col:124
  %tmp133 = load i32, i32* %tmp, align 4 ; line:69 col:70
  %tmp134 = getelementptr inbounds [2 x <1 x float>], [2 x <1 x float>]* @dygar1, i32 0, i32 %tmp133 ; line:69 col:63
  %tmp135 = load i32, i32* %tmp, align 4 ; line:69 col:74
  %tmp136 = getelementptr <1 x float>, <1 x float>* %tmp134, i32 0, i32 %tmp135 ; line:69 col:63
  %tmp137 = load float, float* %tmp136 ; line:69 col:63
  %tmp138 = load i32, i32* %tmp, align 4 ; line:69 col:86
  %tmp139 = getelementptr inbounds [3 x <2 x float>], [3 x <2 x float>]* @dygar2, i32 0, i32 %tmp138 ; line:69 col:79
  %tmp140 = load i32, i32* %tmp, align 4 ; line:69 col:90
  %tmp141 = getelementptr <2 x float>, <2 x float>* %tmp139, i32 0, i32 %tmp140 ; line:69 col:79
  %tmp142 = load float, float* %tmp141 ; line:69 col:79
  %tmp143 = load <1 x float>, <1 x float>* getelementptr inbounds ([2 x <1 x float>], [2 x <1 x float>]* @stgar1, i32 0, i32 0), align 4 ; line:69 col:95
  %tmp144 = extractelement <1 x float> %tmp143, i32 0 ; line:69 col:95
  %tmp145 = load <2 x float>, <2 x float>* getelementptr inbounds ([3 x <2 x float>], [3 x <2 x float>]* @stgar2, i32 0, i32 0), align 4 ; line:69 col:108
  %tmp146 = extractelement <2 x float> %tmp145, i32 1 ; line:69 col:108
  %tmp147 = insertelement <4 x float> undef, float %tmp137, i64 0 ; line:69 col:62
  %tmp148 = insertelement <4 x float> %tmp147, float %tmp142, i64 1 ; line:69 col:62
  %tmp149 = insertelement <4 x float> %tmp148, float %tmp144, i64 2 ; line:69 col:62
  %tmp150 = insertelement <4 x float> %tmp149, float %tmp146, i64 3 ; line:69 col:62
  %tmp151 = fadd <4 x float> %tmp132, %tmp150 ; line:69 col:54
  %tmp152 = getelementptr inbounds %struct.VectRec1, %struct.VectRec1* %stlorc1, i32 0, i32 0 ; line:70 col:20
  %tmp153 = load <1 x float>, <1 x float>* %tmp152, align 4 ; line:70 col:20
  %tmp154 = extractelement <1 x float> %tmp153, i64 0 ; line:70 col:11
  %tmp155 = getelementptr inbounds %struct.VectRec2, %struct.VectRec2* %stlorc2, i32 0, i32 0 ; line:70 col:31
  %tmp156 = getelementptr <2 x float>, <2 x float>* %tmp155, i32 0, i32 1 ; line:70 col:23
  %tmp157 = load float, float* %tmp156 ; line:70 col:23
  %tmp158 = getelementptr inbounds %struct.VectRec1, %struct.VectRec1* %dylorc1, i32 0, i32 0 ; line:70 col:45
  %tmp159 = load <1 x float>, <1 x float>* %tmp158, align 4 ; line:70 col:45
  %tmp160 = extractelement <1 x float> %tmp159, i64 0 ; line:70 col:11
  %tmp161 = getelementptr inbounds %struct.VectRec2, %struct.VectRec2* %dylorc2, i32 0, i32 0 ; line:70 col:56
  %tmp162 = load i32, i32* %tmp, align 4 ; line:70 col:58
  %tmp163 = getelementptr <2 x float>, <2 x float>* %tmp161, i32 0, i32 %tmp162 ; line:70 col:48
  %tmp164 = load float, float* %tmp163 ; line:70 col:48
  %tmp165 = insertelement <4 x float> undef, float %tmp154, i64 0 ; line:70 col:11
  %tmp166 = insertelement <4 x float> %tmp165, float %tmp157, i64 1 ; line:70 col:11
  %tmp167 = insertelement <4 x float> %tmp166, float %tmp160, i64 2 ; line:70 col:11
  %tmp168 = insertelement <4 x float> %tmp167, float %tmp164, i64 3 ; line:70 col:11
  %tmp169 = fadd <4 x float> %tmp151, %tmp168 ; line:69 col:121
  %tmp170 = load <1 x float>, <1 x float>* getelementptr inbounds (%struct.VectRec1, %struct.VectRec1* @stgrec1, i32 0, i32 0), align 4 ; line:70 col:80
  %tmp171 = extractelement <1 x float> %tmp170, i64 0 ; line:70 col:71
  %tmp172 = load float, float* getelementptr inbounds (%struct.VectRec2, %struct.VectRec2* @stgrec2, i32 0, i32 0, i32 1) ; line:70 col:83
  %tmp173 = load <1 x float>, <1 x float>* getelementptr inbounds (%struct.VectRec1, %struct.VectRec1* @dygrec1, i32 0, i32 0), align 4 ; line:70 col:105
  %tmp174 = extractelement <1 x float> %tmp173, i64 0 ; line:70 col:71
  %tmp175 = load i32, i32* %tmp, align 4 ; line:70 col:118
  %tmp176 = getelementptr <2 x float>, <2 x float>* getelementptr inbounds (%struct.VectRec2, %struct.VectRec2* @dygrec2, i32 0, i32 0), i32 0, i32 %tmp175 ; line:70 col:108
  %tmp177 = load float, float* %tmp176 ; line:70 col:108
  %tmp178 = insertelement <4 x float> undef, float %tmp171, i64 0 ; line:70 col:71
  %tmp179 = insertelement <4 x float> %tmp178, float %tmp172, i64 1 ; line:70 col:71
  %tmp180 = insertelement <4 x float> %tmp179, float %tmp174, i64 2 ; line:70 col:71
  %tmp181 = insertelement <4 x float> %tmp180, float %tmp177, i64 3 ; line:70 col:71
  %tmp182 = fadd <4 x float> %tmp169, %tmp181 ; line:70 col:63
  ret <4 x float> %tmp182 ; line:68 col:3
}

attributes #0 = { nounwind }

!pauseresume = !{!1}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !10}
!dx.entryPoints = !{!19}
!dx.fnprops = !{}
!dx.options = !{!23, !24}

!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!3 = !{i32 1, i32 9}
!4 = !{!"lib", i32 6, i32 9}
!5 = !{i32 0, %struct.VectRec1 undef, !6, %struct.VectRec2 undef, !8}
!6 = !{i32 4, !7}
!7 = !{i32 6, !"f", i32 3, i32 0, i32 4, !"REC1", i32 7, i32 9, i32 13, i32 1}
!8 = !{i32 8, !9}
!9 = !{i32 6, !"f", i32 3, i32 0, i32 4, !"REC2", i32 7, i32 9, i32 13, i32 2}
!10 = !{i32 1, <4 x float> (i32, [12 x float]*)* @"\01?tester@@YA?AV?$vector@M$03@@HY0M@M@Z", !11}
!11 = !{!12, !15, !17}
!12 = !{i32 1, !13, !14}
!13 = !{i32 7, i32 9, i32 13, i32 4}
!14 = !{}
!15 = !{i32 0, !16, !14}
!16 = !{i32 4, !"IX", i32 7, i32 4}
!17 = !{i32 0, !18, !14}
!18 = !{i32 4, !"VAL", i32 7, i32 9}
!19 = !{null, !"", null, !20, null}
!20 = !{null, null, !21, null}
!21 = !{!22}
!22 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!23 = !{i32 64}
!24 = !{i32 -1}
!25 = !{!26, !26, i64 0}
!26 = !{!"int", !27, i64 0}
!27 = !{!"omnipotent char", !28, i64 0}
!28 = !{!"Simple C/C++ TBAA"}
