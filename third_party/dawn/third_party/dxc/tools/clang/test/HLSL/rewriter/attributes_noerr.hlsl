// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T ps_5_1 attributes.hlsl

// The following is a directive to override default behavior for "VerifyHelper.py fxc RunAttributes".  When this is specified, main shader must be defined manually.
// :FXC_VERIFY_ARGUMENTS: /T ps_5_1 /E main

//int loop_before_assignment() {
//  // fxc warning X3554: unknown attribute loop, or attribute invalid for this statement
//  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} 
//  int val = 2;
//  return val;
//}

//int loop_before_return() {
//  // fxc warning X3554: unknown attribute loop, or attribute invalid for this statement
//  [loop] // expected-warning {{attribute 'loop' can only be applied to 'for', 'while' and 'do' loop statements}} fxc-warning {{X3554: unknown attribute loop, or attribute invalid for this statement}} 
//  return 0;
//}

int unroll_noarg() {
  int result = 2;
  
  [unroll]
  for (int i = 0; i < 100; i++) result++;
  
  return result;
}

int unroll_zero() {
  int result = 2;
  
  [unroll(0)]
  for (int i = 0; i < 100; i++) result++;
  
  return result;
}

int unroll_one() {
  int result = 2;
  
  [unroll(1)]
  for (int i = 0; i < 100; i++) result++;
  
  return result;
}

int short_unroll() {
  int result = 2;
  
  [unroll(2)]
  for (int i = 0; i < 100; i++) result++;

  //[unroll("2")] // expected-error {{'unroll' attribute requires an integer constant}} fxc-warning {{X3554: cannot match attribute unroll, parameter 1 is expected to be of type int}} fxc-warning {{X3554: unknown attribute unroll, or attribute invalid for this statement, valid attributes are: loop, fastopt, unroll, allow_uav_condition}} 
  //for (int j = 0; j < 100; j++) result++;

  return result;
}

int long_unroll() {
  int result = 2;
  
  [unroll(200)]
  for (int i = 0; i < 100; i++) result++;

  return result;
}

//int neg_unroll() {
//  int result = 2;
//  
//  // fxc error X3084: cannot match attribute unroll, non-uint parameters found
//  [unroll(-1)] // expected-warning {{attribute 'unroll' must have a uint literal argument}} fxc-error {{X3084: cannot match attribute unroll, non-uint parameters found}} 
//  for (int i = 0; i < 100; i++) result++;
//
//  return result;
//}

//int flt_unroll() {
//  int result = 2;
//  
//  // fxc warning X3554: cannot match attribute unroll, parameter 1 is expected to be of type int
//  // fxc warning X3554: unknown attribute unroll, or attribute invalid for this statement, valid attributes are: loop, fastopt, unroll, allow_uav_condition
//  [unroll(1.5)] // expected-warning {{attribute 'unroll' must have a uint literal argument}} fxc-warning {{X3554: cannot match attribute unroll, parameter 1 is expected to be of type int}} fxc-warning {{X3554: unknown attribute unroll, or attribute invalid for this statement, valid attributes are: loop, fastopt, unroll, allow_uav_condition}} 
//  for (int i = 0; i < 100; i++) result++;
//
//  return result;
//}

RWByteAddressBuffer bab;
int bab_address;
bool g_bool;
uint g_uint;
uint g_dealiasTableOffset;
uint g_dealiasTableSize;

int uav() {
  // fxc not complaining.
  //int result = bab_address;
  //while (bab.Load(result) != 0) {
  //  bab.Store(bab_address, g_dealiasTableOffset - result);
  //  result++;
  //}
  //return result;
  uint i;
  [allow_uav_condition]
  for (i = g_dealiasTableOffset; i < g_dealiasTableSize; ++i) {
  }
  return i;
}

