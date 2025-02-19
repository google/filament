struct EmptyStructTest
{
    int empty_struct_member;
};

struct EmptyStruct2Test
{
    EmptyStructTest _m0;
};

static const EmptyStructTest _38 = { 0 };
static const EmptyStruct2Test _26 = { { 0 } };

float GetValue(EmptyStruct2Test self)
{
    return 0.0f;
}

float GetValue_1(EmptyStruct2Test self)
{
    return 0.0f;
}

void frag_main()
{
    EmptyStructTest _27 = { 0 };
    EmptyStruct2Test _28 = { _27 };
    EmptyStruct2Test emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(_28);
    value = GetValue_1(_26);
}

void main()
{
    frag_main();
}
