// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tcs_6_0 -verify %s

struct MyData {
   float3 x[4];  
};

StructuredBuffer<MyData> DataIn;
RWStructuredBuffer<MyData> DataOut;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 dispatchid : SV_DispatchThreadID)
{
    const MyData data = DataIn[dispatchid.x];
    data.x[0] = 1.0f; // expected-error {{read-only variable is not assignable}} fxc-error {{X3025: l-value specifies const object}}
    DataOut[dispatchid.x] = data;
}
