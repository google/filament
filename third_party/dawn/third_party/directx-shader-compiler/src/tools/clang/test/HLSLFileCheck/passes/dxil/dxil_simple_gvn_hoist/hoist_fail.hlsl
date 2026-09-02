// RUN: %dxc -Emain -T cs_6_0 %s | FileCheck %s

// Make sure not crash.
// CHECK:main

cbuffer cb
{
    float4 p;
    float a;
    float b;
};

RWStructuredBuffer<uint> U0;

struct Data
{
    float3 f;
    uint i;
};

Data foo()
{
    Data d;
    InterlockedAdd(U0[0], (uint)1);

    d.f = p.z;

    d.i =  p.x;
    return d;
}

void bar()
{
    float c = (p.x - a) + b;

    if(c > 0)
        return;

    InterlockedAdd(U0[1], (uint)1);
}

Data Update(Data d)
{
    if (p.z > 0)
      d = foo();
    else
      bar();
	return d;
}

RWStructuredBuffer<Data> U1;

[numthreads(64, 1, 1)]
void main(uint ID : SV_DispatchThreadID)
{
    Data d = U1[ID];
    d = Update(d);
    U1[ID] = d;
}