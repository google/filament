// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Regression test for github issue #1724, which was crashing
// in SROA_Parameter_HLSL due to the memcpy instruction between
// the VsInput structs and its source bitcasts getting erased
// while an IRBuilder was pointing to one of these instructions
// as an insertion point.

// CHECK: ret void

struct VsInput
{
	uint instanceId : SV_InstanceID;
	uint vertexId : SV_VertexID;
};

static VsInput __vsInput;

void main(VsInput inputs)
{
	__vsInput = inputs; // This line triggers compiler crash
}