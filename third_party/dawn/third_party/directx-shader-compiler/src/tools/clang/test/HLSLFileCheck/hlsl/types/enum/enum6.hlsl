// RUN: %dxc -E main -T vs_6_0 -HV 2017 %s | FileCheck %s
// CHECK: call void @dx.op.storeOutput.i16(i32 5, i32 2, i32 0, i8 0, i16 3)

enum class E : min16int {
  E1 = 3,
  E2
};
 
struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    E e : FOO;
};

PSInput main(float4 position: POSITION, float4 color: COLOR) {
    float aspect = 320.0 / 200.0;
    PSInput result;
    result.position = position;
    result.position.y *= aspect;
    result.color = color;
    result.e = E::E1;
    return result;
}