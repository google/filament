// RUN: %dxc -T lib_6_3 %s -verify

struct Payload {
    float2 dummy;
};

//[numthreads(8, 1, 1)]
[shader("amplification")]
/* expected-error@+1{{amplification entry point must have a valid numthreads attribute}} */
void ASmain()
{
    Payload pld;
    pld.dummy = float2(1.0,2.0);
    DispatchMesh(8, 1, 1, pld);
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
//[patchconstantfunc("HSPatch")]
/* expected-error@+1{{hull entry point must have a valid patchconstantfunc attribute}} */
float4 HSmain(uint ix : SV_OutputControlPointID)
{
  return 0;
}

struct VSOut {
  float4 pos : SV_Position;
};

//[maxvertexcount(3)]
[shader("geometry")]
/* expected-error@+1{{geometry entry point must have a valid maxvertexcount attribute}} */
void GSmain(inout TriangleStream<VSOut> stream) {
  VSOut v = {0.0, 0.0, 0.0, 0.0};
  stream.Append(v);
  stream.RestartStrip();
}

struct myvert {
    float4 position : SV_Position;
};


[shader("mesh")]
//[NumThreads(8, 8, 2)]
//[OutputTopology("triangle")]
// expected-error@+2{{mesh entry point must have a valid numthreads attribute}}
// expected-error@+1{{mesh entry point must have a valid outputtopology attribute}}
void MSmain(out vertices myvert verts[32],
          uint ix : SV_GroupIndex) {
  SetMeshOutputCounts(32, 16);
  myvert v = {0.0, 0.0, 0.0, 0.0};
  verts[ix] = v;
}

// expected-error@+2{{compute entry point must have a valid numthreads attribute}}
[shader("compute")]
void CSmain() {}
