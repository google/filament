// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Bundle {
      RWStructuredBuffer<float> rw;
  AppendStructuredBuffer<float> append;
 ConsumeStructuredBuffer<float> consume;
};

struct TwoBundle {
    Bundle b1;
    Bundle b2;
};

struct Wrapper {
    TwoBundle b;
};

      RWStructuredBuffer<float> globalRWSBuffer;
  AppendStructuredBuffer<float> globalASBuffer;
 ConsumeStructuredBuffer<float> globalCSBuffer;

Bundle  CreateBundle();
Wrapper CreateWrapper();
Wrapper ReturnWrapper(Wrapper wrapper);

// Static variable
static Bundle  staticBundle  = CreateBundle();
static Wrapper staticWrapper = CreateWrapper();

// CHECK: %counter_var_staticBundle_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticBundle_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticBundle_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_staticWrapper_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticWrapper_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticWrapper_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticWrapper_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticWrapper_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_staticWrapper_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_CreateBundle_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateBundle_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateBundle_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_CreateWrapper_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateWrapper_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateWrapper_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateWrapper_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateWrapper_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_CreateWrapper_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_localWrapper_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_localWrapper_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_localWrapper_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_localWrapper_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_localWrapper_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_localWrapper_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_wrapper_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_wrapper_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_wrapper_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_wrapper_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_wrapper_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_wrapper_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_ReturnWrapper_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_ReturnWrapper_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_ReturnWrapper_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_ReturnWrapper_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_ReturnWrapper_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_ReturnWrapper_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_b_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_b_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_b_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK: %counter_var_w_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_w_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_w_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_w_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_w_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_w_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// CHECK-LABEL: %main = OpFunction

    // Assign to static variable
// CHECK:      [[src:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_CreateBundle_0
// CHECK-NEXT:                OpStore %counter_var_staticBundle_0 [[src]]
// CHECK-NEXT: [[src_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_CreateBundle_1
// CHECK-NEXT:                OpStore %counter_var_staticBundle_1 [[src_0]]
// CHECK-NEXT: [[src_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_CreateBundle_2
// CHECK-NEXT:                OpStore %counter_var_staticBundle_2 [[src_1]]

// CHECK-LABEL: %src_main = OpFunction
float main() : VALUE {
    // Assign to the parameter
// CHECK:      [[src_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticWrapper_0_0_0
// CHECK-NEXT:                OpStore %counter_var_wrapper_0_0_0 [[src_2]]
// CHECK-NEXT: [[src_3:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticWrapper_0_0_1
// CHECK-NEXT:                OpStore %counter_var_wrapper_0_0_1 [[src_3]]
// CHECK-NEXT: [[src_4:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticWrapper_0_0_2
// CHECK-NEXT:                OpStore %counter_var_wrapper_0_0_2 [[src_4]]
// CHECK-NEXT: [[src_5:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticWrapper_0_1_0
// CHECK-NEXT:                OpStore %counter_var_wrapper_0_1_0 [[src_5]]
// CHECK-NEXT: [[src_6:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticWrapper_0_1_1
// CHECK-NEXT:                OpStore %counter_var_wrapper_0_1_1 [[src_6]]
// CHECK-NEXT: [[src_7:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticWrapper_0_1_2
// CHECK-NEXT:                OpStore %counter_var_wrapper_0_1_2 [[src_7]]
    // Make the call
// CHECK:          {{%[0-9]+}} = OpFunctionCall %Wrapper %ReturnWrapper %param_var_wrapper
    // Assign to the return value
// CHECK:      [[src_8:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_0_0
// CHECK-NEXT:                OpStore %counter_var_localWrapper_0_0_0 [[src_8]]
// CHECK-NEXT: [[src_9:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_0_1
// CHECK-NEXT:                OpStore %counter_var_localWrapper_0_0_1 [[src_9]]
// CHECK-NEXT: [[src_10:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_0_2
// CHECK-NEXT:                OpStore %counter_var_localWrapper_0_0_2 [[src_10]]
// CHECK-NEXT: [[src_11:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_1_0
// CHECK-NEXT:                OpStore %counter_var_localWrapper_0_1_0 [[src_11]]
// CHECK-NEXT: [[src_12:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_1_1
// CHECK-NEXT:                OpStore %counter_var_localWrapper_0_1_1 [[src_12]]
// CHECK-NEXT: [[src_13:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_1_2
// CHECK-NEXT:                OpStore %counter_var_localWrapper_0_1_2 [[src_13]]
    Wrapper localWrapper = ReturnWrapper(staticWrapper);

// CHECK:      [[cnt:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_0
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[cnt]] %uint_0
// CHECK-NEXT:     {{%[0-9]+}} = OpAtomicIAdd %int [[ptr]] %uint_1 %uint_0 %int_1
    localWrapper.b.b1.rw.IncrementCounter();
// CHECK:      [[cnt_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_1
// CHECK-NEXT: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[cnt_0]] %uint_0
// CHECK-NEXT: [[add:%[0-9]+]] = OpAtomicIAdd %int [[ptr_0]] %uint_1 %uint_0 %int_1
// CHECK-NEXT:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_float {{%[0-9]+}} %uint_0 [[add]]
    localWrapper.b.b2.append.Append(5.0);

