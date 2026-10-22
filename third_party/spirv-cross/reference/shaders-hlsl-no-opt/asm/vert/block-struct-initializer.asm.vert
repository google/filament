struct Vert
{
    float a;
    float b;
};

struct Foo
{
    float c;
    float d;
};

static const Vert _12 = { 0.0f, 0.0f };
static const Foo _14 = { 0.0f, 0.0f };

static Vert _4 = { 0.0f, 0.0f };
static Foo foo = _14;

struct SPIRV_Cross_Output
{
    float Vert_a : TEXCOORD0;
    float Vert_b : TEXCOORD1;
    Foo foo : TEXCOORD2;
};

void vert_main()
{
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.Vert_a = _4.a;
    stage_output.Vert_b = _4.b;
    stage_output.foo = foo;
    return stage_output;
}
