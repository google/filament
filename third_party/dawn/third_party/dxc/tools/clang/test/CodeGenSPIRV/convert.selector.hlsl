// Convert to half
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=half -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FCONVERT

// Convert to float
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=float -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FCONVERT

// Convert to double
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UTOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=STOF
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=double -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FCONVERT

// int type to int16_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=BITCAST
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT

// float type to int16_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=int16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS

// int type to int32_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=BITCAST
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT

// float type to int32_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=int32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS

// int type to int64_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=SCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=BITCAST

// float type to int64_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=int64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOS

// int type to uint16_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=BITCAST
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT

// float type to uint16_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=uint16_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU

// int type to uint32_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=BITCAST
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint64_t -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT

// float type to uint32_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=uint32_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU

// int type to uint64_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=int16_t -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int32_t -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint16_t -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=uint32_t -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=UCONVERT
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=int64_t -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=BITCAST

// float type to uint64_t
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=half -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU
// RUN: %dxc -fspv-target-env=vulkan1.3 -enable-16bit-types -T cs_6_2 -E main -spirv -HV 2021 -DSOURCE_TYPE=float -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU
// RUN: %dxc -fspv-target-env=vulkan1.3 -T cs_6_0 -E main -spirv -HV 2021 -DSOURCE_TYPE=double -DTARGET_TYPE=uint64_t -I %hlsl_headers %s | FileCheck %s --check-prefix=CHECK --check-prefix=FTOU

#include "vk/opcode_selector.h"

#define VEC_TYPE_INT(TYPE) TYPE##4
#define VEC_TYPE(t) VEC_TYPE_INT(t)

RWStructuredBuffer<VEC_TYPE(SOURCE_TYPE)> source;
RWStructuredBuffer<VEC_TYPE(TARGET_TYPE)> target;

[numthreads(64, 1, 1)] void main() {
// CHECK: [[ac:%[0-9]+]] = OpAccessChain {{%_ptr_StorageBuffer_.*}} %source %int_0 %uint_0
// CHECK: [[ld:%[0-9]+]] = OpLoad {{%.*}} [[ac]]
// STOF: [[result:%[0-9]+]] = OpConvertSToF {{%.*}} [[ld]]
// FTOS: [[result:%[0-9]+]] = OpConvertFToS {{%.*}} [[ld]]
// UTOF: [[result:%[0-9]+]] = OpConvertUToF {{%.*}} [[ld]]
// FTOU: [[result:%[0-9]+]] = OpConvertFToU {{%.*}} [[ld]]
// FCONVERT: [[result:%[0-9]+]] = OpFConvert {{%.*}} [[ld]]
// UCONVERT: [[result:%[0-9]+]] = OpUConvert {{%.*}} [[ld]]
// SCONVERT: [[result:%[0-9]+]] = OpSConvert {{%.*}} [[ld]]
// BITCAST: [[result:%[0-9]+]] = OpBitcast {{%.*}} [[ld]]
// CHECK: [[ac:%[0-9]+]] = OpAccessChain {{%_ptr_StorageBuffer_.*}} %target %int_0 %uint_0
// CHECK: OpStore [[ac]] [[result]]
  target[0] = vk::util::ConversionSelector<SOURCE_TYPE, TARGET_TYPE>::Convert<VEC_TYPE(TARGET_TYPE)>(source[0]);

// CHECK: [[ac:%[0-9]+]] = OpAccessChain {{%_ptr_StorageBuffer_.*}} %source %int_0 %uint_0 %int_0
// CHECK: [[ld:%[0-9]+]] = OpLoad {{%.*}} [[ac]]
// STOF: [[result:%[0-9]+]] = OpConvertSToF {{%.*}} [[ld]]
// FTOS: [[result:%[0-9]+]] = OpConvertFToS {{%.*}} [[ld]]
// UTOF: [[result:%[0-9]+]] = OpConvertUToF {{%.*}} [[ld]]
// FTOU: [[result:%[0-9]+]] = OpConvertFToU {{%.*}} [[ld]]
// FCONVERT: [[result:%[0-9]+]] = OpFConvert {{%.*}} [[ld]]
// UCONVERT: [[result:%[0-9]+]] = OpUConvert {{%.*}} [[ld]]
// SCONVERT: [[result:%[0-9]+]] = OpSConvert {{%.*}} [[ld]]
// BITCAST: [[result:%[0-9]+]] = OpBitcast {{%.*}} [[ld]]
// CHECK: [[ac:%[0-9]+]] = OpAccessChain {{%_ptr_StorageBuffer_.*}} %target %int_0 %uint_0 %int_0
// CHECK: OpStore [[ac]] [[result]]
  target[0].x = vk::util::ConversionSelector<SOURCE_TYPE, TARGET_TYPE>::Convert<TARGET_TYPE>(source[0].x);
}
