struct Foo
{
    float c;
    float d;
};

static const Foo _13 = { 0.0f, 0.0f };

static Foo foo = _13;

struct Vert
{
    float a : TEXCOORD0;
    float b : TEXCOORD1;
};

static Vert _3 = { 0.0f, 0.0f };

struct SPIRV_Cross_Output
{
    Foo foo : TEXCOORD2;
};

void vert_main()
{
}

SPIRV_Cross_Output main(out Vert stage_output_3)
{
    vert_main();
    stage_output_3 = _3;
    SPIRV_Cross_Output stage_output;
    stage_output.foo = foo;
    return stage_output;
}
