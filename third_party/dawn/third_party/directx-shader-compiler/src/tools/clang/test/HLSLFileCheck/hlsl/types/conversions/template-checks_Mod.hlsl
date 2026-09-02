// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: bufferUpdateCounter
// CHECK: bufferUpdateCounter
// CHECK: bufferUpdateCounter

Texture2D<float4> t_float4;

matrix<bool, 1, 2> m_bool;

// TODO: when vector is implemented, enable these
// vector<float, 4> v_float;
// vector<float, 5> v_float_large;
// vector<float, 0> v_float_small;
// vector<SamplerState, 2> v_obj_sampler;

struct s_float2_float3
{
  float2 f_float2;
  float3 f_float3;
};

struct s_float4_sampler
{
  float4 f_float4;
  SamplerState f_sampler;
};

struct Data {
  float4 d;
};

AppendStructuredBuffer<Data> asb;
ConsumeStructuredBuffer<Data> csb;

Data f4;

void main() {

  // expected-note@? {{candidate function template not viable: requires single argument 'value', but no arguments were provided}}
  asb.Append(f4);
  // expected-note@? {{candidate function template not viable: requires single argument 'value', but 2 arguments were provided}}
  Data t = csb.Consume();
  asb.Append(t);
}