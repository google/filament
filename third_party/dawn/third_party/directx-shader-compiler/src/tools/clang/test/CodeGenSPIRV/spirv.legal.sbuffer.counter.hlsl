// RUN: %dxc -T ps_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

struct S1 {
    float4 f;
};

struct S2 {
    float3 f;
};

struct S3 {
    float2 f;
};

// Do not generate decorations for alias buffers

// CHECK-NOT: OpDecorateId %staticgRWSBuffer HlslCounterBufferGOOGLE
// CHECK-NOT: OpDecorateId %staticgASBuffer HlslCounterBufferGOOGLE
// CHECK-NOT: OpDecorateId %staticgCSBuffer HlslCounterBufferGOOGLE

// CHECK-NOT: OpDecorateId %localRWSBuffer HlslCounterBufferGOOGLE
// CHECK-NOT: OpDecorateId %localASBuffer HlslCounterBufferGOOGLE
// CHECK-NOT: OpDecorateId %localCSBuffer HlslCounterBufferGOOGLE

RWStructuredBuffer<S1>      selectRWSBuffer(RWStructuredBuffer<S1>    paramRWSBuffer, bool selector);
AppendStructuredBuffer<S2>  selectASBuffer(AppendStructuredBuffer<S2>  paramASBuffer,  bool selector);
ConsumeStructuredBuffer<S3> selectCSBuffer(ConsumeStructuredBuffer<S3> paramCSBuffer,  bool selector);

// CHECK: %counter_var_globalRWSBuffer = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
RWStructuredBuffer<S1>      globalRWSBuffer;
// CHECK: %counter_var_globalASBuffer = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
AppendStructuredBuffer<S2>  globalASBuffer;
// CHECK: %counter_var_globalCSBuffer = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
ConsumeStructuredBuffer<S3> globalCSBuffer;

// Counter variables for global static variables have an extra level of pointer.
// CHECK: %counter_var_staticgRWSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
static RWStructuredBuffer<S1>      staticgRWSBuffer = globalRWSBuffer;
// CHECK: %counter_var_staticgASBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
static AppendStructuredBuffer<S2>  staticgASBuffer  = globalASBuffer;
// CHECK: %counter_var_staticgCSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
static ConsumeStructuredBuffer<S3> staticgCSBuffer  = globalCSBuffer;

// Counter variables for function returns, function parameters, and local variables have an extra level of pointer.
// CHECK:      %counter_var_paramRWSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_selectRWSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_localRWSBufferMain = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_paramCSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_selectCSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_localASBufferMain = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_paramASBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_selectASBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_localRWSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_localCSBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK-NEXT: %counter_var_localASBuffer = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// Counter variables for global static variables are initialized.
// CHECK: %main = OpFunction
// CHECK: OpStore %counter_var_staticgRWSBuffer %counter_var_globalRWSBuffer
// CHECK: OpStore %counter_var_staticgASBuffer %counter_var_globalASBuffer
// CHECK: OpStore %counter_var_staticgCSBuffer %counter_var_globalCSBuffer

