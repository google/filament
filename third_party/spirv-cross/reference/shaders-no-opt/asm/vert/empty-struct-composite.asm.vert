#version 450

struct Test
{
    int empty_struct_member;
};

void main()
{
    Test _14 = Test(0);
    Test t = _14;
}

