// RUN: %dxc -T vs_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

//CHECK: [[name:%[0-9]+]] = OpString "@type.sampler"
//CHECK: [[info_none:%[0-9]+]] = OpExtInst %void [[ext:%[0-9]+]] DebugInfoNone
//CHECK: [[sampler:%[0-9]+]] = OpExtInst %void [[ext]] DebugTypeComposite [[name]] Structure [[src:%[0-9]+]] 0 0 [[cu:%[0-9]+]] {{%[0-9]+}} [[info_none]]

//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[sampler]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %s3
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[sampler]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %s2
//CHECK: OpExtInst %void [[ext]] DebugGlobalVariable {{%[0-9]+}} [[sampler]] [[src]] {{[0-9]+}} {{[0-9]+}} [[cu]] {{%[0-9]+}} %s1

SamplerState           s1 : register(s1);
SamplerComparisonState s2 : register(s2);
sampler                s3 : register(s3);

void main() {
}
