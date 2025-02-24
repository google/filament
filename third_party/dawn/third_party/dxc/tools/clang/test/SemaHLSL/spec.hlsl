// RUN: %dxc -Tlib_6_3 -HV 2018 -verify %s
// RUN: %dxc -Tps_6_0 -HV 2018 -verify %s

// This file provides test cases that are cross-references to the HLSL
// specification.

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T ps_5_1 spec.hlsl

namespace ns_general {
// * General
// ** Scope
// ** Normative references
// ** Definitions
// ** Implementation compliance
// ** Structure of this specification
// ** Syntax notation
// ** The HLSL memory model
// ** The HLSL object model

// Objects can contain other objects, called subobjects. A subobject can be a
// member subobject, a base class subobject, or an array element.
  void fn(inout uint a) { a += 2; }
  void subobjects() {
    class Class { uint field; };
    class SuperClass { Class C; uint field; };

    uint scalar = 0;
    Class C = { 0 };
    SuperClass SC = { 0, 0 };
    uint2 u2 = 0;
    uint arr2[2] = { 0, 0 };
    uint2x2 mat2 = 0;
    fn(scalar);
    fn(C.field);
    fn(SC.C.field);
    fn(u2.x);
    fn(mat2._11);
  }

// ** Program execution
// ** Multi-threaded executions and data races
// ** Acknowledgments

}

namespace ns_lexical {
// * Lexical Conventions
// ** Separate translation
// ** Phases of translation
// ** Character sets
// ** Trigraph sequences
// ** Preprocessing tokens
// ** Alternative tokens
// ** Tokens
// ** Comments
// ** Header names
// ** Preprocessing numbers
// ** Identifiers
// ** Keywords
// ** Operators and punctuators
// ** Literals
}

namespace ns_basic {
// * Basic Concepts
// ** Declarations and definitions
// ** One definition rule
// ** Scope
// ** Name lookup
// ** Program and linkage
// ** Start and termination
// ** Storage duration
// ** Object lifetime

// ** Types

//extern int arr[]; // the type of arr is incomplete ; fxc-eerror {{error X3074: 'ns_basic::arr': implicit array missing initial value}}
//typedef int UNKA[]; // UNKA is an incomplete type ; fxc-error {{error X3072: array dimensions of type must be explicit}}
//  cbuffer C; // fxc-error {{error X3000: syntax error: unexpected token ';'}}

  string s = "my string"; // expected-error {{string declaration may only appear in global scope}}
  int int_var;
  uint uint_var;
  dword dword_var;
  min16int min16int_var;
  min16uint mint6uint_var;

  int fn_dword_overload(dword d) { return 1; }

  void fn_void_conversions() {
    //(void)1; // fxc-error {{cannot convert from 'int' to 'void'}}
  }

// ** Lvalues and rvalues
// ** Alignment
}

namespace ns_std_conversions {
// * Standard conversions

  struct s_mixed { int2 i2; float2 f2; };
  struct s_mixed2 { s_mixed s; int arr[4]; };
  struct s_i4i4 { int4 i4; int4 i4_2; };
  struct s_i4i4_2 { int4 i4; int4 i4_2; };
  void fn_mixed(s_mixed a) { }
  s_mixed fn_int_mixed(int a) { s_mixed r = {0,0,2,3}; return r; }
  
  void fn_foo(inout s_i4i4 v) { v.i4.z += 1; }
  void fn_init_or_assign() {
    // Standard conversions will occur as initialization including use of an
    // argument in a function call and expression in a return statement.
    //
    // Assignment has different rules (in particular, flattening does not occur).
    //
    // However, flattening occurs only for initialization lists, which are only
    // allowed variable initialization. So the distinction is moot.
    s_mixed s = { 1, 2, 1, 2 };
    s_mixed2 s2 = { s, 1, 2, 3, 4 }; 
    s_i4i4 si4i4 = { 1, 2, 3, 4, 1, 2, 3, 4 };
    si4i4 = (s_i4i4)s2;

//TODO: Implicit casting must be between two structurally identical types (as
//compared by flattening structure); explicit casting is required when the
//right-hand type has more elements.
    s_i4i4_2 si4i42 = { 1, 2, 3, 4, 1, 2, 3, 4 };
    si4i4 = si4i42;

    // fn_foo((s_i4i4)s2); // cannot use casts on l-values
    float4 f4 = (float4)s;
    int2 i2 = (int2)s; // explicit cast 
    int2 io = (int2)si4i4;
  }

// ** Lvalue-to-rvalue conversion
// ** Vector conversions
  void fn_f1(float1 a) { }
  void fn_f4(float4 b) { }
  void fn_u4(float4 b) { }
  void fn_f(float f) { }
  void fn_iof(inout float f) { f += 1; }
  void fn_iof1(inout float1 f) { f.x += 1; }
  void fn_vector_conversions() {
    float f = 1;
    int i = 1; uint u = 1;
    float1 f1 = f; uint1 u1 = { 1 };
    float3 f3 = { 1, 2, 3 };
    float4 f4 = { 1, 2, 3, 4}; uint4 u4 = { 1, 2, 3, 4 };
    fn_f1(f);  // vector splat
    fn_f4(f);  // vector splat
    fn_f1(i);  // vector splat
    fn_f4(i);  // vector splat
    fn_u4(f4); // vector element
    fn_f4(u4); // vector element
    f3 = f4;   // expected-warning {{implicit truncation of vector type}}
    // f4 = f3; // fxc-error {{error X3017: cannot implicitly convert from 'float3' to 'float4'}}
    fn_iof(f1); // inout case (float1->float - vector single element conversion; float->float1 vector splat)
    fn_iof1(u); // inout case (uint->float1 - vector splat; float1->uint vector single element conversion)
  }

