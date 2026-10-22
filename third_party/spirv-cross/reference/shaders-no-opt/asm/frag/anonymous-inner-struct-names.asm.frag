#version 450

struct anon_aa
{
    int foo;
};

struct anon_ab
{
    int foo;
};

struct anon_a
{
    anon_aa _aa;
    anon_ab ab;
};

struct anon_ba
{
    int foo;
};

struct anon_bb
{
    int foo;
};

struct anon_b
{
    anon_ba _ba;
    anon_bb bb;
};

struct anon_ca
{
    int foo;
};

struct anon_c
{
    anon_ca _ca;
};

struct anon_da
{
    int foo;
};

struct anon_d
{
    anon_da da;
};

struct anon_e
{
    int a;
};

void main()
{
}

