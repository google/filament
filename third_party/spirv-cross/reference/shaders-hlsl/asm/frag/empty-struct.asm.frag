struct EmptyStructTest
{
    int empty_struct_member;
};

float GetValue(EmptyStructTest self)
{
    return 0.0f;
}

float GetValue_1(EmptyStructTest self)
{
    return 0.0f;
}

void frag_main()
{
    EmptyStructTest _24 = { 0 };
    EmptyStructTest emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(_24);
}

void main()
{
    frag_main();
}
