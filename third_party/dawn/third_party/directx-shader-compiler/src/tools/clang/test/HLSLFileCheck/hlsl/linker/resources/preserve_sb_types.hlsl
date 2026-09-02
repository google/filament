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
// REFLECT:         D3D12_SHADER_BUFFER_DESC: Name: StructBuf
// REFLECT:           Type: D3D_CT_RESOURCE_BIND_INFO
// REFLECT:           Size: 80
// REFLECT:               D3D12_SHADER_TYPE_DESC: Name: Element
// REFLECT:                 Class: D3D_SVC_STRUCT
// REFLECT:                   D3D12_SHADER_TYPE_DESC: Name: float4x4
// REFLECT:                   D3D12_SHADER_TYPE_DESC: Name: float4
// REFLECT:     Bound Resources:
// REFLECT:       D3D12_SHADER_INPUT_BIND_DESC: Name: StructBuf
// REFLECT:         Type: D3D_SIT_STRUCTURED


// CHECK: %[[ResTy:[^ ]+]] = type { %[[ElTy:.*struct.Element]] }
// CHECK: %[[ElTy]] = type { {{\[4 x \<4 x float\>\]|\%class\.matrix\.float\.4\.4}}, <4 x float> }

// TBD: Why is resource global sometimes `global` and sometimes `constant`?
// LIBGV_ORIG: @[[GV:[^ ]+]] = external {{global|constant}} %[[ResTy]]
// LIBGV_HDL: @[[GV:[^ ]+]] = external {{global|constant}} %dx.types.Handle

// CHECK: !dx.resources = !{![[AllRes:[0-9]+]]}
// CHECK: !dx.typeAnnotations = !{![[TypeAnn:[0-9]+]],
// CHECK: ![[AllRes]] = !{![[AllSRVs:[0-9]+]], null, null, null}
// CHECK: ![[AllSRVs]] = !{![[Res_StructBuf:[0-9]+]]}

// LIBGV_ORIG: ![[Res_StructBuf]] = !{i32 0, %[[ResTy:[^*]+]]* @[[GV:[^,]+]], !"StructBuf", i32 -1, i32 -1, i32 1, i32 12, i32 0, ![[ExtraProps:[0-9]+]]}
// LIBGV_HDL: ![[Res_StructBuf]] = !{i32 0, %[[ResTy:[^*]+]]* bitcast (%dx.types.Handle* @[[GV:[^ ]+]] to %[[ResTy]]*), !"StructBuf", i32 -1, i32 -1, i32 1, i32 12, i32 0, ![[ExtraProps:[0-9]+]]}
// CHKSHAD: ![[Res_StructBuf]] = !{i32 0, %[[ResTy:[^*]+]]* undef, !"StructBuf", i32 0, i32 0, i32 1, i32 12, i32 0, ![[ExtraProps:[0-9]+]]}

// Struct stride:
// CHECK: ![[ExtraProps]] = !{i32 1, i32 80}

// CHECK: ![[TypeAnn]] =
// CHECK-SAME: %[[ElTy]] undef, ![[TypeAnn_ElTy:[0-9]+]]

// CHECK: ![[TypeAnn_ElTy]] = !{i32 80, ![[FieldAnn_m:[0-9]+]]
// CHECK: ![[FieldAnn_m]] = !{i32 6, !"m"

struct Element {
  row_major float4x4 m;
  float4 f;
};

StructuredBuffer<Element> StructBuf;

export float4 xform(float4 v) {
  return mul(v, StructBuf[0].m);
}

[shader("vertex")]
float4 main(float3 pos : Position) : SV_Position {
  return xform(float4(pos, 1)) * StructBuf[0].f;
}
