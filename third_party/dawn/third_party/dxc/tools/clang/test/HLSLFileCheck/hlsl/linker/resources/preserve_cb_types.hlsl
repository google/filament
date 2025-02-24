// RUN: %dxilver 1.7 | %dxc -T lib_6_5 -Fo dxl_6_5 %s | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG

// Why is GV not mutated to handle for lib_6_6+?
// RUN: %dxilver 1.7 | %dxc -T lib_6_6 -Fo dxl_6_6 %s | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -Fo dxl_6_7 %s | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG

// RUN: %dxilver 1.7 | %dxc -T lib_6_x -Fo dxl_6_x %s | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL

// Target vs_6_5
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD

// Target vs_6_6
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD

// Target vs_6_7
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,CHKSHAD

// Target lib_6_5
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_5 dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_5 dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_5 dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_5 dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG

// Target lib_6_6
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_6 dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_6 dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_6 dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL

// TBD: Why isn't resource GV type mutated to handle when going through lib_6_x?
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_6 dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG

// Target lib_6_7
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_7 dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_7 dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_7 dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL

// TBD: Why isn't resource GV type mutated to handle when going through lib_6_x?
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_7 dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_ORIG

// Target lib_6_x
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_x dxl_6_5 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_x dxl_6_6 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_x dxl_6_7 %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL
// RUN: %dxilver 1.7 | %dxl -E main -T lib_6_x dxl_6_x %s  | FileCheck %s -check-prefixes=CHECK,LIBGV_HDL



// RUN: %dxilver 1.7 | %dxc -T lib_6_5 -Fo dxl_6_5 %s | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxc -T lib_6_6 -Fo dxl_6_6 %s | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxc -T lib_6_7 -Fo dxl_6_7 %s | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxc -T lib_6_x -Fo dxl_6_x %s | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT

// Target vs_6_5
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_5 dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT

// Target vs_6_6
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_6 dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT

// Target vs_6_7
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT
// RUN: %dxilver 1.7 | %dxl -E main -T vs_6_7 dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECT

// Target lib_6_5
// RUN: %dxilver 1.7 | %dxl -T lib_6_5 dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_5 dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_5 dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_5 dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT

// Target lib_6_6
// RUN: %dxilver 1.7 | %dxl -T lib_6_6 dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_6 dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_6 dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_6 dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT

// Target lib_6_7
// RUN: %dxilver 1.7 | %dxl -T lib_6_7 dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_7 dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_7 dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_7 dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT

// Target lib_6_x
// RUN: %dxilver 1.7 | %dxl -T lib_6_x dxl_6_5 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_x dxl_6_6 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_x dxl_6_7 %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT
// RUN: %dxilver 1.7 | %dxl -T lib_6_x dxl_6_x %s  | %D3DReflect %s | FileCheck %s -check-prefixes=REFLECTLIB,REFLECT

// REFLECTLIB: D3D12_FUNCTION_DESC: Name: main
// REFLECTLIB: Shader Version: Vertex
// REFLECT:     Constant Buffers:
// REFLECT:       ID3D12ShaderReflectionConstantBuffer:
// REFLECT:         D3D12_SHADER_BUFFER_DESC: Name: CB0
// REFLECT:           Type: D3D_CT_CBUFFER
// REFLECT:           Size: 80
// REFLECT:           Num Variables: 2
// REFLECT:           ID3D12ShaderReflectionVariable:
// REFLECT:             D3D12_SHADER_VARIABLE_DESC: Name: m
// REFLECT:               uFlags: (D3D_SVF_USED)
// REFLECT:             ID3D12ShaderReflectionType:
// REFLECT:               D3D12_SHADER_TYPE_DESC: Name: float4x4
// REFLECT:                 Class: D3D_SVC_MATRIX_ROWS
// REFLECT:                 Type: D3D_SVT_FLOAT
// REFLECT:           ID3D12ShaderReflectionVariable:
// REFLECT:             D3D12_SHADER_VARIABLE_DESC: Name: f
// REFLECT:               Size: 16
// REFLECT:               StartOffset: 64
// REFLECT:             ID3D12ShaderReflectionType:
// REFLECT:               D3D12_SHADER_TYPE_DESC: Name: float4
// REFLECT:                 Class: D3D_SVC_VECTOR
// REFLECT:                 Type: D3D_SVT_FLOAT
// REFLECT:     Bound Resources:
// REFLECT:       D3D12_SHADER_INPUT_BIND_DESC: Name: CB0
// REFLECT:         Type: D3D_SIT_CBUFFER


// GVHLSL: %[[ResTy:[^ ]]] = type { {{\[4 x \<4 x float\>\]|\%class\.matrix\.float\.4\.4}}, <4 x float> }

// GVHLSL: @[[GV:[^ ]]] = external global %[[ResTy]]
// GVHDL: @[[GV:[^ ]]] = external constant dx.types.Handle

// CHECK: !dx.resources = !{![[AllRes:[0-9]+]]}
// CHECK: !dx.typeAnnotations = !{![[TypeAnn:[0-9]+]],
// CHECK: ![[AllRes]] = !{null, null, ![[AllCBs:[0-9]+]], null}
// CHECK: ![[AllCBs]] = !{![[Res_CB0:[0-9]+]]}

// LIBGV_ORIG: ![[Res_CB0]] = !{i32 0, %[[ResTy:[^*]+]]* @[[GV:[^,]+]], !"CB0", i32 -1, i32 -1, i32 1, i32 80, null}
// LIBGV_HDL: ![[Res_CB0]] = !{i32 0, %[[ResTy:[^*]+]]* bitcast (%dx.types.Handle* @[[GV:[^ ]+]] to %[[ResTy]]*), !"CB0", i32 -1, i32 -1, i32 1, i32 80, null}
// CHKSHAD: ![[Res_CB0]] = !{i32 0, %[[ResTy:[^*]+]]* undef, !"CB0", i32 0, i32 0, i32 1, i32 80, null}

// CHECK: ![[TypeAnn]] =
// CHECK-SAME: %[[ResTy]] undef, ![[TypeAnn_CB0:[0-9]+]]

// CHECK: ![[TypeAnn_CB0]] = !{i32 80, ![[FieldAnn_m:[0-9]+]]
// CHECK: ![[FieldAnn_m]] = !{i32 6, !"m", i32 2, !{{[0-9]+}}, i32 3, i32 0, i32 7, i32 9, i32 9, i1 true}

cbuffer CB0 {
  row_major float4x4 m;
  float4 f;
}

export float4 xform(float4 v) {
  return mul(v, m);
}

[shader("vertex")]
float4 main(float3 pos : Position) : SV_Position {
  return xform(float4(pos, 1)) * f;
}
