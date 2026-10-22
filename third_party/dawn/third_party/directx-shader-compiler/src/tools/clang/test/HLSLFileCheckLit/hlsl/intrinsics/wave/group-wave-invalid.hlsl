// REQUIRES: dxil-1-10

// RUN: not %dxc -T vs_6_10 -E mainVS %s 2>&1 | FileCheck %s -check-prefix=VS
// RUN: not %dxc -T ps_6_10 -E mainPS %s 2>&1 | FileCheck %s -check-prefix=PS
// RUN: not %dxc -T gs_6_10 -E mainGS %s 2>&1 | FileCheck %s -check-prefix=GS
// RUN: not %dxc -T hs_6_10 -E mainHS %s 2>&1 | FileCheck %s -check-prefix=HS
// RUN: not %dxc -T ds_6_10 -E mainDS %s 2>&1 | FileCheck %s -check-prefix=DS

// VS-DAG: error: Opcode GetGroupWaveCount not valid in shader model vs_6_10.
// VS-DAG: error: Opcode GetGroupWaveIndex not valid in shader model vs_6_10.

float4 mainVS() : SV_Position {
    float4 pos;
    
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();

    pos.x = waveIdx + waveCount;
    pos.y = 0.0;
    pos.z = 0.0;
    pos.w = 1.0;

    return pos;
}

// PS-DAG: error: Opcode GetGroupWaveCount not valid in shader model ps_6_10.
// PS-DAG: error: Opcode GetGroupWaveIndex not valid in shader model ps_6_10.

float4 mainPS(float4 input : SV_Position) : SV_Target {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    return input + waveIdx + waveCount;
}

// GS-DAG: error: Opcode GetGroupWaveCount not valid in shader model gs_6_10.
// GS-DAG: error: Opcode GetGroupWaveIndex not valid in shader model gs_6_10.

struct GSInput {
    float4 pos : SV_Position;
};

struct GSOutput {
    float4 pos : SV_Position;
};

[maxvertexcount(3)]
void mainGS(triangle GSInput input[3], inout TriangleStream<GSOutput> outStream) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    GSOutput output;
    for (int i = 0; i < 3; i++) {
        output.pos = input[i].pos + waveIdx + waveCount;
        outStream.Append(output);
    }
    outStream.RestartStrip();
}

// HS-DAG: error: Opcode GetGroupWaveIndex not valid in shader model hs_6_10.
// HS-DAG: error: Opcode GetGroupWaveCount not valid in shader model hs_6_10.

struct HSInput {
    float4 pos : SV_Position;
};

struct HSOutput {
    float4 pos : SV_Position;
};

struct HSConstantOutput {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSConstantOutput PatchConstantFunc(InputPatch<HSInput, 3> ip, uint patchID : SV_PrimitiveID) {
    HSConstantOutput output;
    output.edges[0] = 1.0;
    output.edges[1] = 1.0;
    output.edges[2] = 1.0;
    output.inside = 1.0;
    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunc")]
HSOutput mainHS(InputPatch<HSInput, 3> ip, uint cpID : SV_OutputControlPointID) {
    HSOutput output;
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    output.pos = ip[cpID].pos + waveIdx + waveCount;
    return output;
}

// DS-DAG: error: Opcode GetGroupWaveCount not valid in shader model ds_6_10.
// DS-DAG: error: Opcode GetGroupWaveIndex not valid in shader model ds_6_10.

struct DSInput {
    float4 pos : SV_Position;
};

struct DSOutput {
    float4 pos : SV_Position;
};

[domain("tri")]
DSOutput mainDS(HSConstantOutput constants, float3 uvw : SV_DomainLocation,
              const OutputPatch<DSInput, 3> patch) {
    DSOutput output;
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    output.pos = patch[0].pos * uvw.x + patch[1].pos * uvw.y + patch[2].pos * uvw.z + waveIdx + waveCount;
    return output;
}

