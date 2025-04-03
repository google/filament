; RUN: %dxopt %s -dynamic-vector-to-array,ReplaceAllVectors=0 -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.VectRec1 = type { <1 x float> }
%struct.VectRec2 = type { <2 x float> }

; Vec2s should be preserved.
; CHECK-DAG: @dyglob2 = internal global <2 x float> zeroinitializer, align 4
; CHECK-DAG: @dygar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
; CHECK-DAG: @dygrec2.0 = internal global <2 x float> zeroinitializer, align 4

; CHECK-DAG: @stgrec2.0 = internal global <2 x float> zeroinitializer, align 4
; CHECK-DAG: @stglob2 = internal global <2 x float> zeroinitializer, align 4
; CHECK-DAG: @stgar2 = internal global [3 x <2 x float>] zeroinitializer, align 4

; Dynamic Vec1s should be reduced.
; CHECK-DAG: @dygar1.v = internal global [2 x [1 x float]] zeroinitializer, align 4
; CHECK-DAG: @dygrec1.0.v = internal global [1 x float] zeroinitializer, align 4
; CHECK-DAG: @dyglob1.v = internal global [1 x float] zeroinitializer, align 4

; These static accessed Vec1s were already reduced by SROA
; CHECK-DAG: @stgar1.0 = internal global [2 x float] zeroinitializer, align 4
; CHECK-DAG: @stglob1.0 = internal global float 0.000000e+00, align 4
; CHECK-DAG: @stgrec1.0.0 = internal global float 0.000000e+00, align 4

@dyglob1 = internal global <1 x float> zeroinitializer, align 4
@dyglob2 = internal global <2 x float> zeroinitializer, align 4
@stglob2 = internal global <2 x float> zeroinitializer, align 4
@dygar1 = internal global [2 x <1 x float>] zeroinitializer, align 4
@dygar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
@stgar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
@dygrec2.0 = internal global <2 x float> zeroinitializer, align 4
@stgrec2.0 = internal global <2 x float> zeroinitializer, align 4
@stgar1.0 = internal global [2 x float] zeroinitializer, align 4
@dygrec1.0 = internal global <1 x float> zeroinitializer, align 4
@stglob1.0 = internal global float 0.000000e+00, align 4
@stgrec1.0.0 = internal global float 0.000000e+00, align 4

