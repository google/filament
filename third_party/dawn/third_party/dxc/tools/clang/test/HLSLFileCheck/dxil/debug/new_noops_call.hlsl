// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

typedef float4 MyCoolFloat4; 
static float4 myStaticGlobalVar = float4(1.0, 1.0, 1.0, 1.0);

cbuffer cb : register(b0) {
  uint indices[4];
}

// Local var with same name as outer scope
float4 localScopeVar_func(float4 val)
{
    float4 color = val * val;
    return color;
}

// Local var with same name as register
float4 localRegVar_func(float4 val)
{
    float4 r1 = val;
    return r1;
}

// Array
float4 array_func(float4 val)
{
    float result[4];
    result[0] = val.x;
    result[1] = val.y;
    result[2] = val.z;
    result[3] = val.w;
    return float4(result[indices[0]], result[indices[1]], result[indices[2]], result[indices[3]]);
}

// Typedef
float4 typedef_func(float4 val)
{
    MyCoolFloat4 result = val;
    return result;
}

// Global
float4 global_func(float4 val)
{
    myStaticGlobalVar *= val;
    return myStaticGlobalVar;
}

float4 depth4(float4 val)
{
    val = val * val;
    return val;
}

float4 depth3(float4 val)
{
    val = depth4(val) * val;
    return val;
}

float4 depth2(float4 val)
{
    val = depth3(val) * val;
    return val;
}

[RootSignature("CBV(b0)")]
float4 main( float4 unused : SV_POSITION, float4 color : COLOR ) : SV_Target
{
    // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
    // xHECK-SAME: @dx.preserve.value
    // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1
    float4 ret1 = localScopeVar_func(color);
    // ** call **
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // CHECK: %[[v1:.+]] = fmul
    // CHECK: %[[v2:.+]] = fmul
    // CHECK: %[[v3:.+]] = fmul
    // CHECK: %[[v4:.+]] = fmul
    // xHECK: select i1 %[[p]], float %[[v1]], float %[[v1]]
    // xHECK: select i1 %[[p]], float %[[v2]], float %[[v2]]
    // xHECK: select i1 %[[p]], float %[[v3]], float %[[v3]]
    // xHECK: select i1 %[[p]], float %[[v4]], float %[[v4]]
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // ** return **

    float4 ret2 = localRegVar_func(ret1);
    // ** call **
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // ** copy **
    // xHECK: select i1 %[[p]],
    // xHECK: select i1 %[[p]],
    // xHECK: select i1 %[[p]],
    // xHECK: select i1 %[[p]],
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // ** return **

    float4 ret3 = array_func(ret2);
    // ** call **
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // CHECK: store
    // CHECK: store
    // CHECK: store
    // CHECK: store
    // CHECK: load
    // CHECK: load
    // CHECK: load
    // CHECK: load
    // ** return **

    float4 ret4 = typedef_func(ret3);
    // ** call **
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // ** copy **
    // xHECK: select i1 %[[p]], float %{{.+}}
    // xHECK: select i1 %[[p]], float %{{.+}}
    // xHECK: select i1 %[[p]], float %{{.+}}
    // xHECK: select i1 %[[p]], float %{{.+}}
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // ** return **

    float4 ret5 = global_func(ret4);
    // ** call **
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // CHECK: %[[a1:.+]] = fmul
    // CHECK: %[[a2:.+]] = fmul
    // CHECK: %[[a3:.+]] = fmul
    // CHECK: %[[a4:.+]] = fmul
    // xHECK: select i1 %[[p]], float %[[a1]], float %[[a1]]
    // xHECK: select i1 %[[p]], float %[[a2]], float %[[a2]]
    // xHECK: select i1 %[[p]], float %[[a3]], float %[[a3]]
    // xHECK: select i1 %[[p]], float %[[a4]], float %[[a4]]
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // ** return **

    float4 ret6 = depth2(ret5);
    // ** call **
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // depth2() {
      // ** call **
      // CHECK: load i32, i32*
      // CHECK-SAME: @dx.nothing
      // depth3() {
        // ** call **
        // CHECK: load i32, i32*
        // CHECK-SAME: @dx.nothing
        // depth4() {
          // CHECK: %[[b1:.+]] = fmul
          // CHECK: %[[b2:.+]] = fmul
          // CHECK: %[[b3:.+]] = fmul
          // CHECK: %[[b4:.+]] = fmul
          // CHECK: load i32, i32*
          // CHECK-SAME: @dx.nothing
        // }
        // CHECK: %[[c1:.+]] = fmul
        // CHECK: %[[c2:.+]] = fmul
        // CHECK: %[[c3:.+]] = fmul
        // CHECK: %[[c4:.+]] = fmul
        // CHECK: load i32, i32*
        // CHECK-SAME: @dx.nothing
      // }
      // CHECK: %[[d1:.+]] = fmul
      // CHECK: %[[d2:.+]] = fmul
      // CHECK: %[[d3:.+]] = fmul
      // CHECK: %[[d4:.+]] = fmul
    // }
    // xHECK: select i1 %[[p]], float %{{.+}}, float %[[d1]]
    // xHECK: select i1 %[[p]], float %{{.+}}, float %[[d2]]
    // xHECK: select i1 %[[p]], float %{{.+}}, float %[[d3]]
    // xHECK: select i1 %[[p]], float %{{.+}}, float %[[d4]]
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing

    return max(ret6, color);
    // CHECK: call float @dx.op.binary.f32(i32 35
    // CHECK: call float @dx.op.binary.f32(i32 35
    // CHECK: call float @dx.op.binary.f32(i32 35
    // CHECK: call float @dx.op.binary.f32(i32 35

}

