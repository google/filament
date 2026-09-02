// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Bundle {
      RWStructuredBuffer<float> rw;
  AppendStructuredBuffer<float> append;
 ConsumeStructuredBuffer<float> consume;
};

// Counter variables for the this object of getCSBuffer()
// CHECK: %counter_var_getCSBuffer_this_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getCSBuffer_this_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getCSBuffer_this_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getCSBuffer_this_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getCSBuffer_this_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getCSBuffer_this_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// Counter variables for the this object of getASBuffer()
// CHECK: %counter_var_getASBuffer_this_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getASBuffer_this_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getASBuffer_this_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getASBuffer_this_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getASBuffer_this_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getASBuffer_this_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

// Counter variables for the this object of getRWSBuffer()
// CHECK: %counter_var_getRWSBuffer_this_0_0_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getRWSBuffer_this_0_0_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getRWSBuffer_this_0_0_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getRWSBuffer_this_0_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getRWSBuffer_this_0_1_1 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_getRWSBuffer_this_0_1_2 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private

struct TwoBundle {
    Bundle b1;
    Bundle b2;

    // Checks at the end of the file
    ConsumeStructuredBuffer<float> getCSBuffer() { return b1.consume; }
};

struct Wrapper {
    TwoBundle b;

    // Checks at the end of the file
    AppendStructuredBuffer<float> getASBuffer() { return b.b1.append; }

    // Checks at the end of the file
    RWStructuredBuffer<float> getRWSBuffer() { return b.b2.rw; }
};

ConsumeStructuredBuffer<float> globalCSBuffer;
AppendStructuredBuffer<float> globalASBuffer;
RWStructuredBuffer<float> globalRWSBuffer;

// CHECK-LABEL: %src_main = OpFunction
float main() : VVV {
    TwoBundle localBundle;
    Wrapper   localWrapper;

    localBundle.b1.consume = globalCSBuffer;
    localWrapper.b.b1.append = globalASBuffer;
    localWrapper.b.b2.rw = globalRWSBuffer;

// CHECK:      [[counter:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localBundle_0_0
// CHECK-NEXT:                    OpStore %counter_var_getCSBuffer_this_0_0 [[counter]]
// CHECK-NEXT: [[counter_0:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localBundle_0_1
// CHECK-NEXT:                    OpStore %counter_var_getCSBuffer_this_0_1 [[counter_0]]
// CHECK-NEXT: [[counter_1:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localBundle_0_2
// CHECK-NEXT:                    OpStore %counter_var_getCSBuffer_this_0_2 [[counter_1]]
// CHECK-NEXT: [[counter_2:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localBundle_1_0
// CHECK-NEXT:                    OpStore %counter_var_getCSBuffer_this_1_0 [[counter_2]]
// CHECK-NEXT: [[counter_3:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localBundle_1_1
// CHECK-NEXT:                    OpStore %counter_var_getCSBuffer_this_1_1 [[counter_3]]
// CHECK-NEXT: [[counter_4:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localBundle_1_2
// CHECK-NEXT:                    OpStore %counter_var_getCSBuffer_this_1_2 [[counter_4]]
// CHECK-NEXT:                    OpFunctionCall %_ptr_Uniform_type_ConsumeStructuredBuffer_float %TwoBundle_getCSBuffer %localBundle
    float value = localBundle.getCSBuffer().Consume();

// CHECK:      [[counter_5:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_0
// CHECK-NEXT:                    OpStore %counter_var_getASBuffer_this_0_0_0 [[counter_5]]
// CHECK-NEXT: [[counter_6:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_1
// CHECK-NEXT:                    OpStore %counter_var_getASBuffer_this_0_0_1 [[counter_6]]
// CHECK-NEXT: [[counter_7:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_2
// CHECK-NEXT:                    OpStore %counter_var_getASBuffer_this_0_0_2 [[counter_7]]
// CHECK-NEXT: [[counter_8:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_0
// CHECK-NEXT:                    OpStore %counter_var_getASBuffer_this_0_1_0 [[counter_8]]
// CHECK-NEXT: [[counter_9:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_1
// CHECK-NEXT:                    OpStore %counter_var_getASBuffer_this_0_1_1 [[counter_9]]
// CHECK-NEXT: [[counter_10:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_2
// CHECK-NEXT:                    OpStore %counter_var_getASBuffer_this_0_1_2 [[counter_10]]
// CHECK-NEXT:                    OpFunctionCall %_ptr_Uniform_type_AppendStructuredBuffer_float %Wrapper_getASBuffer %localWrapper
    localWrapper.getASBuffer().Append(4.2);

// CHECK:      [[counter_11:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_0
// CHECK-NEXT:                    OpStore %counter_var_getRWSBuffer_this_0_0_0 [[counter_11]]
// CHECK-NEXT: [[counter_12:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_1
// CHECK-NEXT:                    OpStore %counter_var_getRWSBuffer_this_0_0_1 [[counter_12]]
// CHECK-NEXT: [[counter_13:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_0_2
// CHECK-NEXT:                    OpStore %counter_var_getRWSBuffer_this_0_0_2 [[counter_13]]
// CHECK-NEXT: [[counter_14:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_0
// CHECK-NEXT:                    OpStore %counter_var_getRWSBuffer_this_0_1_0 [[counter_14]]
// CHECK-NEXT: [[counter_15:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_1
// CHECK-NEXT:                    OpStore %counter_var_getRWSBuffer_this_0_1_1 [[counter_15]]
// CHECK-NEXT: [[counter_16:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_localWrapper_0_1_2
// CHECK-NEXT:                    OpStore %counter_var_getRWSBuffer_this_0_1_2 [[counter_16]]
// CHECK-NEXT:                    OpFunctionCall %_ptr_Uniform_type_RWStructuredBuffer_float %Wrapper_getRWSBuffer %localWrapper
    RWStructuredBuffer<float> localRWSBuffer = localWrapper.getRWSBuffer();

    return localRWSBuffer[5];
}

// CHECK-LABEL: %TwoBundle_getCSBuffer = OpFunction
// CHECK:             [[counter_17:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_getCSBuffer_this_0_2
// CHECK-NEXT:                           OpStore %counter_var_getCSBuffer [[counter_17]]

// CHECK-LABEL: %Wrapper_getASBuffer = OpFunction
// CHECK:           [[counter_18:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_getASBuffer_this_0_0_1
// CHECK-NEXT:                         OpStore %counter_var_getASBuffer [[counter_18]]

// CHECK-LABEL: %Wrapper_getRWSBuffer = OpFunction
// CHECK:            [[counter_19:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_getRWSBuffer_this_0_1_0
// CHECK-NEXT:                          OpStore %counter_var_getRWSBuffer [[counter_19]]
