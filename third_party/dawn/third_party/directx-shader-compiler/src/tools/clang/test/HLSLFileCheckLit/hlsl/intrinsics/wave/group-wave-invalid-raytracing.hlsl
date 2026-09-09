// REQUIRES: dxil-1-10

// RUN: not %dxc -T lib_6_10 %s 2>&1 | FileCheck %s

// CHECK-DAG: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(callable).
// CHECK-DAG: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(intersection).
// CHECK-DAG: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(anyhit).
// CHECK-DAG: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(miss).
// CHECK-DAG: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(closesthit).
// CHECK-DAG: error: Opcode GetGroupWaveCount not valid in shader model lib_6_10(raygeneration).
// CHECK-DAG: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(callable).
// CHECK-DAG: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(intersection).
// CHECK-DAG: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(anyhit).
// CHECK-DAG: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(miss).
// CHECK-DAG: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(closesthit).
// CHECK-DAG: error: Opcode GetGroupWaveIndex not valid in shader model lib_6_10(raygeneration).

struct [raypayload] Payload {
    float value : write(closesthit, miss, anyhit, caller) : read(caller);
};
struct Attributes {
    float2 barycentrics;
};

RWStructuredBuffer<uint> output : register(u0);

[shader("raygeneration")]
void RayGenMain() {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    output[0] = waveIdx + waveCount;
}

[shader("closesthit")]
void ClosestHitMain(inout Payload payload, in Attributes attribs) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    payload.value = waveIdx + waveCount;
}

[shader("miss")]
void MissMain(inout Payload payload) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    payload.value = waveIdx + waveCount;
}

[shader("anyhit")]
void AnyHitMain(inout Payload payload, in Attributes attribs) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    payload.value = waveIdx + waveCount;
}

[shader("intersection")]
void IntersectionMain() {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    output[0] = waveIdx + waveCount;
}

[shader("callable")]
void CallableMain(inout Payload payload) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    payload.value = waveIdx + waveCount;
}