//[domain] int domain_fn_missing() { return 1; }          // expected-error {{'domain' attribute takes one argument}} 
//[domain()] int domain_fn_empty() { return 1; }          // expected-error {{'domain' attribute takes one argument}} fxc-error {{X3000: syntax error: unexpected token ')'}} 
//[domain("blerch")] int domain_fn_bad() { return 1; }    // expected-error {{attribute 'domain' must have one of these values: tri,quad,isoline}} 
//[domain("quad")] int domain_fn() { return 1; }          /* fxc-warning {{X3554: unknown attribute domain, or attribute invalid for this statement}} */
//[domain(1)] int domain_fn_int() { return 1; }           // expected-error {{attribute 'domain' must have a string literal argument}} 
//[domain("quad","quad")] int domain_fn_mul() { return 1; } // expected-error {{'domain' attribute takes one argument}} 
//[instance] int instance_fn() { return 1; }             // expected-error {{'instance' attribute takes one argument}} fxc-warning {{X3554: unknown attribute instance, or attribute invalid for this statement}} 
//[maxtessfactor] int maxtessfactor_fn() { return 1; }   // expected-error {{'maxtessfactor' attribute takes one argument}} fxc-warning {{X3554: unknown attribute maxtessfactor, or attribute invalid for this statement}} 
//[numthreads] int numthreads_fn() { return 1; }         // expected-error {{'numthreads' attribute requires exactly 3 arguments}} fxc-warning {{X3554: unknown attribute numthreads, or attribute invalid for this statement}} 
//[outputcontrolpoints] int outputcontrolpoints_fn() { return 1; } // expected-error {{'outputcontrolpoints' attribute takes one argument}} fxc-warning {{X3554: unknown attribute outputcontrolpoints, or attribute invalid for this statement}} 
//[outputtopology] int outputtopology_fn() { return 1; } // expected-error {{'outputtopology' attribute takes one argument}} fxc-warning {{X3554: unknown attribute outputtopology, or attribute invalid for this statement}} 
//[partitioning] int partitioning_fn() { return 1; }     // expected-error {{'partitioning' attribute takes one argument}} fxc-warning {{X3554: unknown attribute partitioning, or attribute invalid for this statement}} 
//[patchconstantfunc] int patchconstantfunc_fn() { return 1; } // expected-error {{'patchconstantfunc' attribute takes one argument}} fxc-warning {{X3554: unknown attribute patchconstantfunc, or attribute invalid for this statement}} 



struct HSFoo
{
    float3 pos : POSITION;
};

Texture2D<float4> tex1[10] : register( t20, space10 );

//[domain(123)]     // expected-error {{attribute 'domain' must have a string literal argument}} 
//[partitioning()]  // expected-error {{'partitioning' attribute takes one argument}} fxc-error {{X3000: syntax error: unexpected token ')'}} 
//[outputtopology("not_triangle_cw")] // expected-error {{attribute 'outputtopology' must have one of these values: point,line,triangle,triangle_cw,triangle_ccw}} 
//[outputcontrolpoints(-1)] // expected-warning {{attribute 'outputcontrolpoints' must have a uint literal argument}} 
//[patchconstantfunc("PatchFoo", "ExtraArgument")] // expected-error {{'patchconstantfunc' attribute takes one argument}} 
//void all_wrong() { }

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("PatchFoo")]
HSFoo HSMain( InputPatch<HSFoo, 16> p, 
              uint i : SV_OutputControlPointID,
              uint PatchID : SV_PrimitiveID )
{
    HSFoo output;
    float4 r = float4(p[PatchID].pos, 1);
    r += tex1[r.x].Load(r.xyz);
    output.pos = p[i].pos + r.xyz;
    return output;
}

float4 cp4[2];
int4 i4;
float4 cp5;
float4x4 m4;
float f;

struct global_struct { float4 cp5[5]; };

struct main_output
{
  //float4 t0 : SV_Target0;
  float4 p0 : SV_Position0;
};

float4 myexpr() { return cp5; }
static const float4 f4_const = float4(1, 2, 3, 4);
bool b;
int clip_index;
static const bool b_true = true;
global_struct gs;
float4 f4;

//
// Things we can't do in clipplanes:
//
// - use literal in any part of expression
// - use static const in any part of expression
// - use ternary operator
// - use binary operator
// - make function calls
// - use indexed expressions
// - implicit conversions
// - explicit conversions
//

//// fxc error X3084: Clip plane attribute parameters must be non-literal constants
//[clipplanes(float4(1, 2, f, 4))] // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_literals();
//
//[clipplanes(b)]           // expected-error {{clipplanes argument must be a float4 type but is 'bool'}} 
//void clipplanes_const();
//
//// fxc error X3084: Clip plane attribute parameters must be non-literal constants
//[clipplanes(f4_const)]    // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_const();  
//
//// fxc error X3084: Clip plane attribute parameters must be non-literal constants
//[clipplanes(b ? cp4[clip_index] : cp4[clip_index])] // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_ternary();
//
//// fxc error X3084: Clip plane attribute parameters must be non-literal constants
//[clipplanes(cp5 + cp5)]      // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_binop();
//
//// fxc error X3682: expressions with side effects are illegal as attribute parameters
//[clipplanes(myexpr())]      // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_call();
//
//// fxc error X3084: Indexed expressions are illegal as attribute parameters
//[clipplanes(cp4[clip_index])] // expected-error {{invalid expression for clipplanes argument array subscript: must be numeric literal}} 
//void clipplanes_bad_indexed();

