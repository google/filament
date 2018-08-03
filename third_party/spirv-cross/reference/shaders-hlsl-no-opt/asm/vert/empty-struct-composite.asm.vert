struct Test
{
    int empty_struct_member;
};

void vert_main()
{
    Test _14 = { 0 };
    Test t = _14;
}

void main()
{
    vert_main();
}
