// RUN: %dxilver 1.8 | %dxc -T lib_6_8 -Vd %s | %D3DReflect %s | %FileCheck %s -check-prefixes=RDAT

// Check that ShaderCompatInfo in RDAT has expected flags for scenario with
// thread launch node calling a function using derivatives (through Sample()).
// Validation is disabled to allow this to produce the RDAT blob to check.
// Used to generate deriv-in-nested-fn-node-lib68-launch.ll

// RDAT: FunctionTable[{{.*}}] = {

Texture2D<float4> T : register(t0);
SamplerState S : register(s0);
RWBuffer<float4> Buf : register(u0);

// RDAT-LABEL:   UnmangledName: "write_value"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: 0
// RDAT:   ShaderStageFlag: (Pixel | Vertex | Geometry | Hull | Domain | Compute | Library | RayGeneration | Intersection | AnyHit | ClosestHit | Miss | Callable | Mesh | Amplification | Node)
// RDAT:   MinShaderTarget: 0x60060

[noinline] export
void write_value(float value) {
    Buf[uint(value)] = value;
}

// RDAT-LABEL:   UnmangledName: "fn_sample"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: (Opt_UsesDerivatives)
// RDAT:   ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT:   MinShaderTarget: 0x60060

[noinline] export
void fn_sample() {
    write_value(T.Sample(S, float2(0, 0)).x);
}

// RDAT-LABEL:   UnmangledName: "intermediate"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: (Opt_UsesDerivatives)
// RDAT:   ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
// RDAT:   MinShaderTarget: 0x60060

[noinline] export
void intermediate() {
    write_value(1);
    fn_sample();
    write_value(2);
}

// RDAT-LABEL:   UnmangledName: "main"
// RDAT:   FeatureInfo1: 0
// RDAT:   FeatureInfo2: (Opt_UsesDerivatives)
// RDAT:   ShaderStageFlag: (Node)
// RDAT:   MinShaderTarget: 0xf0068
// RDAT:   LaunchType: Thread

[shader("node")]
[NodeLaunch("thread")]
void main() {
    intermediate();
}
