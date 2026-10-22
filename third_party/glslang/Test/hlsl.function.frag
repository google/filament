float4 fun0()
{
    return 1.0f;
}

float4 fun2(float4 const col)
{
    return (1.0f, 2.0f, 3.0f, 4.0f);
}

uint fun3(const float4 col)
{
	return 7;   
}

float4 fun4(uint id1, uniform uint id2)
{
    return id1 * id2;
}

float4 fun1(int index)
{
    uint entityId = fun3(fun2(fun0()));
    return fun4(entityId, entityId);
}

int main() : SV_TARGET
{
    return fun1;
}