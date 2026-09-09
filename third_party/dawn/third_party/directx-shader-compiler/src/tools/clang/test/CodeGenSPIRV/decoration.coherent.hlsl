// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s -check-prefix=GLSL450
// RUN: %dxc -T ps_6_0 -E main -fcgl -fspv-use-vulkan-memory-model -fspv-target-env=vulkan1.1 %s -spirv | FileCheck %s -check-prefix=VULKAN

// When the GLSL450 memory model is used, there should be no memory operands on the loads and stores.
// When the Vulkan memory model is used, there should be no decorations. There should be memory operands on the loads and stores instead.

struct StructA
{
    uint one;
};

// GLSL450: OpMemoryModel Logical GLSL450
// VULKAN: OpMemoryModel Logical Vulkan

// GLSL450: OpDecorate %buffer1 Coherent
// VULKAN-NOT: OpDecorate %buffer1 Coherent
globallycoherent RWByteAddressBuffer buffer1;

// GLSL450: OpDecorate %buffer2 Coherent
// VULKAN-NOT: OpDecorate %buffer2 Coherent
globallycoherent RWStructuredBuffer<StructA> buffer2;

// GLSL450-NOT: OpDecorate %buffer3 Coherent
// VULKAN-NOT: OpDecorate %buffer3 Coherent
RWByteAddressBuffer buffer3;

// GLSL450-NOT: OpDecorate %buffer4 Coherent
// VULKAN-NOT: OpDecorate %buffer3 Coherent
RWStructuredBuffer<StructA> buffer4;

void main()
{

// GLSL450: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer1
// GLSL450: OpLoad %uint [[ac]]{{$}}
// GLSL450: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer3
// GLSL450: OpLoad %uint [[ac]]{{$}}
// GLSL450: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer4
// GLSL450: OpLoad %uint [[ac]]{{$}}
// GLSL450: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer2
// GLSL450: OpStore [[ac]] {{%[0-9]+$}}

// VULKAN: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer1
// VULKAN: OpLoad %uint [[ac]] MakePointerVisible|NonPrivatePointer %uint_5
// VULKAN: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer3
// VULKAN: OpLoad %uint [[ac]]{{$}}
// VULKAN: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer4
// VULKAN: OpLoad %uint [[ac]]{{$}}
// VULKAN: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer2
// VULKAN: OpStore [[ac]] {{%[0-9]+}} MakePointerAvailable|NonPrivatePointer %uint_5

  buffer2[0].one = buffer1.Load(0) + buffer3.Load(0) + buffer4[0].one;

// GLSL450: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer2
// GLSL450: OpLoad %uint [[ac]]{{$}}
// GLSL450: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer4
// GLSL450: OpStore [[ac]] {{%[0-9]+$}}

// VULKAN: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer2
// VULKAN: OpLoad %uint [[ac]] MakePointerVisible|NonPrivatePointer %uint_5
// VULKAN: [[ac:%[0-9]+]] = OpAccessChain {{.*}} %buffer4
// VULKAN: OpStore [[ac]] {{%[0-9]+$}}
  buffer4[0].one = buffer2[0].one;
  
}
