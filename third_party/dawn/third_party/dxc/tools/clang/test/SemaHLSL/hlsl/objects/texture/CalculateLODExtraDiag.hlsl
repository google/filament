// RUN: %dxc -T lib_6_8 %s -verify

// Check cs/as/mesh and node.

SamplerComparisonState ss : register(s2);

RWStructuredBuffer<uint> o;
Texture1D        <float>  t1;

// expected-note@+3{{entry function defined here}}
[numthreads(3,8,1)]
[shader("compute")]
void foo(uint3 id : SV_GroupThreadID)
{
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'foo' requires derivatives - when used in compute/amplification/mesh shaders or broadcast nodes, numthreads must be either 1D with X as a multiple of 4 or both X and Y must be multiples of 2}}
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
}

// Make sure 2d mode ok with z != 1.
[numthreads(4,2,3)]
[shader("compute")]
void foo2(uint3 id : SV_GroupThreadID)
{
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
}

// expected-note@+3{{entry function defined here}}
[numthreads(3,1,1)]
[shader("compute")]
void bar(uint3 id : SV_GroupThreadID)
{
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'bar' requires derivatives - when used in compute/amplification/mesh shaders or broadcast nodes, numthreads must be either 1D with X as a multiple of 4 or both X and Y must be multiples of 2}}
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
}

// expected-note@+4{{entry function defined here}}
[shader("mesh")]
[numthreads(3,1,1)]
[outputtopology("triangle")]
void mesh(uint ix : SV_GroupIndex, uint3 id : SV_GroupThreadID) {
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'mesh' requires derivatives - when used in compute/amplification/mesh shaders or broadcast nodes, numthreads must be either 1D with X as a multiple of 4 or both X and Y must be multiples of 2}}
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
}


struct Payload {
    float2 dummy;
};

// expected-note@+3{{entry function defined here}}
[numthreads(3, 2, 1)]
[shader("amplification")]
void ASmain()
{
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'ASmain' requires derivatives - when used in compute/amplification/mesh shaders or broadcast nodes, numthreads must be either 1D with X as a multiple of 4 or both X and Y must be multiples of 2}}
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
    Payload pld;
    pld.dummy = float2(1.0,2.0);
    DispatchMesh(8, 1, 1, pld);
}

struct RECORD {
  uint a;
};

// expected-note@+5{{entry function defined here}}
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input) {
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'node01' requires derivatives - when used in compute/amplification/mesh shaders or broadcast nodes, numthreads must be either 1D with X as a multiple of 4 or both X and Y must be multiples of 2}}
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
 }

// expected-note@+5{{entry function defined here}}
[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node02()
{
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'node02' requires derivatives - only available in pixel, compute, amplification, mesh, or broadcast node shaders}}
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
}

// expected-note@+2 {{entry function defined here}}
[Shader("vertex")]
float4 vs(float2 a :A) :SV_Position {
  float r = 0;
  if (1>3)
    // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'vs' requires derivatives - only available in pixel, compute, amplification, mesh, or broadcast node shaders}}
    r = t1.CalculateLevelOfDetail(ss, 0.5).x;
  return r;
}

SamplerComparisonState s;
Texture1D t;
// expected-note@+3{{entry function defined here}}
// expected-note@+2{{entry function defined here}}
[shader("vertex")]
float4 vs2(float a:A) : SV_Position {
  // expected-error@+1 {{Intrinsic CalculateLevelOfDetail potentially used by 'vs2' requires derivatives - only available in pixel, compute, amplification, mesh, or broadcast node shaders}}
  return t.CalculateLevelOfDetail(s, a) +
  // expected-error@+1 {{Intrinsic CalculateLevelOfDetailUnclamped potentially used by 'vs2' requires derivatives - only available in pixel, compute, amplification, mesh, or broadcast node shaders}}
    t.CalculateLevelOfDetailUnclamped(ss, a);
}