// CHECK: %src_main = OpFunction
float4 main() : SV_Target {
// Update the counter variable associated with the parameter
// CHECK:      [[ptr:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticgRWSBuffer
// CHECK-NEXT:                OpStore %counter_var_paramRWSBuffer [[ptr]]
    selectRWSBuffer(staticgRWSBuffer, true)
// Use the counter variable associated with the function
// CHECK:      [[ptr_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_selectRWSBuffer
// CHECK-NEXT:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_int [[ptr_0]] %uint_0
        .IncrementCounter();

// Update the counter variable associated with the parameter
// CHECK:                     OpStore %counter_var_paramRWSBuffer %counter_var_globalRWSBuffer
// Update the counter variable associated with the lhs of the assignment
// CHECK:      [[ptr_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_selectRWSBuffer
// CHECK-NEXT:                OpStore %counter_var_localRWSBufferMain [[ptr_1]]
    RWStructuredBuffer<S1> localRWSBufferMain = selectRWSBuffer(globalRWSBuffer, true);

// Use the counter variable associated with the local variable
// CHECK:      [[ptr_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localRWSBufferMain
// CHECK-NEXT:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_int [[ptr_2]] %uint_0
    localRWSBufferMain.DecrementCounter();

// Update the counter variable associated with the parameter
// CHECK:                      OpStore %counter_var_paramCSBuffer %counter_var_globalCSBuffer
// CHECK:      [[call:%[0-9]+]] = OpFunctionCall %_ptr_Uniform_type_ConsumeStructuredBuffer_S3 %selectCSBuffer
    S3 val3 = selectCSBuffer(globalCSBuffer, true)
// CHECK-NEXT: [[ptr1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_selectCSBuffer
// CHECK-NEXT: [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[ptr1]] %uint_0
// CHECK-NEXT: [[prev:%[0-9]+]] = OpAtomicISub %int [[ptr2]] %uint_1 %uint_0 %int_1
// CHECK-NEXT:  [[idx:%[0-9]+]] = OpISub %int [[prev]] %int_1
// CHECK-NEXT: [[ptr3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S3 [[call]] %uint_0 [[idx]]
// CHECK-NEXT:  [[val:%[0-9]+]] = OpLoad %S3 [[ptr3]]
// CHECK-NEXT:  [[vec:%[0-9]+]] = OpCompositeExtract %v2float [[val]] 0
// CHECK-NEXT:  [[tmp:%[0-9]+]] = OpCompositeConstruct %S3_0 [[vec]]
// CHECK-NEXT:                 OpStore %val3 [[tmp]]
        .Consume();

    float3 vec = float3(val3.f, 1.0);
    S2 val2 = {vec};

// Update the counter variable associated with the parameter
// CHECK:      [[ptr_3:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticgASBuffer
// CHECK-NEXT:                OpStore %counter_var_paramASBuffer [[ptr_3]]

// CHECK:     [[call_0:%[0-9]+]] = OpFunctionCall %_ptr_Uniform_type_AppendStructuredBuffer_S2 %selectASBuffer %param_var_paramASBuffer %param_var_selector_2
// CHECK-NEXT:                OpStore %localASBufferMain [[call_0]]
    AppendStructuredBuffer<S2> localASBufferMain = selectASBuffer(staticgASBuffer, false);
// Use the counter variable associated with the local variable
// CHECK-NEXT: [[ptr_4:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_selectASBuffer
// CHECK-NEXT:                OpStore %counter_var_localASBufferMain [[ptr_4]]

// CHECK-NEXT: [[ptr1_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_AppendStructuredBuffer_S2 %localASBufferMain
// CHECK-NEXT: [[ptr2_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localASBufferMain
// CHECK-NEXT: [[ptr3_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[ptr2_0]] %uint_0
// CHECK-NEXT:  [[idx_0:%[0-9]+]] = OpAtomicIAdd %int [[ptr3_0]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[ptr4:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S2 [[ptr1_0]] %uint_0 [[idx_0]]
// CHECK-NEXT:  [[val_0:%[0-9]+]] = OpLoad %S2_0 %val2
// CHECK-NEXT:  [[vec_0:%[0-9]+]] = OpCompositeExtract %v3float [[val_0]] 0
// CHECK-NEXT:  [[tmp_0:%[0-9]+]] = OpCompositeConstruct %S2 [[vec_0]]
// CHECK-NEXT:                 OpStore [[ptr4]] [[tmp_0]]
    localASBufferMain.Append(val2);

    return float4(val2, 2.0);
}

RWStructuredBuffer<S1>      selectRWSBuffer(RWStructuredBuffer<S1>    paramRWSBuffer, bool selector) {
// CHECK: OpStore %counter_var_localRWSBuffer %counter_var_globalRWSBuffer
    RWStructuredBuffer<S1>      localRWSBuffer = globalRWSBuffer;
    if (selector)
// Use the counter variable associated with the function
// CHECK:      [[ptr_5:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_paramRWSBuffer
// CHECK-NEXT:                OpStore %counter_var_selectRWSBuffer [[ptr_5]]
// CHECK:                     OpReturnValue
        return paramRWSBuffer;
    else
// Use the counter variable associated with the function
// CHECK:      [[ptr_6:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localRWSBuffer
// CHECK-NEXT:                OpStore %counter_var_selectRWSBuffer [[ptr_6]]
// CHECK:                     OpReturnValue
        return localRWSBuffer;
}

ConsumeStructuredBuffer<S3> selectCSBuffer(ConsumeStructuredBuffer<S3> paramCSBuffer,  bool selector) {
// CHECK: OpStore %counter_var_localCSBuffer %counter_var_globalCSBuffer
    ConsumeStructuredBuffer<S3> localCSBuffer  = globalCSBuffer;
    if (selector)
// Use the counter variable associated with the function
// CHECK:      [[ptr_7:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_paramCSBuffer
// CHECK-NEXT:                OpStore %counter_var_selectCSBuffer [[ptr_7]]
// CHECK:                     OpReturnValue
        return paramCSBuffer;
    else
// Use the counter variable associated with the function
// CHECK:      [[ptr_8:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localCSBuffer
// CHECK-NEXT:                OpStore %counter_var_selectCSBuffer [[ptr_8]]
// CHECK:                     OpReturnValue
        return localCSBuffer;
}

AppendStructuredBuffer<S2>  selectASBuffer(AppendStructuredBuffer<S2>  paramASBuffer,  bool selector) {
// CHECK: OpStore %counter_var_localASBuffer %counter_var_globalASBuffer
    AppendStructuredBuffer<S2>  localASBuffer  = globalASBuffer;
    if (selector)
// Use the counter variable associated with the function
// CHECK:      [[ptr_9:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_paramASBuffer
// CHECK-NEXT:                OpStore %counter_var_selectASBuffer [[ptr_9]]
// CHECK:                     OpReturnValue
        return paramASBuffer;
    else
// Use the counter variable associated with the function
// CHECK:      [[ptr_10:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localASBuffer
// CHECK-NEXT:                OpStore %counter_var_selectASBuffer [[ptr_10]]
// CHECK:                     OpReturnValue
        return localASBuffer;
}
