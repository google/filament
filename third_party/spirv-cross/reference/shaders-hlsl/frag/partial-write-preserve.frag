struct B
{
    float a;
    float b;
};

static const B _80 = { 10.0f, 20.0f };

cbuffer UBO : register(b0)
{
    int _42_some_value : packoffset(c0);
};


void partial_inout(inout float4 x)
{
    x.x = 10.0f;
}

void complete_inout(out float4 x)
{
    x = 50.0f.xxxx;
}

void branchy_inout(inout float4 v)
{
    v.y = 20.0f;
    if (_42_some_value == 20)
    {
        v = 50.0f.xxxx;
    }
}

void branchy_inout_2(out float4 v)
{
    if (_42_some_value == 20)
    {
        v = 50.0f.xxxx;
    }
    else
    {
        v = 70.0f.xxxx;
    }
    v.y = 20.0f;
}

void partial_inout(inout B b)
{
    b.b = 40.0f;
}

void frag_main()
{
    float4 a = 10.0f.xxxx;
    float4 param = a;
    partial_inout(param);
    a = param;
    float4 param_1;
    complete_inout(param_1);
    a = param_1;
    float4 param_2 = a;
    branchy_inout(param_2);
    a = param_2;
    float4 param_3;
    branchy_inout_2(param_3);
    a = param_3;
    B b = _80;
    B param_4 = b;
    partial_inout(param_4);
    b = param_4;
}

void main()
{
    frag_main();
}
