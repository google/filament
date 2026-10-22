// RUN: %dxc -Tlib_6_8 -Wno-unused-value -verify %s

// Tests that the compiler is well-behaved with regard to uses of incomplete types.
// Regression test for GitHub #2058, which crashed in this case.


struct S; // expected-note 24 {{forward declaration of 'S'}}
template <int N> struct T; // expected-note 4 {{template is declared here}}

ConstantBuffer<S> CB; // expected-error {{variable has incomplete type 'S'}}
ConstantBuffer<T<1> > TB; // expected-error {{implicit instantiation of undefined template 'T<1>'}}

S s; // expected-error {{variable has incomplete type 'S'}}
T<1> t; // expected-error {{implicit instantiation of undefined template 'T<1>'}}

cbuffer BadBuffy {
  S cb_s; // expected-error {{variable has incomplete type 'S'}}
  T<1> cb_t; // expected-error {{implicit instantiation of undefined template 'T<1>'}}
};

tbuffer BadTuffy {
  S tb_s; // expected-error {{variable has incomplete type 'S'}}
  T<1> tb_t; // expected-error {{implicit instantiation of undefined template 'T<1>'}}
};

S func( // expected-error {{incomplete result type 'S' in function definition}}
  S param) // expected-error {{variable has incomplete type 'S'}}
{
  S local; // expected-error {{variable has incomplete type 'S'}}
  return (S)0; // expected-error {{'S' is an incomplete type}}
}

[shader("geometry")]
[maxvertexcount(3)]
void gs_point(line S e, // expected-error {{variable has incomplete type 'S'}}
              inout PointStream<S> OutputStream0) {} // expected-error {{variable has incomplete type 'S'}}

[shader("geometry")]
[maxvertexcount(12)]
void gs_line(line S a, // expected-error {{variable has incomplete type 'S'}}
             inout LineStream<S> OutputStream0) {} // expected-error {{variable has incomplete type 'S'}}


[shader("geometry")]
[maxvertexcount(12)]
void gs_line(line S a, // expected-error {{variable has incomplete type 'S'}}
             inout TriangleStream<S> OutputStream0) {} // expected-error {{variable has incomplete type 'S'}}


[shader("domain")]
[domain("tri")]
void ds_main(OutputPatch<S, 3> TrianglePatch) {} // expected-error{{variable has incomplete type 'S'}}

void patch_const(InputPatch<S, 3> inpatch, // expected-error{{variable has incomplete type 'S'}}
                 OutputPatch<S, 3> outpatch) {} // expected-error{{variable has incomplete type 'S'}}

[shader("hull")]
[domain("tri")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(32)]
[patchconstantfunc("patch_const")]
void hs_main(InputPatch<S, 3> TrianglePatch) {} // expected-error{{variable has incomplete type 'S'}}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
// expected-error@+1{{Broadcasting node shader 'broadcast' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
void broadcast(DispatchNodeInputRecord<S> input,  // expected-error{{variable has incomplete type 'S'}}
                NodeOutput<S> output) // expected-error{{variable has incomplete type 'S'}}
{
  ThreadNodeOutputRecords<S> touts; // expected-error{{variable has incomplete type 'S'}}
  GroupNodeOutputRecords<S> gouts; // expected-error{{variable has incomplete type 'S'}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(8,1,1)]
void coalesce(GroupNodeInputRecords<S> input) {} // expected-error{{variable has incomplete type 'S'}}

[Shader("node")]
[NodeLaunch("thread")]
void threader(ThreadNodeInputRecord<S> input) {} // expected-error{{variable has incomplete type 'S'}}
