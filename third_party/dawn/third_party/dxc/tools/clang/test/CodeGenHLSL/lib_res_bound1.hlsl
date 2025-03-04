struct S
{
    float values;
};

ConstantBuffer<S> g_buf : register(b0);
RWByteAddressBuffer b : register(u0);

float Extern(uint dtid);

[shader("compute")]
[numthreads(31, 1, 1)]
void main(uint dtid : SV_DispatchThreadId)
{
    b.Store2(dtid * 4, float2(Extern(dtid), g_buf.values));
}