// CHECK:      [[cnt_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_ReturnWrapper_0_0_2
// CHECK-NEXT: [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[cnt_1]] %uint_0
// CHECK-NEXT: [[sub:%[0-9]+]] = OpAtomicISub %int [[ptr_1]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[pre:%[0-9]+]] = OpISub %int [[sub]] %int_1
// CHECK-NEXT:     {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_float {{%[0-9]+}} %uint_0 [[pre]]
    return ReturnWrapper(staticWrapper).b.b1.consume.Consume();
}

// CHECK-LABEL: %CreateBundle = OpFunction
Bundle CreateBundle() {
    Bundle b;
    // Assign to final struct fields who have associated counters
// CHECK: OpStore %counter_var_b_0 %counter_var_globalRWSBuffer
    b.rw      = globalRWSBuffer;
// CHECK: OpStore %counter_var_b_1 %counter_var_globalASBuffer
    b.append  = globalASBuffer;
// CHECK: OpStore %counter_var_b_2 %counter_var_globalCSBuffer
    b.consume = globalCSBuffer;

    // Assign from local variable
// CHECK:      [[src_14:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_b_0
// CHECK-NEXT:                OpStore %counter_var_CreateBundle_0 [[src_14]]
// CHECK-NEXT: [[src_15:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_b_1
// CHECK-NEXT:                OpStore %counter_var_CreateBundle_1 [[src_15]]
// CHECK-NEXT: [[src_16:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_b_2
// CHECK-NEXT:                OpStore %counter_var_CreateBundle_2 [[src_16]]
    return b;
}

// CHECK-LABEL: %CreateWrapper = OpFunction
Wrapper CreateWrapper() {
    Wrapper w;

    // Assign from final struct fields who have associated counters
// CHECK:      [[src_17:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticBundle_0
// CHECK-NEXT:                OpStore %counter_var_w_0_0_0 [[src_17]]
    w.b.b1.rw      = staticBundle.rw;
// CHECK:      [[src_18:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticBundle_1
// CHECK-NEXT:                OpStore %counter_var_w_0_0_1 [[src_18]]
    w.b.b1.append  = staticBundle.append;
// CHECK:      [[src_19:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticBundle_2
// CHECK-NEXT:                OpStore %counter_var_w_0_0_2 [[src_19]]
    w.b.b1.consume = staticBundle.consume;

    // Assign to intermediate structs whose fields have associated counters
// CHECK:      [[src_20:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticBundle_0
// CHECK-NEXT:                OpStore %counter_var_w_0_1_0 [[src_20]]
// CHECK-NEXT: [[src_21:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticBundle_1
// CHECK-NEXT:                OpStore %counter_var_w_0_1_1 [[src_21]]
// CHECK-NEXT: [[src_22:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_staticBundle_2
// CHECK-NEXT:                OpStore %counter_var_w_0_1_2 [[src_22]]
    w.b.b2         = staticBundle;

    // Assign from intermediate structs whose fields have associated counters
// CHECK:      [[src_23:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_w_0_0_0
// CHECK-NEXT:                OpStore %counter_var_staticBundle_0 [[src_23]]
// CHECK-NEXT: [[src_24:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_w_0_0_1
// CHECK-NEXT:                OpStore %counter_var_staticBundle_1 [[src_24]]
// CHECK-NEXT: [[src_25:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_w_0_0_2
// CHECK-NEXT:                OpStore %counter_var_staticBundle_2 [[src_25]]
    staticBundle   = w.b.b1;

// CHECK:      [[src_26:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_w_0_1_0
// CHECK-NEXT:                OpStore %counter_var_w_0_0_0 [[src_26]]
// CHECK-NEXT: [[src_27:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_w_0_1_1
// CHECK-NEXT:                OpStore %counter_var_w_0_0_1 [[src_27]]
// CHECK-NEXT: [[src_28:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_w_0_1_2
// CHECK-NEXT:                OpStore %counter_var_w_0_0_2 [[src_28]]
    w.b.b1         = w.b.b2;

    return w;
}

// CHECK-LABEL: %ReturnWrapper = OpFunction
Wrapper ReturnWrapper(Wrapper wrapper) {
    // Assign from parameter
// CHECK:      [[src_29:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_wrapper_0_0_0
// CHECK-NEXT:                OpStore %counter_var_ReturnWrapper_0_0_0 [[src_29]]
// CHECK-NEXT: [[src_30:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_wrapper_0_0_1
// CHECK-NEXT:                OpStore %counter_var_ReturnWrapper_0_0_1 [[src_30]]
// CHECK-NEXT: [[src_31:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_wrapper_0_0_2
// CHECK-NEXT:                OpStore %counter_var_ReturnWrapper_0_0_2 [[src_31]]
// CHECK-NEXT: [[src_32:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_wrapper_0_1_0
// CHECK-NEXT:                OpStore %counter_var_ReturnWrapper_0_1_0 [[src_32]]
// CHECK-NEXT: [[src_33:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_wrapper_0_1_1
// CHECK-NEXT:                OpStore %counter_var_ReturnWrapper_0_1_1 [[src_33]]
// CHECK-NEXT: [[src_34:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_wrapper_0_1_2
// CHECK-NEXT:                OpStore %counter_var_ReturnWrapper_0_1_2 [[src_34]]
    return wrapper;
}