//
// Thing we can do in clipplanes:
//
// - index into array
// - index into vector
// - swizzle (even into an rvalue, even from scalar)
// - member access
// - constructors
// 

//// index into swizzle into r-value - allowed in fxc, disallowed in new compiler
//[clipplanes(cp4[0].xyzw)]               // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_swizzle();
//
//// index into vector - allowed in fxc, disallowed in new compiler
//[clipplanes(cp4[0][0].xxxx)]            // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_vector_index();
//
//// index into matrix - allowed in fxc, disallowed in new compiler
//[clipplanes(m4[0])]                     // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_matrix_index();
//
//// constructor - allowed in fxc, disallowed in new compiler
//[clipplanes(float4(f, f, f, f))]        // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_matrix_index();
//
//// swizzle from scalar - allowed in fxc, disallowed in new compiler
//[clipplanes(f.xxxx)]                    // expected-error {{invalid expression for clipplanes argument: must be global reference, member access or array subscript}} 
//void clipplanes_bad_scalar_swizzle();

[clipplanes(
  f4,         // simple global reference
  cp4[0],     // index into array
  gs.cp5[2]   // use '.' operator
  )]
float4 clipplanes_good();


[clipplanes(
  (f4),         // simple global reference
  cp4[(0)],     // index into array
  (gs).cp5[2],  // use '.' operator
  ((gs).cp5[2]) // use '.' operator
  )]
float4 clipplanes_good_parens();


[earlydepthstencil]
float4 main() : SV_Target0 {
    int val = 2;

    //[earlydepthstencil] // expected-error {{attribute is valid only on functions}} 
    //float2 f2 = { 1, 2 };
    //
    //[loop]
    //for (int i = 0; i < 4; i++) { // expected-note {{previous definition is here}}
    //    val *= 2;
    //}
    //
    //[unroll]
    //// fxc warning X3078: 'i': loop control variable conflicts with a previous declaration in the outer scope; most recent declaration will be used
    //for (int i = 0; i < 4; i++) { // expected-warning {{redefinition of 'i' shadows declaration in the outer scope; most recent declaration will be used}}
    //    val *= 2;
    //}
    //
    //// fxc error X3524: can't use loop and unroll attributes together
    //[loop][unroll] // expected-error {{loop and unroll attributes are not compatible}} fxc-error {{X3524: can't use loop and unroll attributes together}} 
    //for (int k = 0; k < 4; k++) { val *= 2; } // expected-note {{previous definition is here}}
    //
    //// fxc error X3524: can't use fastopt and unroll attributes together
    //[fastopt][unroll] // expected-error {{fastopt and unroll attributes are not compatible}} fxc-error {{X3524: can't use fastopt and unroll attributes together}} 
    //for (int k = 0; k < 4; k++) { val *= 2; } // expected-warning {{redefinition of 'k' shadows declaration in the outer scope; most recent declaration will be used}}
    //
    //loop_before_assignment();
    //loop_before_return();
    
    val = 2;
    
    [loop]
    do { val *= 2; } while (val < 10);

    [fastopt]
    while (val > 10) { val--; }
    
    [branch]
    if (g_bool) {
      val += 4;
    }
    
    [flatten]
    if (!g_bool) {
      val += 4;
    }

    [flatten]
    switch (g_uint) {
      // fxc error X3533: non-empty case statements must have break or return
      // case 0: val += 100;
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }
    
    [branch]
    switch (g_uint) {
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }
    
    [forcecase]
    switch (g_uint) {
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }
    
    [call]
    switch (g_uint) {
      case 1: val += 101; break;
      case 2:
      case 3: val += 102; break;
      break;
    }
    
    /*val += domain_fn();
    val += instance_fn();
    val += maxtessfactor_fn();
    val += numthreads_fn();
    val += outputcontrolpoints_fn();
    val += outputtopology_fn();
    val += partitioning_fn();
    val += patchconstantfunc_fn();*/

    val += long_unroll();
    val += short_unroll();
    //val += neg_unroll();
    //val += flt_unroll();
    val += uav();
    
    return val;
}

// Test NoInline
[noinline] bool Test_noinline() {
  //[noinline] bool b = false;                  /* expected-warning {{'noinline' attribute only applies to functions}} fxc-pass {{}} */
  //[noinline] while (g_bool) return g_bool;    /* expected-error {{'noinline' attribute cannot be applied to a statement}} fxc-pass {{}} */
  return true;
}
