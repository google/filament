// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

Texture2D<float4> t_float4;
//Texture2D<SamplerState> t_obj_sampler;          /* expected-error {{'SamplerState' is an object and cannot be used as a type parameter}} fxc-error {{X3124: object element type cannot be an object type}} */
//Texture2D<Texture2D<float4> > t_obj_tex;        /* expected-error {{'Texture2D<float4>' cannot be used as a type parameter}} fxc-error {{X3124: object element type cannot be an object type}} */
//
//matrix<SamplerState, 1, 2> m_obj_sampler;       /* expected-error {{'SamplerState' cannot be used as a type parameter where a scalar is required}} fxc-error {{X3123: matrix element type must be a scalar type}} */
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
//RWBuffer<s_float2_float3> rwb_struct;           /* fxc-error {{X3037: elements of typed buffers and textures must fit in four 32-bit quantities}} */

struct s_float4_sampler
{
  float4 f_float4;
  SamplerState f_sampler;
};
//RWBuffer<s_float4_sampler> rwb_struct_objs; /* expected-error {{'SamplerState' is an object and cannot be used as a type parameter}} expected-note {{usage of 'SamplerState' found in field 'f_sampler' of type 's_float4_sampler'}} fxc-pass {{}} */

void vain() {
  // Nothing to do here.
  AppendStructuredBuffer<float4> asb;
  float4 f4;
  //asb.Append();       /* expected-error {{no matching member function for call to 'Append'}} fxc-error {{X3013:     AppendStructuredBuffer<float4>.Append(<unknown>)}} fxc-error {{X3013: 'Append': no matching 0 parameter intrinsic method}} fxc-error {{X3013: Possible intrinsic methods are:}} */
  // expected-note@? {{candidate function template not viable: requires single argument 'value', but no arguments were provided}}
  //asb.Append(f4, f4); /* expected-error {{no matching member function for call to 'Append'}} fxc-error {{X3013:     AppendStructuredBuffer<float4>.Append(<unknown>)}} fxc-error {{X3013: 'Append': no matching 2 parameter intrinsic method}} fxc-error {{X3013: Possible intrinsic methods are:}} */
  asb.Append(f4);
  // expected-note@? {{candidate function template not viable: requires single argument 'value', but 2 arguments were provided}}
}