struct Test
{
    int empty_struct_member;
};

void vert_main()
{
    Test _13 = { 0 };
    Test t = _13;
}

void main()
{
    vert_main();
}
