#version 450

struct EmptyStructTest
{
    int empty_struct_member;
};

struct EmptyStruct2Test
{
    EmptyStructTest _m0;
};

float GetValue(EmptyStruct2Test self)
{
    return 0.0;
}

float GetValue_1(EmptyStruct2Test self)
{
    return 0.0;
}

void main()
{
    EmptyStructTest _27 = EmptyStructTest(0);
    EmptyStruct2Test emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(EmptyStruct2Test(_27));
    value = GetValue_1(EmptyStruct2Test(EmptyStructTest(0)));
}

