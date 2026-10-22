// RUN: %dxc /T vs_6_0 /Zi /O1 %s | FileCheck %s
// RUN: %dxc /T vs_6_0 /Zi /O2 %s | FileCheck %s
// RUN: %dxc /T vs_6_0 /Zi /O3 %s | FileCheck %s

// CHECK: @main
// CHECK: @call @llvm.dbg.declare

// Regression test for #4536
// HL Matrix lowering recreated dbg.declare calls but didn't copy over the
// debug location, which led to assert in code that expected all dbg insts
// to have debug location.

static const float StaticConst1 = 0;
static const float StaticConst2 = 0;
static const float StaticConst3 = 0;
static const float StaticConst4 = 0;
static const float StaticConst5 = 0;
static const float StaticConst6 = 0;
static const float StaticConst7 = 0;
static const float StaticConst8 = 0;
static const float StaticConst9 = 0;
static const float StaticConst10 = 0;
static const float StaticConst11 = 0;
static const float StaticConst12 = 0;
static const float StaticConst13 = 0;
static const float StaticConst14 = 0;
static const float StaticConst15 = 0;
static const float StaticConst16 = 0;
static const float StaticConst17 = 0;
static const float StaticConst18 = 0;
static const float StaticConst19 = 0;
static const float StaticConst20 = 0;
static const float StaticConst21 = 0;
static const float StaticConst22 = 0;
static const float StaticConst23 = 0;
static const float StaticConst24 = 0;
static const float StaticConst25 = 0;
static const float StaticConst26 = 0;
static const float StaticConst27 = 0;
static const float StaticConst28 = 0;
static const float StaticConst29 = 0;
static const float StaticConst30 = 0;
static const float StaticConst31 = 0;
static const float StaticConst32 = 0;
static const float StaticConst33 = 0;
static const float StaticConst34 = 0;
static const float StaticConst35 = 0;
static const float StaticConst36 = 0;
static const float StaticConst37 = 0;
static const float StaticConst38 = 0;
static const float StaticConst39 = 0;

// Removing any of the above "fixes" the problem, but so does uncommenting these!
//static const float StaticConst40 = 0;
//static const float StaticConst41 = 0;
//static const float StaticConst42 = 0;
//static const float StaticConst43 = 0;
//static const float StaticConst44 = 0;
//static const float StaticConst45 = 0;

StructuredBuffer<float> Buf;

float calculateSomething(float a, float b, float c, float d)
{
    return 0;
}

float4x4 createMatrix(StructuredBuffer<float> buffer, float otherParam)
{
    float v1 = calculateSomething(0, 0, 0, 0);
    float v2 = calculateSomething(0, 0, 0, 0);
    float v3 = 0;
    float v4 = 0;
    float v5 = 0;
    float v6 = 0;
    float v7 = 0;
    float v8 = 0;
    float v9 = 0;
    float v10 = 0;
    // Removing any of the above "fixes" the problem, but so does uncommenting these!
    //float v11 = 0;
    //float v12 = 0;
    //float v13 = 0;
    return 0;
}

float4x4 getMatrix()
{
    return createMatrix(Buf, 0);
}

void doNothing(float param) { }

struct NestedStruct1
{
    float4x4 matrixMember;

    void init()
    {
        getMatrix();
        matrixMember = getMatrix();
    }
};

struct NestedStruct2
{
    NestedStruct1 nested1;
    float2x2 otherMember;
    void setNested1(NestedStruct1 val) { nested1 = val; }
};

struct NestedStruct3
{
    NestedStruct2 nested2;
    void setNested2(NestedStruct2 val) { nested2 = val; }
};

struct NestedStruct4
{
    NestedStruct3 nested3;
    void setNested3(NestedStruct3 val) { nested3 = val; }
};

void main(
    float In0 : TEXCOORD0,
    float In1 : TEXCOORD1,
    float In2 : TEXCOORD2,
    float In3 : TEXCOORD3,
    out float4 Position : SV_Position)
{
    NestedStruct4 s4;
    NestedStruct3 s3;
    NestedStruct2 s2;
    NestedStruct1 s1;

    doNothing(0);
    doNothing(0);
    doNothing(0);
    doNothing(0);
    doNothing(0);
    doNothing(0);
    doNothing(0);

    s1.init();
    s2.setNested1(s1);
    s3.setNested2(s2);
    s4.setNested3(s3);

    Position = 0;
}