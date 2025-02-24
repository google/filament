// RUN: %dxc -T lib_6_6 -fcgl  %s -spirv | FileCheck %s

struct Inner {
    float2 cull2 : SV_CullDistance2;            // Builtin CullDistance
    float3 foo   : FOO;                         // Input variable
};

struct PsIn {
    float4 pos   : SV_Position;                 // Builtin FragCoord
    float2 clip0 : SV_ClipDistance0;            // Builtin ClipDistance
    Inner  s;
};

// CHECK: OpEntryPoint Fragment %entry "entry"
// CHECK-DAG: %gl_ClipDistance
// CHECK-DAG: %gl_CullDistance
// CHECK-DAG: %gl_FragCoord
// CHECK-DAG: %in_var_FOO
// CHECK-DAG: %in_var_BAR
// CHECK-DAG: %out_var_SV_Target

// CHECK: OpEntryPoint Fragment %entry_with_same_interfaces "entry_with_same_interfaces"
// CHECK-DAG: %gl_ClipDistance
// CHECK-DAG: %gl_CullDistance
// CHECK-DAG: %gl_FragCoord
// CHECK-DAG: %in_var_FOO_0
// CHECK-DAG: %in_var_BAR_0
// CHECK-DAG: %out_var_SV_Target_0

// CHECK: OpEntryPoint Fragment %entry_with_slightly_different_interfaces "entry_with_slightly_different_interfaces"
// CHECK-DAG: %gl_ClipDistance
// CHECK-DAG: %gl_CullDistance
// CHECK-DAG: %gl_FragCoord
// CHECK-DAG: %in_var_ZOO
// CHECK-DAG: %out_var_SV_TARGET0

// CHECK: OpEntryPoint Fragment %entry_with_completely_different_interfaces "entry_with_completely_different_interfaces"
// CHECK-DAG: %gl_ClipDistance
// CHECK-DAG: %gl_CullDistance
// CHECK-DAG: %in_var_COLOR
// CHECK-DAG: %in_var_X
// CHECK-DAG: %in_var_Y
// CHECK-DAG: %in_var_Z
// CHECK-DAG: %out_var_SV_TARGET1

[shader("pixel")]
float4 entry(
               PsIn   psIn,
               float  clip1 : SV_ClipDistance1, // Builtin ClipDistance
               float3 cull1 : SV_CullDistance1, // Builtin CullDistance
            in float  bar   : BAR               // Input variable
           ) : SV_Target {                      // Output variable
    return psIn.pos + float4(clip1 + bar, cull1);
}

[shader("pixel")]
float4 entry_with_same_interfaces(
               PsIn   psIn,
               float  clip1 : SV_ClipDistance1, // Builtin ClipDistance
               float3 cull1 : SV_CullDistance1, // Builtin CullDistance
            in float  bar   : BAR               // Input variable
           ) : SV_Target {                      // Output variable
    return psIn.pos + float4(clip1 + bar, cull1);
}

[shader("pixel")]
float4 entry_with_slightly_different_interfaces(
               float4 pos   : SV_Position,
               float  clip2 : SV_ClipDistance2, // Builtin ClipDistance
               float3 cull4 : SV_CullDistance4, // Builtin CullDistance
            in float  zoo   : ZOO               // Input variable
           ) : SV_TARGET0 {                     // Output variable
    return pos + float4(clip2 + zoo, cull4);
}

[shader("pixel")]
float4 entry_with_completely_different_interfaces(
               float4 color : COLOR,
               float  clip3 : SV_ClipDistance3, // Builtin ClipDistance
            in float  x     : X,                // Input variable
            in float  y     : Y,                // Input variable
            in float  z     : Z                 // Input variable
           ) : SV_TARGET1 {                     // Output variable
    return color + float4(clip3, x, y, z);
}
