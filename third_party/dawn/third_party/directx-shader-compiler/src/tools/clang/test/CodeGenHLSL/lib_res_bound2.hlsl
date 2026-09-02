struct S1
{
    float values;
};

ConstantBuffer<S1> g_buf : register(b1);

export
float Extern(uint dtid)
{
     return g_buf.values + dtid;
}