; Function Attrs: nounwind
; CHECK-LOCAL: define <4 x float> @"\01?tester
define <4 x float> @"\01?tester@@YA?AV?$vector@M$03@@HY0M@M@Z"(i32 %ix, [12 x float]* %vals) #0 {
bb:
  ; Vec2s are preserved.
  ; CHECK-DAG: %dyloc2 = alloca <2 x float>
  ; CHECK-DAG: %dylar2 = alloca [4 x <2 x float>]
  ; CHECK-DAG: %dylorc2.0 = alloca <2 x float>

  ; CHECK-DAG: %stloc2 = alloca <2 x float>
  ; CHECK-DAG: %stlar2 = alloca [4 x <2 x float>]
  ; CHECK-DAG: %stlorc2.0 = alloca <2 x float>

  ; Statics vec1s are unaltered by dynamic vector to array.
  ; CHECK-DAG: %stloc1 = alloca <1 x float>
  ; CHECK-DAG: %stlar1.0 = alloca [3 x float]
  ; CHECK-DAG: %stlorc1.0 = alloca <1 x float>

  ; Dynamic vec1s are removed and lose their names.
  ; CHECK-DAG: alloca [1 x float]
  ; CHECK-DAG: alloca [3 x [1 x float]]
  ; CHECK-DAG: alloca [1 x float]

  %dylorc1.0 = alloca <1 x float>
  %stlorc1.0 = alloca <1 x float>
  %dylorc2.0 = alloca <2 x float>
  %stlorc2.0 = alloca <2 x float>
  %stlar1.0 = alloca [3 x float]
  %tmp = alloca i32, align 4
  %dyloc1 = alloca <1 x float>, align 4
  %dyloc2 = alloca <2 x float>, align 4
  %dylar1 = alloca [3 x <1 x float>], align 4
  %dylar2 = alloca [4 x <2 x float>], align 4
  %stloc1 = alloca <1 x float>, align 4
  %stloc2 = alloca <2 x float>, align 4
  %stlar2 = alloca [4 x <2 x float>], align 4
  store i32 %ix, i32* %tmp, align 4

  %tmp13 = load i32, i32* %tmp, align 4 ; line:53 col:7
  %tmp14 = icmp sgt i32 %tmp13, 0 ; line:53 col:10
  %tmp15 = icmp ne i1 %tmp14, false ; line:53 col:10
  %tmp16 = icmp ne i1 %tmp15, false ; line:53 col:10
  br i1 %tmp16, label %bb17, label %bb76 ; line:53 col:7

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
  %tmp31 = load i32, i32* %tmp, align 4 ; line:56 col:31
  %tmp32 = getelementptr inbounds [3 x <1 x float>], [3 x <1 x float>]* %dylar1, i32 0, i32 %tmp30, i32 %tmp31 ; line:56 col:20
  store float %tmp29, float* %tmp32 ; line:56 col:35
  %tmp33 = getelementptr inbounds [3 x float], [3 x float]* %stlar1.0, i32 0, i32 1 ; line:56 col:5
  store float %tmp29, float* %tmp33 ; line:56 col:18
  %tmp34 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 3 ; line:57 col:37
  %tmp35 = load float, float* %tmp34, align 4 ; line:57 col:37
  %tmp36 = load i32, i32* %tmp, align 4 ; line:57 col:27
  %tmp37 = load i32, i32* %tmp, align 4 ; line:57 col:31
  %tmp38 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %dylar2, i32 0, i32 %tmp36, i32 %tmp37 ; line:57 col:20
  store float %tmp35, float* %tmp38 ; line:57 col:35
  %tmp39 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %stlar2, i32 0, i32 1, i32 0 ; line:57 col:5
  store float %tmp35, float* %tmp39 ; line:57 col:18
  %tmp40 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 4 ; line:58 col:36
  %tmp41 = load float, float* %tmp40, align 4 ; line:58 col:36
  %tmp42 = load i32, i32* %tmp, align 4 ; line:58 col:30
  %tmp43 = getelementptr inbounds <1 x float>, <1 x float>* %dylorc1.0, i32 0, i32 %tmp42 ; line:58 col:20
  store float %tmp41, float* %tmp43 ; line:58 col:34
  %tmp44 = getelementptr inbounds <1 x float>, <1 x float>* %stlorc1.0, i32 0, i32 0 ; line:58 col:5
  store float %tmp41, float* %tmp44 ; line:58 col:18
  %tmp45 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 5 ; line:59 col:36
  %tmp46 = load float, float* %tmp45, align 4 ; line:59 col:36
  %tmp47 = load i32, i32* %tmp, align 4 ; line:59 col:30
  %tmp48 = getelementptr inbounds <2 x float>, <2 x float>* %dylorc2.0, i32 0, i32 %tmp47 ; line:59 col:20
  store float %tmp46, float* %tmp48 ; line:59 col:34
  %tmp49 = getelementptr inbounds <2 x float>, <2 x float>* %stlorc2.0, i32 0, i32 1 ; line:59 col:5
  store float %tmp46, float* %tmp49 ; line:59 col:18
  %tmp50 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 6 ; line:61 col:32
  %tmp51 = load float, float* %tmp50, align 4 ; line:61 col:32
  %tmp52 = load i32, i32* %tmp, align 4 ; line:61 col:26
  %tmp53 = getelementptr <1 x float>, <1 x float>* @dyglob1, i32 0, i32 %tmp52 ; line:61 col:18
  store float %tmp51, float* %tmp53 ; line:61 col:30
  store float %tmp51, float* @stglob1.0 ; line:61 col:16
  %tmp54 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 7 ; line:62 col:32
  %tmp55 = load float, float* %tmp54, align 4 ; line:62 col:32
  %tmp56 = load i32, i32* %tmp, align 4 ; line:62 col:26
  %tmp57 = getelementptr <2 x float>, <2 x float>* @dyglob2, i32 0, i32 %tmp56 ; line:62 col:18
  store float %tmp55, float* %tmp57 ; line:62 col:30
  store float %tmp55, float* getelementptr inbounds (<2 x float>, <2 x float>* @stglob2, i32 0, i32 1) ; line:62 col:16
  %tmp58 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 8 ; line:63 col:37
  %tmp59 = load float, float* %tmp58, align 4 ; line:63 col:37
  %tmp60 = load i32, i32* %tmp, align 4 ; line:63 col:27
  %tmp61 = load i32, i32* %tmp, align 4 ; line:63 col:31
  %tmp62 = getelementptr inbounds [2 x <1 x float>], [2 x <1 x float>]* @dygar1, i32 0, i32 %tmp60, i32 %tmp61 ; line:63 col:20
  store float %tmp59, float* %tmp62 ; line:63 col:35
  store float %tmp59, float* getelementptr inbounds ([2 x float], [2 x float]* @stgar1.0, i32 0, i32 1) ; line:63 col:18
  %tmp63 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 9 ; line:64 col:37
  %tmp64 = load float, float* %tmp63, align 4 ; line:64 col:37
  %tmp65 = load i32, i32* %tmp, align 4 ; line:64 col:27
  %tmp66 = load i32, i32* %tmp, align 4 ; line:64 col:31
  %tmp67 = getelementptr inbounds [3 x <2 x float>], [3 x <2 x float>]* @dygar2, i32 0, i32 %tmp65, i32 %tmp66 ; line:64 col:20
  store float %tmp64, float* %tmp67 ; line:64 col:35
  store float %tmp64, float* getelementptr inbounds ([3 x <2 x float>], [3 x <2 x float>]* @stgar2, i32 0, i32 1, i32 1) ; line:64 col:18
  %tmp68 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 10 ; line:65 col:36
  %tmp69 = load float, float* %tmp68, align 4 ; line:65 col:36
  %tmp70 = load i32, i32* %tmp, align 4 ; line:65 col:30
  %tmp71 = getelementptr inbounds <1 x float>, <1 x float>* @dygrec1.0, i32 0, i32 %tmp70 ; line:65 col:20
  store float %tmp69, float* %tmp71 ; line:65 col:34
  store float %tmp69, float* @stgrec1.0.0 ; line:65 col:18
  %tmp72 = getelementptr inbounds [12 x float], [12 x float]* %vals, i32 0, i32 11 ; line:66 col:36
  %tmp73 = load float, float* %tmp72, align 4 ; line:66 col:36
  %tmp74 = load i32, i32* %tmp, align 4 ; line:66 col:30
  %tmp75 = getelementptr inbounds <2 x float>, <2 x float>* @dygrec2.0, i32 0, i32 %tmp74 ; line:66 col:20
  store float %tmp73, float* %tmp75 ; line:66 col:34
  store float %tmp73, float* getelementptr inbounds (<2 x float>, <2 x float>* @stgrec2.0, i32 0, i32 1) ; line:66 col:18
  br label %bb76 ; line:67 col:3

