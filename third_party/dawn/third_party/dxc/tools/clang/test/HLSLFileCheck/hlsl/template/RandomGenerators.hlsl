// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0x404FD93640000000)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0x4047269780000000)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0x4045A83E40000000)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0x4045669660000000)

struct MyRandomGeneratorA
{
  float seed;
  float Rand()
  {
    // Not a good random number generator
    seed = (seed * 17 + 57) / 99.0;
    return seed;
  }
};

struct MyRandomGeneratorB
{
  float Rand()
  {
    // An even worse random number generator
    return 42.0;
  }
};

template<typename GeneratorType>
float4 Model(GeneratorType Generator)
{
 float4 E;
 E.x = Generator.Rand();
 E.y = Generator.Rand();
 E.z = Generator.Rand();
 E.w = Generator.Rand();
 return E;
};

float4 main() : SV_TARGET {
 MyRandomGeneratorA genA;
 genA.seed = 123.0;
 MyRandomGeneratorB genB;

 return Model<MyRandomGeneratorA>(genA) + Model<MyRandomGeneratorB>(genB);
}