  struct struct_f44 { float4x4 f44; };
  void fn_f14(float1x4 a) { }
  void fn_f22(float2x2 a) { }
  void fn_f41(float4x1 a) { }
  void fn_f44(float4x4 a) { }
  void fn_matrix_conversions() {
    float f = 1; uint u = 1;
    float1 f1 = 1; uint1 u1 = 1;
    float2 f2 = { 1, 2 }; uint2 u2 = { 1, 2 };
    float3 f3 = 0; uint3 u3 = 0;
    float4 f4 = { 1, 2, 3, 4 }; uint4 u4 = { 1, 2, 3, 4 };
    struct_f44 s44 = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};
    float1x1 f11 = { 1 }; uint1x1 u11 = { 1 };
    float1x2 f12 = { 1, 2 };
    float1x3 f13 = 0;
    float2x2 f22 = f4; uint2x2 u22 = u4;
    float2x3 f23 = 0;
    float3x1 f31 = 0;
    float3x2 f32 = 0;
    float3x3 f33 = 0;
    float4x4 f44 = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};
    float1x4 f14 = {1,2,3,4}; uint1x4 u14 = {1,2,3,4};
    float4x1 f41 = {1,2,3,4}; uint4x1 u41 = {1,2,3,4};
    uint4x4 u44 = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};
    fn_f44(f);  // matrix splat conversion
    //fn_f44(f4); // no matrix splat conversion fxc-error {{error X3017: cannot implicitly convert from 'float4' to 'float4x4'}}
    //fn_f44(s44); // no matrix splat conversion fxc-error {{error X3017: cannot convert from 'struct ns_std_conversions::struct_f44' to 'float4x4'}}
    fn_f14(f4);  // matrix vector conversion
    fn_f22(f4);  // matrix vector conversion (not vector-like, but still same element count)
    fn_f22(u4);  // matrix vector conversion (with element implicit conversion)
    fn_f44(u44); // matrix element conversion
    // fn_f14(u41); // no matrix element conversion from different shape, even with same element count - fxc-error {{cannot implicitly convert from 'uint4x1' to 'float4'}}
    //fn_f41(u14); // no matrix element conversion from different shape, even with same element count - fxc-error {{cannot implicitly convert from 'uint4' to 'float4x1'}}
    //fn_f14(f41); // no matrix from a different shape, even with the same type and element count - fxc-error {{cannot implicitly convert from 'float4x1' to 'float4'}}
    fn_f14(1);

    u = f11; // matrix single element conversion
    // expected-warning@+1 {{implicit truncation of vector type}}
    u = f14; // matrix scalar truncation conversion

    u2 = f11; // matrix single element vector conversion
    u4 = f22; // matrix to vector conversion
    //u3 = f22; // cannot convert if target has less
    //u3 = f12; // cannot convert if target has more

    u44 = f44; // matrix element-type conversion
    // expected-warning@+1 {{implicit truncation of vector type}}
    u22 = f44; // can convert to smaller
    // expected-warning@+1 {{implicit truncation of vector type}}
    u22 = f33; // can convert to smaller
    // expected-warning@+1 {{implicit truncation of vector type}}
    f32 = f33; // can convert as long as each dimension is smaller
    //u44 = f22; // cannot convert to bigger
  }

// ** Qualification conversions
// ** Integral promotions
// ** Floating point promotion
// ** Integral conversions
// ** Floating point conversions
// ** Floating-integral conversions
// ** Pointer conversions
// ** Pointer to member conversions
// ** Boolean conversions
// ** Integer conversion rank
}

// * Expressions
// ** Primary expressions
// ** Postfix expressions
// ** Unary expressions
// ** Explicit type conversion (cast notation)
// ** Pointer-to-member operators
// ** Multiplicative operators
// ** Additive operators
// ** Shift operators
// ** Relational operators
// ** Equality operators
// ** Bitwise AND operator
// ** Bitwise exclusive OR operator
// ** Bitwise inclusive OR operator
// ** Logical AND operator
// ** Logical OR operator
// ** Conditional operator
// ** Assignment and compound assignment operators
// ** Comma operator
// ** Constant expressions

