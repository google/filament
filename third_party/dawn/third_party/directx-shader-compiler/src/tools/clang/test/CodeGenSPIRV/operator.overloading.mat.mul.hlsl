// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

struct GLFloat3x2 {
    float3x2 m;
};

struct GLFloat3x4 {
    float3x4 m;
};

struct GLFloat3x3 {
    float3x3 m;

    float3x3 operator*(GLFloat3x3 x) {
        return mul(m, x.m);
    }

    float3x2 operator*(GLFloat3x2 x) {
        return mul(m, x.m);
    }

    float3x4 operator*(GLFloat3x4 x) {
        return mul(m, x.m);
    }

    float3 operator[](int x) {
        return m[x];
    }
};

float3 main(float4 pos: SV_Position) : SV_Target {
    GLFloat3x3 a = {pos.xyz, pos.xyz, pos.xyz,};
    GLFloat3x3 b = {pos.xxx, pos.yyy, pos.zzz,};
    GLFloat3x2 c = {pos.xy, pos.yz, pos.zx,};
    GLFloat3x4 d = {pos, pos, pos,};

// CHECK: [[d:%[a-zA-Z0-9_]+]] = OpLoad %GLFloat3x4 %d
// CHECK:              OpStore %param_var_x [[d]]
// CHECK:              OpFunctionCall %mat3v4float %GLFloat3x3_operator_Star %a %param_var_x
    a * d;

// CHECK: [[c:%[a-zA-Z0-9_]+]] = OpLoad %GLFloat3x2 %c
// CHECK:              OpStore %param_var_x_0 [[c]]
// CHECK:              OpFunctionCall %mat3v2float %GLFloat3x3_operator_Star_0 %b %param_var_x_0
    b * c;

// CHECK: [[b:%[a-zA-Z0-9_]+]] = OpLoad %GLFloat3x3 %b
// CHECK:              OpStore %param_var_x_1 [[b]]
// CHECK:              OpFunctionCall %mat3v3float %GLFloat3x3_operator_Star_1 %a %param_var_x_1
    a * b;

// CHECK: OpStore %param_var_x_2 %int_1
// CHECK: OpFunctionCall %v3float %GLFloat3x3_operator_Subscript %a %param_var_x_2
    return a[1];
}

// CHECK:                 OpFunction %mat3v4float None
// CHECK: [[this:%[a-zA-Z0-9_]+]] = OpLoad %mat3v3float
// CHECK:    [[x:%[a-zA-Z0-9_]+]] = OpLoad %mat3v4float
// CHECK:                 OpMatrixTimesMatrix %mat3v4float [[x]] [[this]]

// CHECK:                 OpFunction %mat3v2float None
// CHECK: [[this_0:%[a-zA-Z0-9_]+]] = OpLoad %mat3v3float
// CHECK:    [[x_0:%[a-zA-Z0-9_]+]] = OpLoad %mat3v2float
// CHECK:                 OpMatrixTimesMatrix %mat3v2float [[x_0]] [[this_0]]

// CHECK:                 OpFunction %mat3v3float None
// CHECK: [[this_1:%[a-zA-Z0-9_]+]] = OpLoad %mat3v3float
// CHECK:    [[x_1:%[a-zA-Z0-9_]+]] = OpLoad %mat3v3float
// CHECK:                 OpMatrixTimesMatrix %mat3v3float [[x_1]] [[this_1]]

// CHECK: %GLFloat3x3_operator_Subscript = OpFunction %v3float None
// CHECK:                     [[x_2:%[a-zA-Z0-9_]+]] = OpLoad %int
// CHECK:                    [[ux:%[a-zA-Z0-9_]+]] = OpBitcast %uint [[x_2]]
// CHECK:                  [[this_2:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Function_v3float {{%[a-zA-Z0-9_]+}} %int_0 [[ux]]
// CHECK:                                  OpLoad %v3float [[this_2]]
