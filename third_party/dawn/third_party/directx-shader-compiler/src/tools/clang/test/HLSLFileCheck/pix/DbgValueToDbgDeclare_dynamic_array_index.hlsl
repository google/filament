// RUN: %dxc -Tcs_6_0 /Od %s | %opt -S -dxil-annotate-with-virtual-regs | %FileCheck %s

// Check that there is an alloca backing the local array
// CHECK: [[ARRAYNAME:%.*]] = alloca [4 x float]

// Grab the GEP for the above array's element that we're expecting to store to:
// CHECK: [[ARRAYELEMENTPTR:%.*]] = getelementptr inbounds [4 x float], [4 x float]* [[ARRAYNAME]]

// Check that the store to the alloca is annotated with pix-alloca-reg-read metadata 
// (meaning that the pass accurately noted that the 8.0 is stored to a dynamic array index)
// CHECK: store float 8.000000e+00, float* [[ARRAYELEMENTPTR]]
// CHECK-SAME: !pix-alloca-reg-write


RWByteAddressBuffer RawUAV: register(u1);

[numthreads(1, 1, 1)]
void main()
{
    float local_array[4];
    local_array[RawUAV.Load(0)] = 8;
    local_array[RawUAV.Load(1)] = 128;

    RawUAV.Store(64+0,local_array[0]);
    RawUAV.Store(64+4,local_array[1]);
}