// * Statements
// ** Labeled statement
// ** Expression statement
// ** Compound statement or block
// ** Selection statements
// ** Iteration statements
// ** Jump statements
// ** Declaration statement
// ** Ambiguity resolution

// * Declarations
// ** Specifiers
// ** Enumeration declarations
// ** Namespaces
// ** The asm declaration
// ** Linkage specifications
// ** Attributes

// * Declarators
// ** Type names
// ** Ambiguity resolution
// ** Meaning of declarators
// ** Function definitions
// ** Initializers

// * Classes
// ** Class names
// ** Class members
// ** Member functions
// ** Static members
// ** Unions
// ** Bit-fields
// ** Nested class declarations
// ** Local class declarations
// ** Nested type names

// * Derived classes
// ** Multiple base classes
// ** Member name lookup
// ** Virtual functions
// ** Abstract classes

// * Member access control
// ** Access specifiers
// ** Accessibility of base classes and base class members
// ** Access declarations
// ** Friends
// ** Protected member access
// ** Access to virtual functions
// ** Multiple access
// ** Nested classes

// * Special member functions
// ** Constructors
// ** Temporary objects
// ** Conversions
// ** Destructors
// ** Free store
// ** Initialization
// ** Construction and destruction
// ** Copying and moving class objects
// ** Inheriting Constructors

namespace ns_overloading {
// * Overloading
// ** Overloadable declarations
  int f(int a) { return a; } // expected-note {{previous definition is here}}
  int f(const int a) { return a; } // expected-error {{redefinition of 'f'}}
  int f_default_0(int a = 3);
  //int f_default_0(int a = 4); // error X3114: 'a': default parameters can only be provided in the first prototype
  int f_default(int a = 1) { return a; } // expected-note {{previous definition is here}}
  int f_default(int a = 3); // expected-error {{redefinition of default argument}}
//  int f_default_args(int a, int b = 0);
  int f_default_args(int a = 0, int b); // expected-error {{missing default argument on parameter 'b'}}
  int f_default_args() { return 1; }
  int f_default_args_equiv(int a);
  int f_default_args_equiv(int a = 1);
  int f_default_args_equiv(int a) { return a; }
// ** Declaration matching
// ** Overload resolution
// ** Address of overloaded function
// ** Overloaded operators
// ** Built-in operators

  float test() {
    int a = 1;
    // f(a); // error X3067: 'f': ambiguous function call
    return f_default_args_equiv();
  }
}


// * Templates
// ** Template parameters
// ** Names of template specializations
// ** Template arguments
// ** Type equivalence
// ** Template declarations
// ** Name resolution
// ** Template instantiation and specialization
// ** Function template specializations

// * Exception handling
// ** Throwing an exception
// ** Constructors and destructors
// ** Handling an exception
// ** Exception specifications
// ** Special functions

// * Preprocessing directives
// ** Conditional inclusion
// ** Source file inclusion
// ** Macro replacement
// ** Line control
// ** Error directive
// ** Pragma directive
// ** Null directive
// ** Predefined macro names
// ** Pragma operator

// * Library introduction
// ** General
// ** The C standard library
// ** Definitions
// ** Additional definitions
// ** Method of description (Informative)
// ** Library-wide requirements

// * Language support library
// ** General
// ** Types
// ** Implementation properties
// ** Integer types
// ** Start and termination
// ** Dynamic memory management
// ** Type identification
// ** Exception handling
// ** Initializer lists
// ** Other runtime support

// * Diagnostics library
// ** General
// ** Exception classes
// ** Assertions
// ** Error numbers
// ** System error support

// * General utilities library
// ** General
// ** Requirements
// ** Utility components
// ** Tuples
// ** Class template bitset
// ** Compile-time rational arithmetic
// ** Metaprogramming and type traits
// ** Function objects
// ** Memory
// ** Time utilities
// ** Date and time functions
// ** Class type_index

// * References

// * Appendix - Grammar summary
// ** Keywords
// ** Lexical conventions
// ** Basic concepts
// ** Expressions
// ** Statements
// ** Declarations
// ** Declarators
// ** Classes
// ** Derived classes
// ** Special member functions
// ** Overloading
// ** Templates
// ** Exception handling
// ** Preprocessing directives

struct main_output
{
  float4 t0 : SV_Target0;
  //float4 p0 : SV_Position0;
};

main_output main() {
  main_output o;
  o.t0 = 0;
  o.t0.x = ns_overloading::test();
  ns_std_conversions::fn_init_or_assign();
  ns_std_conversions::fn_vector_conversions();
  ns_std_conversions::fn_matrix_conversions();
  return o;
}