bb76:                                             ; preds = %bb17, %bb
  %tmp77 = load <1 x float>, <1 x float>* %dyloc1, align 4 ; line:68 col:17
  %tmp78 = extractelement <1 x float> %tmp77, i32 0 ; line:68 col:17
  %tmp79 = load <2 x float>, <2 x float>* %dyloc2, align 4 ; line:68 col:27
  %tmp80 = extractelement <2 x float> %tmp79, i32 1 ; line:68 col:27
  %tmp81 = load <1 x float>, <1 x float>* %stloc1, align 4 ; line:68 col:37
  %tmp82 = extractelement <1 x float> %tmp81, i32 0 ; line:68 col:37
  %tmp83 = load <2 x float>, <2 x float>* %stloc2, align 4 ; line:68 col:47
  %tmp84 = extractelement <2 x float> %tmp83, i32 1 ; line:68 col:47
  %tmp85 = insertelement <4 x float> undef, float %tmp78, i64 0 ; line:68 col:16
  %tmp86 = insertelement <4 x float> %tmp85, float %tmp80, i64 1 ; line:68 col:16
  %tmp87 = insertelement <4 x float> %tmp86, float %tmp82, i64 2 ; line:68 col:16
  %tmp88 = insertelement <4 x float> %tmp87, float %tmp84, i64 3 ; line:68 col:16
  %tmp89 = load i32, i32* %tmp, align 4 ; line:68 col:73
  %tmp90 = load i32, i32* %tmp, align 4 ; line:68 col:77
  %tmp91 = getelementptr inbounds [3 x <1 x float>], [3 x <1 x float>]* %dylar1, i32 0, i32 %tmp89, i32 %tmp90 ; line:68 col:66
  %tmp92 = load float, float* %tmp91 ; line:68 col:66
  %tmp93 = load i32, i32* %tmp, align 4 ; line:68 col:89
  %tmp94 = load i32, i32* %tmp, align 4 ; line:68 col:93
  %tmp95 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %dylar2, i32 0, i32 %tmp93, i32 %tmp94 ; line:68 col:82
  %tmp96 = load float, float* %tmp95 ; line:68 col:82
  %tmp97 = getelementptr [3 x float], [3 x float]* %stlar1.0, i32 0, i32 0 ; line:68 col:98
  %load = load float, float* %tmp97 ; line:68 col:98
  %insert = insertelement <1 x float> undef, float %load, i64 0 ; line:68 col:98
  %tmp98 = extractelement <1 x float> %insert, i32 0 ; line:68 col:98
  %tmp99 = getelementptr inbounds [4 x <2 x float>], [4 x <2 x float>]* %stlar2, i32 0, i32 0 ; line:68 col:111
  %tmp100 = load <2 x float>, <2 x float>* %tmp99, align 4 ; line:68 col:111
  %tmp101 = extractelement <2 x float> %tmp100, i32 1 ; line:68 col:111
  %tmp102 = insertelement <4 x float> undef, float %tmp92, i64 0 ; line:68 col:65
  %tmp103 = insertelement <4 x float> %tmp102, float %tmp96, i64 1 ; line:68 col:65
  %tmp104 = insertelement <4 x float> %tmp103, float %tmp98, i64 2 ; line:68 col:65
  %tmp105 = insertelement <4 x float> %tmp104, float %tmp101, i64 3 ; line:68 col:65
  %tmp106 = fadd <4 x float> %tmp88, %tmp105 ; line:68 col:57
  %tmp107 = load <1 x float>, <1 x float>* @dyglob1, align 4 ; line:69 col:10
  %tmp108 = extractelement <1 x float> %tmp107, i32 0 ; line:69 col:10
  %tmp109 = load <2 x float>, <2 x float>* @dyglob2, align 4 ; line:69 col:21
  %tmp110 = extractelement <2 x float> %tmp109, i32 1 ; line:69 col:21
  %load3 = load float, float* @stglob1.0 ; line:69 col:32
  %insert4 = insertelement <1 x float> undef, float %load3, i64 0 ; line:69 col:32
  %tmp111 = extractelement <1 x float> %insert4, i32 0 ; line:69 col:32
  %tmp112 = load <2 x float>, <2 x float>* @stglob2, align 4 ; line:69 col:43
  %tmp113 = extractelement <2 x float> %tmp112, i32 1 ; line:69 col:43
  %tmp114 = insertelement <4 x float> undef, float %tmp108, i64 0 ; line:69 col:9
  %tmp115 = insertelement <4 x float> %tmp114, float %tmp110, i64 1 ; line:69 col:9
  %tmp116 = insertelement <4 x float> %tmp115, float %tmp111, i64 2 ; line:69 col:9
  %tmp117 = insertelement <4 x float> %tmp116, float %tmp113, i64 3 ; line:69 col:9
  %tmp118 = fadd <4 x float> %tmp106, %tmp117 ; line:68 col:124
  %tmp119 = load i32, i32* %tmp, align 4 ; line:69 col:70
  %tmp120 = load i32, i32* %tmp, align 4 ; line:69 col:74
  %tmp121 = getelementptr inbounds [2 x <1 x float>], [2 x <1 x float>]* @dygar1, i32 0, i32 %tmp119, i32 %tmp120 ; line:69 col:63
  %tmp122 = load float, float* %tmp121 ; line:69 col:63
  %tmp123 = load i32, i32* %tmp, align 4 ; line:69 col:86
  %tmp124 = load i32, i32* %tmp, align 4 ; line:69 col:90
  %tmp125 = getelementptr inbounds [3 x <2 x float>], [3 x <2 x float>]* @dygar2, i32 0, i32 %tmp123, i32 %tmp124 ; line:69 col:79
  %tmp126 = load float, float* %tmp125 ; line:69 col:79
  %load1 = load float, float* getelementptr inbounds ([2 x float], [2 x float]* @stgar1.0, i32 0, i32 0) ; line:69 col:95
  %insert2 = insertelement <1 x float> undef, float %load1, i64 0 ; line:69 col:95
  %tmp127 = extractelement <1 x float> %insert2, i32 0 ; line:69 col:95
  %tmp128 = load <2 x float>, <2 x float>* getelementptr inbounds ([3 x <2 x float>], [3 x <2 x float>]* @stgar2, i32 0, i32 0), align 4 ; line:69 col:108
  %tmp129 = extractelement <2 x float> %tmp128, i32 1 ; line:69 col:108
  %tmp130 = insertelement <4 x float> undef, float %tmp122, i64 0 ; line:69 col:62
  %tmp131 = insertelement <4 x float> %tmp130, float %tmp126, i64 1 ; line:69 col:62
  %tmp132 = insertelement <4 x float> %tmp131, float %tmp127, i64 2 ; line:69 col:62
  %tmp133 = insertelement <4 x float> %tmp132, float %tmp129, i64 3 ; line:69 col:62
  %tmp134 = fadd <4 x float> %tmp118, %tmp133 ; line:69 col:54
  %tmp135 = load <1 x float>, <1 x float>* %stlorc1.0, align 4 ; line:70 col:20
  %tmp136 = extractelement <1 x float> %tmp135, i64 0 ; line:70 col:11
  %tmp137 = getelementptr inbounds <2 x float>, <2 x float>* %stlorc2.0, i32 0, i32 1 ; line:70 col:23
  %tmp138 = load float, float* %tmp137 ; line:70 col:23
  %tmp139 = load <1 x float>, <1 x float>* %dylorc1.0, align 4 ; line:70 col:45
  %tmp140 = extractelement <1 x float> %tmp139, i64 0 ; line:70 col:11
  %tmp141 = load i32, i32* %tmp, align 4 ; line:70 col:58
  %tmp142 = getelementptr inbounds <2 x float>, <2 x float>* %dylorc2.0, i32 0, i32 %tmp141 ; line:70 col:48
  %tmp143 = load float, float* %tmp142 ; line:70 col:48
  %tmp144 = insertelement <4 x float> undef, float %tmp136, i64 0 ; line:70 col:11
  %tmp145 = insertelement <4 x float> %tmp144, float %tmp138, i64 1 ; line:70 col:11
  %tmp146 = insertelement <4 x float> %tmp145, float %tmp140, i64 2 ; line:70 col:11
  %tmp147 = insertelement <4 x float> %tmp146, float %tmp143, i64 3 ; line:70 col:11
  %tmp148 = fadd <4 x float> %tmp134, %tmp147 ; line:69 col:121
  %load5 = load float, float* @stgrec1.0.0 ; line:70 col:80
  %insert6 = insertelement <1 x float> undef, float %load5, i64 0 ; line:70 col:80
  %tmp149 = extractelement <1 x float> %insert6, i64 0 ; line:70 col:71
  %tmp150 = load float, float* getelementptr inbounds (<2 x float>, <2 x float>* @stgrec2.0, i32 0, i32 1) ; line:70 col:83
  %tmp151 = load <1 x float>, <1 x float>* @dygrec1.0, align 4 ; line:70 col:105
  %tmp152 = extractelement <1 x float> %tmp151, i64 0 ; line:70 col:71
  %tmp153 = load i32, i32* %tmp, align 4 ; line:70 col:118
  %tmp154 = getelementptr inbounds <2 x float>, <2 x float>* @dygrec2.0, i32 0, i32 %tmp153 ; line:70 col:108
  %tmp155 = load float, float* %tmp154 ; line:70 col:108
  %tmp156 = insertelement <4 x float> undef, float %tmp149, i64 0 ; line:70 col:71
  %tmp157 = insertelement <4 x float> %tmp156, float %tmp150, i64 1 ; line:70 col:71
  %tmp158 = insertelement <4 x float> %tmp157, float %tmp152, i64 2 ; line:70 col:71
  %tmp159 = insertelement <4 x float> %tmp158, float %tmp155, i64 3 ; line:70 col:71
  %tmp160 = fadd <4 x float> %tmp148, %tmp159 ; line:70 col:63
  ret <4 x float> %tmp160 ; line:68 col:3
}

attributes #0 = { nounwind }

!dx.version = !{!3}
!3 = !{i32 1, i32 9}
