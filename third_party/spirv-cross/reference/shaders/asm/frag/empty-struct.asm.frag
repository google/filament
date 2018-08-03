#version 450

struct EmptyStructTest
{
    int empty_struct_member;
};

float GetValue(EmptyStructTest self)
{
    return 0.0;
}

float GetValue_1(EmptyStructTest self)
{
    return 0.0;
}

void main()
{
    EmptyStructTest _23 = EmptyStructTest(0);
    EmptyStructTest emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(_23);
}

