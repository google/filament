// RUN: %dxc -T ps_6_0 -E main -Zi %s | FileCheck %s

// Make sure llvm.dbg.value exist.
// CHECK: call void @llvm.dbg.value

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

typedef float4 MyCoolFloat4;

static float4 myStaticGlobalVar = float4(1.0, 1.0, 1.0, 1.0);


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
    return float4(result[0], result[1], result[2], result[3]);
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

float4 main( float4 unused : SV_POSITION, float4 color : COLOR ) : SV_Target
{
    float4 ret1 = localScopeVar_func(color);
    float4 ret2 = localRegVar_func(ret1);
    float4 ret3 = array_func(ret2);
    float4 ret4 = typedef_func(ret3);
    float4 ret5 = global_func(ret4);
    float4 ret6 = depth2(ret5);
    return max(ret6, color);
}