// RUN: %clang_cc1 -fsyntax-only -Wno-unused-value -ffreestanding -verify %s

float f_arr_empty_init[] = { 1, 2, 3 };
//float f_arr_empty_pack[] = { 1, 2 ... }; // expected-error {{expansion is unsupported in HLSL}}

struct s_arr_i_f { int i; float f; };
s_arr_i_f arr_struct_none[] = { }; // TODO: this should fail - see comments in HLSLExternalSource::InitializeInitSequenceForHLSL
s_arr_i_f arr_struct_one[] = { 1, 2 };
//s_arr_i_f arr_struct_incomplete[] = { 1, 2, 3 }; // expected-error {{too few elements in vector initialization (expected 4 elements, have 3)}}
s_arr_i_f arr_struct_two[] = { 1, 2, 3, 4 };

int g_int;
//typeof(g_int) g_typeof_int; // expected-error {{unknown type name 'typeof'; did you mean 'typedef'?}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{expected ';' after top level declarator}}
//typedef int (*fn_int)(int); // expected-error {{pointers are unsupported in HLSL}}
//auto g_auto = 3; // expected-error {{'auto' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//__is_signed g_is_signed; // expected-error {{'__is_signed' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//register int g_register; // expected-error {{'register' is a reserved keyword in HLSL}}
//__thread int g_thread; // expected-error {{'__thread' is a reserved keyword in HLSL}}
//thread_local int g_threadLocal; // expected-error {{expected unqualified-id}} expected-error {{unknown type name 'thread_local'}}
//_Thread_local int g_Thread_local; // expected-error {{'_Thread_local' is a reserved keyword in HLSL}}
//_Alignas(float) int g_Alignas; // expected-error {{'_Alignas' is a reserved keyword in HLSL}}
//alignas(float) int g_alignas; // expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{expected ';' after top level declarator}}
//constexpr int g_constexpr = 3; // expected-error {{unknown type name 'constexpr'}} expected-error {{expected unqualified-id}}
//friend int f_friend; // expected-error {{'friend' is a reserved keyword in HLSL}}

// Alternate numerics and builtin types.
//short g_short; // expected-error {{'short' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//long g_long; // expected-error {{'long' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//signed int g_signed_int; // expected-error {{'signed' is a reserved keyword in HLSL}}
unsigned int g_unsigned_int;
//char g_char; // expected-error {{'char' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//_Bool g_Bool; // expected-error {{unknown type name '_Bool'}}
//_vector int altivec_vector; // expected-error {{unknown type name '_vector'}} expected-error {{expected unqualified-id}}

//restrict int g_restrict; // expected-error {{unknown type name 'restrict'}} expected-error {{expected unqualified-id}}

//__underlying_type(int) g_underlying_type; // expected-error {{__underlying_type is unsupported in HLSL}}
//_Atomic(something) g_Atomic; // expected-error {{'_Atomic' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//typeof(g_int) g_anotherInt; // expected-error {{unknown type name 'typeof'; did you mean 'typedef'?}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{expected ';' after top level declarator}}

// More GNU-specific keywords.
//_Decimal32 g__Decimal32; // expected-error {{GNU decimal type extension not supported}}
//_Decimal64 g__Decimal64; // expected-error {{GNU decimal type extension not supported}}
//_Decimal128 g__Decimal128; // expected-error {{GNU decimal type extension not supported}}
//__null g___null; // expected-error {{expected unqualified-id}}
//__alignof g___alignof; // expected-error {{expected unqualified-id}}
//__imag g___imag; // expected-error {{expected unqualified-id}}
//__int128 g___int128; // expected-error {{'__int128' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
//__label__ g___label; // expected-error {{expected unqualified-id}}
//__real g___real; // expected-error {{expected unqualified-id}}
//__FUNCTION__ g___FUNCTION; // expected-error {{expected unqualified-id}}
//__PRETTY__ g___PRETTY; // expected-error {{unknown type name '__PRETTY__'}}

//struct s_with_bitfield {
//  int f_bitfield : 3; // expected-error {{bitfields are not supported in HLSL}}
//};

//struct s_with_friend {
//  friend void some_fn(); // expected-error {{'friend' is a reserved keyword in HLSL}}
//};

//typedef int (*fn_int_const)(int) const; // expected-error {{pointers are unsupported in HLSL}} expected-error {{expected ';' after top level declarator}} expected-warning {{declaration does not declare anything}}
//typedef int (*fn_int_volatile)(int) volatile; // expected-error {{pointers are unsupported in HLSL}} expected-error {{expected ';' after top level declarator}} expected-warning {{declaration does not declare anything}}
//
//void fn_throw() throw() { } // expected-error {{exception specification is unsupported in HLSL}}
//
//// This would otherwise be 'exception specification is unsupported in HLSL', but noexcept is not a keyword for HLSL.
//void fn_noexcept() noexcept { }; // expected-error {{expected function body after function declarator}}
//
//// This would be a failure because of unsupported trailer return types, but we mis-parse it differently.
//auto fn_trailing() -> int { return 1; } ; // expected-error {{'auto' is a reserved keyword in HLSL}} expected-error {{expected function body after function declarator}}
//
//void fn_param_with_default(int val = 1) { } // expected-error {{default arguments are not supported}}
//void fn_with_variadic(int a, ...) { } // expected-error {{variadic arguments is unsupported in HLSL}}
//
//float f_arr_empty_uninit[]; // expected-error {{definition of variable with array type needs an explicit size or an initializer}}
//float f_arr_static[static 3];  // expected-error {{static keyword on array derivation is unsupported in HLSL}}
//float f_arr_star[*];  // expected-error {{variable-length array is unsupported in HLSL}} expected-error {{definition of variable with array type needs an explicit size or an initializer}}
//
//#define <<(x) (x) // expected-error {{macro names must be identifiers}}
//
//typedef int bool; // expected-error {{redeclaration of HLSL built-in type 'bool'}} expected-warning {{typedef requires a name}}

// This would generate an 'unknown pragma ignored' warning, but the default configuration ignores the warning.
#pragma align(4)

// Objective-C @ support for NSString literals.
//const bool b = @"hello" != @"goodbye"; // expected-error {{expected expression}}
//
//int fn_eq_default() = default; // expected-error {{'= default' is a function definition and must occur in a standalone declaration}}
//
//typename typedef float4 TFloat4; // expected-error {{'typename' is a reserved keyword in HLSL}}
//
//class C {
//  int fn_eq_default() = default; // expected-error {{function deletion and defaulting is unsupported in HLSL}}
//
//  // Errors are a bit misleading here, but ultimate we don't support these.
//  void* operator new(); // expected-error {{'operator' is a reserved keyword in HLSL}} expected-error {{pointers are unsupported in HLSL}}
//  void* operator new(int); // expected-error {{'operator' is a reserved keyword in HLSL}} expected-error {{pointers are unsupported in HLSL}}
//  void* operator new(size_t); // expected-error {{'operator' is a reserved keyword in HLSL}} expected-error {{pointers are unsupported in HLSL}}
//
//  C() = delete; // expected-error {{member 'C' has the same name as its class}} expected-error {{HLSL requires a type specifier for all declarations}} expected-error {{constructor cannot have a return type}}
//};
//
//asm ("int 3c"); // expected-error {{expected unqualified-id}}
//@property int at_property_int; // expected-error {{expected unqualified-id}}
//-int minus_int; // expected-error {{expected external declaration}}

static int static_int() { return 1; }

//#import "foo.h" // expected-error {{invalid preprocessing directive}}

//int knr_fn(f) int f; { return 1; } // expected-error {{unknown type name 'f'}} expected-error {{expected ';' after top level declarator}} expected-error {{expected unqualified-id}}

// TODO: this should be an error, but there is no C++ mention in any errors in any case;
// the only error case we would have cared to fix is for Objective C++
// [[attribute]] int g_value;

int is_supported() {
  // GNU Extensions (in impl-reserved namespace)
  //KEYWORD(_Decimal32, KEYALL)
  //KEYWORD(_Decimal64, KEYALL)
  //KEYWORD(_Decimal128, KEYALL)
  //KEYWORD(__null, KEYCXX)
  //KEYWORD(__alignof, KEYALL)
  //KEYWORD(__attribute, KEYALL)
  //KEYWORD(__builtin_choose_expr, KEYALL)
  //KEYWORD(__builtin_offsetof, KEYALL)
  //if (__builtin_types_compatible_p(int, int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
  //KEYWORD(__builtin_va_arg, KEYALL)
  //KEYWORD(__extension__, KEYALL)
  //KEYWORD(__imag, KEYALL)
  //KEYWORD(__int128, KEYALL)
  //KEYWORD(__label__, KEYALL)
  //KEYWORD(__real, KEYALL)
  //KEYWORD(__thread, KEYALL)
  //KEYWORD(__FUNCTION__, KEYALL)
  //KEYWORD(__PRETTY_FUNCTION__, KEYALL)

  // GNU and MS Type Traits
//  if (__has_nothrow_assign(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_nothrow_move_assign(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_nothrow_copy(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_nothrow_constructor(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_trivial_assign(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_trivial_move_assign(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_trivial_copy(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_trivial_constructor(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_trivial_move_constructor(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_trivial_destructor(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__has_virtual_destructor(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_abstract(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_base_of(int, int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_class(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_convertible_to(int, int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_empty(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_enum(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_final(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_literal(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_literal_type(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_pod(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_polymorphic(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_trivial(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_union(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//
//  // Clang-only C++ Type Traits
//  if (__is_trivially_constructible(int)) return 1; // expected-error {{__is_trivially_constructible is unsupported in HLSL}}
//  if (__is_trivially_copyable(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_trivially_assignable(int, int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//
//  // Embarcadero Expression Traits
//  if (__is_lvalue_expr(1)) return 1; // expected-error {{__is_lvalue_expr is unsupported in HLSL}}
//  if (__is_rvalue_expr(1)) return 1; // expected-error {{__is_rvalue_expr is unsupported in HLSL}}
//
//  // Embarcadero Unary Type Traits
//  if (__is_arithmetic(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_floating_point(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_integral(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_complete_type(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_void(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_array(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_function(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_reference(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_lvalue_reference(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_rvalue_reference(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_fundamental(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_object(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_scalar(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_compound(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_pointer(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_member_object_pointer(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_member_function_pointer(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_member_pointer(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_const(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_volatile(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_standard_layout(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_signed(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_unsigned(int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//
//  // Embarcadero Binary Type Traits
//  if (__is_same(int, int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__is_convertible(int, int)) return 1; // expected-error {{type trait functions are not supported in HLSL}}
//  if (__array_rank(int, 1)) return 1; // expected-error {{__array_rank is unsupported in HLSL}}
//  if (__array_extent(int, 1)) return 1; // expected-error {{__array_extent is unsupported in HLSL}}
//
//  __attribute__((common)) int i; // expected-error {{attribute annotations are unsupported in HLSL}}
//
  return 0;
}

//[availability(*, unavailable, renamed="somethingelse")] void fn_unavailable(); // expected-error {{'availability' is unsupported in HLSL}}
//__declspec(align(16)) struct SAligned { int i; }; // expected-error {{'__declspec' is a reserved keyword in HLSL}}
//void __fastcall fn_fastcall(); // expected-error {{'__fastcall' is a reserved keyword in HLSL}}
//
//// These aren't even recognized as keywords.
//int _pascal fn_pascal();        // expected-error {{expected ';' after top level declarator}}
//int _kernel fn_kernel();        // expected-error {{expected ';' after top level declarator}}
//int __private int__private; // expected-error {{expected ';' after top level declarator}}
//int __global int__global; // expected-error {{expected ';' after top level declarator}}
//int __local int__local; // expected-error {{expected ';' after top level declarator}}
//int __constant int__constant; // expected-error {{expected ';' after top level declarator}}
//int global intglobal; // expected-error {{expected ';' after top level declarator}}
//int local intlocal; // expected-error {{expected ';' after top level declarator}}
//int constant intconstant; // expected-error {{expected ';' after top level declarator}}

typedef void VOID_TYPE;

//template <typename T> // expected-error {{'template' is a reserved keyword in HLSL}}
//int fn_template(T t)
//{
//  return (int)t;
//}

//int template; // expected-error {{'template' is a reserved keyword in HLSL}} expected-error {{expected unqualified-id}}
//
//int get_value(VOID_TYPE) { // expected-error {{empty parameter list defined with a typedef of 'void' not allowed in HLSL}}
//  return 1;
//}
//
//extern "C" int get_value(int) { // expected-error {{expected unqualified-id}}
//  return 2;
//}
//
//void vla(int size) {
//  int n[size]; // expected-error {{variable length arrays are not supported in HLSL}}
//  return n[0];
//}
//
//enum MyEnum  { MyEnum_MyVal1, MyEnum_MyVal2 }; // expected-error {{enum is unsupported in HLSL}} expected-warning {{declaration does not declare anything}}
//enum class MyEnumWithClass { MyEnumWithClass_MyVal1, MyEnumWithClass_MyVal2 }; // expected-error {{enum is unsupported in HLSL}} expected-warning {{declaration does not declare anything}}

float4 fn_with_semantic() : SV_Target0{
  return 0;
}

float4 fn_with_semantic_arg(float4 arg : SV_SOMETHING) : SV_Target0{
  return arg;
}

//int fn_with_try_body()
//try // expected-error {{expected function body after function declarator}}
//{
//  return 1;
//}
//
//// unnamed namespace definition.
//namespace { // expected-error {{expected identifier}}
//  int f_anon_ns_int;
//}

// original namespace definition.
namespace MyNS {
}

//namespace MyNs::Nested { // expected-error {{nested namespace definition must define each namespace separately}}
//}
//
//// original namespace definition with inline.
//inline namespace MyInlineNs { // expected-error {{expected unqualified-id}}
//}

// extension namespace definition.
namespace MyNs {
  int my_ns_extension;
}

// namespace alias definition.
//namespace NamespaceAlias = MyNs; // expected-error {{expected identifier}}
//
//using MyNS; // expected-error {{'using' is a reserved keyword in HLSL}}
//int using; // expected-error {{'using' is a reserved keyword in HLSL}}

// function defined outside namespace
namespace MyNs2
{
  float outsideFunc(int x);
}
float MyNs2::outsideFunc(int x)
{
  return x;
}
float4 main()
{
  return MyNs2::outsideFunc(1);
}

struct my_struct { };

class my_class { };

interface my_interface { };

class my_class_2 : my_class { };

class my_class_3 : my_interface { };

class my_class_4 : my_struct { };

struct my_struct_2 : my_struct { };

struct my_struct_3 : my_class { };

struct my_struct_4 : my_interface { };
struct my_struct_5 : my_class, my_interface { };
//struct my_struct_6 : my_class, my_interface, my_struct { }; // expected-error {{multiple concrete base types specified}}
//
//interface my_interface_2 : my_interface { }; // expected-error {{interfaces cannot inherit from other types}}
//
//class my_class_public : public my_class { }; // expected-error {{base type access specifier is unsupported in HLSL}}

//struct forward_struct; // this fails in fxc, but we allow it now for HLSL version >= 2016
struct my_struct_type_decl { int a; } my_struct_var_decl;
//struct my_struct_type_decl_parens { int a; } (my_struct_var_decl_parens); // expected-error {{expected ';' after struct}} expected-error {{HLSL requires a type specifier for all declarations}}
//struct my_struct_type_const { int a; } const my_struct_type_var; // // expected-error {{expected ';' after struct}} expected-error {{HLSL requires a type specifier for all declarations}}
struct my_struct_type_init { int a; } my_struct_type_init_one = { 1 }, my_struct_type_init_two = { 2 };
//struct my_struct_type_static { int a; } static my_struct_type_static; // expected-error {{expected ';' after struct}} expected-warning {{declaration does not declare anything}}
struct { int my_anon_struct_field; } my_anon_struct_type;

void fn_my_struct_type_decl() {
  my_struct_type_decl local_var;
}

//struct s_with_template_member {
//  template<typename T> T fn(); // expected-error {{'template' is a reserved keyword in HLSL}}
//};
//
//struct s_with_using {
//  using MyNS; // expected-error {{'using' is a reserved keyword in HLSL}}
//};
//
//struct s_with_init {
//  int i = 1; // expected-error {{struct/class members cannot have default values}}
//};

struct s_with_multiple {
  int i, j;
};

//class c_outer {
//  class c_inner { int disallowed; }; // expected-error {{nested class is unsupported in HLSL}}
//};
//
//class c_outer_typedef {
//  typedef int local_int; // expected-error {{nested typedefs are not supported in HLSL}}
//};

class c_outer_fn {
  int fn() {
    class local_class { int j; };
    typedef int local_int;
	return 0;
  }
};

//class c_public {
//public: int i; // expected-error {{'public' is a reserved keyword in HLSL}}
//};

namespace ns_with_struct {
  struct s { int i; };
}

//matrix<int...> g_matrix_pack; // expected-error {{ellipsis is unsupported in HLSL}}
//matrix<ns_with_struct::s> g_matrix_ns; // expected-error {{'ns_with_struct::s' cannot be used as a type parameter where a scalar is required}}
matrix<int, 1, 2> g_matrix_simple;
//matrix<s_with_multiple, 1, 2> g_matrix_user; // expected-error {{'s_with_multiple' cannot be used as a type parameter where a scalar is required}}
//matrix<float2, 1, 1> g_matrix_vector_shorthand; // expected-error {{'float2' cannot be used as a type parameter where a scalar is required}}
//matrix<template matrix<typename, 1, 2> > g_matrix_template_template; // expected-error {{expected expression}} expected-error {{expected unqualified-id}}
//matrix<matrix<typename, 1, 2> > g_matrix_template_template_nokw; // expected-error {{expected a qualified name after 'typename'}} expected-error {{expected a type}} expected-error {{expected a type}}
//matrix<matrix<float, 1, 2> ... > g_matrix_template_template_ellipsis; // expected-error {{ellipsis is unsupported in HLSL}}

#pragma unknown

int global_fn() { return 1; }
//void fn_int_arg(int); // expected-note {{candidate function not viable: no known conversion from 'int2' to 'int' for 1st argument}}

void statements()
{
  int local_i = 1; // expected-note {{declared here}}

//  // attempt to parse a label.
//my_label: local_i = 1; // expected-error {{label is unsupported in HLSL}}
//
//  // attempt to parse an obj-c statement
//  @throw local_i; // expected-error {{expected expression}}
//
  // verify that compound statement blocks have their own scope
  {
    // inner block with scope
    float local_f;
  }
  {
    // inner block with scope
    double local_f;
  }

  // verify that GNU local label (__label__) is disallowed
  //{
  //  __label__ X, Y; // expected-error {{local label is unsupported in HLSL}}
  //}

  // expression statements
  (void)local_i; // disallowed in fxc as the cast is impossible, but allowed for compat now
  //local_i;

  if (local_i == 0) {
    local_i++;
  }

  if (int my_if_local = global_fn()) { // this is an addition to fxc HLSL
    local_i++;
    my_if_local++;
  } else {
    my_if_local--;
  }
  // declaration in 'if' conditional clause does not leak out
  //my_if_local++; // expected-error {{use of undeclared identifier 'my_if_local'}}

  switch (int my_switch_local = global_fn()) {
  case 0: my_switch_local--; return;
  }
  while (int my_while_local = global_fn()) {
    my_while_local--;
  }

  switch(local_i) {
  case 0:
    local_i = 2;
    break;
  case 1 + 2:
    local_i = 3;
    break;
  //case local_i: // expected-error {{expression is not an integral constant expression}} expected-note {{read of non-const variable 'local_i' is not allowed in a constant expression}}
  //  break;
  //case 10 ... 12: // expected-error {{case range is unsupported in HLSL}}
  //  break;
  default:
    local_i = 100;
    // fall through
  }

  while (local_i > 0) {
    local_i -= 1;
    if (local_i == 1) continue;
    if (local_i == 2) break;
  }

  //asm(); // expected-error {{'asm' is a reserved keyword in HLSL}}

  //try { // expected-error {{'try' is a reserved keyword in HLSL}}
  //  local_i = 1;
  //} catch(...) {
  //  local_i = 2;
  //}

  // do/while (without braces)
  do local_i += 1; while (local_i < 3);

  // for, leaking declaration from control part
  for (int i = 0; i < 10; ++i) {
    local_i = i;
    break;
  }
  local_i = i - 1;
  // for, not leaking declaration from body
  for (int val = 0; i < 10; ++i) {
    int val_inner = val;
    break;
  }
  //local_i = val_inner; // expected-error {{use of undeclared identifier 'val_inner'}}
  //// for, redeclaring local
  //int red_same; // expected-note {{previous definition is here}}
  //for (int red_same = 0;;) break; // expected-warning {{redefinition of 'red_same' shadows declaration in the outer scope; most recent declaration will be used}}
  //// for, redeclaring local with different type
  //int red_different; // expected-note {{previous definition is here}}
  //for (float red_different = 0;;) break; // expected-warning {{redefinition of 'red_different' with a different type: 'float' vs 'int' shadows declaration in the outer scope; most recent declaration will be used}}

  //// this proves that the more recent variable is in scope; int2 to int is disallowed
  //int2 red_i_then_int = 0; // expected-note {{previous definition is here}}
  //for (int red_i_then_int = 0;;) break; //expected-warning {{redefinition of 'red_i_then_int' with a different type: 'int' vs 'int2' shadows declaration in the outer scope; most recent declaration will be used}}
  //fn_int_arg(red_i_then_int);
  //fn_int_arg(int2(0,0)); // expected-error {{no matching function for call to 'fn_int_arg'}}

  // for without declaration
  for (local_i = 0; ;) {
    break;
  }
  // for without initialization
  for (int j;;) { break; }
  // ranged for is disallowed
  //for (int n : local_i) { // expected-error {{expected ';' in 'for' statement specifier}} expected-error {{expected ';' in 'for' statement specifier}}
  //  break;
  //}
  //for (int n_again in local_i) { // expected-error {{expected ';' in 'for' statement specifier}} expected-error {{unknown type name 'local_i'}} expected-error {{expected unqualified-id}} expected-error {{variable declaration in condition must have an initializer}}
  //}

  // if/else
  if (local_i == 0)
    local_i = 1;
  else {
    local_i = 2;
  }
  
  // empty/null statements
  ;;;

  //switch (local_i) {
  //default: // expected-error {{label at end of compound statement: expected statement}}
  //}

  //// goto statement
  //goto my_label; // expected-error {{goto is unsupported in HLSL}}

  // return statement
  return;

  // discard statement
  discard;
}

void expressions()
{
  int local_i;
  local_i = 1 > 2 ? 0 : 1;

  // GNU extension to ternary operator, missing second argument to mean first
  //local_i = 1 > 2 ? : 1; // expected-error {{use of GNU ?: conditional expression extension, omitting middle operand is unsupported in HLSL}}

  // initializer lists as right-hand in binary operator (err_init_list_bin_op), but HLSL does not support init lists here
  //local_i = 1 + 2 * { 1 }; // expected-error {{expected expression}}

  local_i = true;
  //local_i = __objc_no;            // expected-error {{'__objc_no' is a reserved keyword in HLSL}}
  //local_i = __objc_yes;           // expected-error {{'__objc_yes' is a reserved keyword in HLSL}}
  //local_i = nullptr;              // expected-error {{use of undeclared identifier 'nullptr'}}
  //local_i = decltype(1)(1);       // expected-error {{use of undeclared identifier 'decltype'}}
  //local_i = __is_integral(bool);  // expected-error {{type trait functions are not supported in HLSL}}
  //local_i = &local_i;             // expected-error {{operator is not supported}}
  local_i = 'c';                    // this is fine
  local_i = '\xFF';                 // this is fine
  local_i = '\x94';                 // this is fine
  //local_i = 'ab';                 // expected-error {{unsupported style of char literal - use a single-character char-based literal}} expected-warning {{multi-character character constant}}
  //local_i = L'a';                 // expected-error {{non-ASCII/multiple-char character constant is unsupported in HLSL}}
  //local_i = L"AB";                // expected-error {{non-ASCII string constant is unsupported in HLSL}}
  //local_i = __FUNCTION__;         // expected-error {{'__FUNCTION__' is a reserved keyword in HLSL}}
  //local_i = _Generic('a',default:0); // expected-error {{'_Generic' is a reserved keyword in HLSL}}
  //// for string literals, see string.hlsl
  //local_i = *local_i;             // expected-error {{operator is not supported}}
  local_i = +local_i;
  local_i = -local_i;
  local_i = ~local_i;
  local_i = !local_i;
  //local_i = __real local_i;       // expected-error {{'__real' is a reserved keyword in HLSL}}
  //local_i = __imag local_i;       // expected-error {{'__imag' is a reserved keyword in HLSL}}
  //local_i = typeid 123;           // expected-error {{'typeid' is a reserved keyword in HLSL}}
  //local_i = __uuidof(local_i);    // expected-error {{use of undeclared identifier '__uuidof'}}
  struct CInternal {
    int i;
    int fn() { return this.i; }
    CInternal getSelf() { return this; }
    //int operator+(int);           // expected-error {{'operator' is a reserved keyword in HLSL}}
  };
  //if (__is_pod(int)) { }          // expected-error {{type trait functions are not supported in HLSL}}
  //^(int x){ return x + 1; };      // expected-error {{compound literal is unsupported in HLSL}} expected-error {{block is unsupported in HLSL}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  CInternal internal;
  //internal->fn();                 // expected-error {{operator is not supported}}
  //local_i = (int3) { 1, 2, 3 };   // expected-error {{compound literal is unsupported in HLSL}}
  
  //Texture2D<::c_outer_fn> local_texture; // expected-error {{'::c_outer_fn' cannot be used as a type parameter}}
  //::new local_new; // expected-error {{new' is a reserved keyword in HLSL}}
  //::template foo local_template; // expected-error {{'template' is a reserved keyword in HLSL}} expected-error {{unknown type name 'foo'}}

  //class CInlineWithTry {
  //  void fn()
  //  try                           // expected-error {{expected function body after function declarator}}
  //  {
  //    int n;
  //  }
  //  catch(...)
  //  {
  //  }

  //  void fn_other()
  //  try                           // expected-error {{expected function body after function declarator}}
  //  {
  //    return;
  //  }

  //  int local_field_1 = 1;        // expected-error {{struct/class members cannot have default values}}
  //  int local_field_2[] = { 1 };  // expected-error {{struct/class members cannot have default values}} expected-error {{array bound cannot be deduced from an in-class initializer}}
  //};
}

// Pragmas.
// TODO: unhandled pragmas should result in a warning
int unused_i;
#pragma unused(unused_i)
#pragma unknown
#pragma GCC visibility push(public)
#pragma pack(push, 1)
#pragma ms_struct(on)
#pragma comment(lib, "kernel32.lib")
#pragma align 64
#pragma weak expressions
#pragma weak expressions = expressions
#pragma redefine_extname g_int new_name_g_int
#pragma STDC FP_CONTRACT on
#pragma OPENCL EXTENSION
#pragma clang __debug captured
#pragma

// Preprocessor directives.
#define A_DEFINE 1
#if (A_DEFINE==1)
#elif (A_DEFINE==2)
#else
#warning it does not work
#endif

// Verified this works but it trips the error processor.
// #error err

#ifdef A_DEFINE
#endif

#undef A_DEFINE
#ifndef A_DEFINE
#else
#error it does not work
#endif

#line 321
//int;
// expected-warning@321 {{declaration does not declare anything}}

float4 plain(float4 param4 /* : FOO */) /*: FOO */{
  //int i[0]; // expected-error {{array dimension must be between 1 and 65536}}
  //const j; // expected-error {{HLSL requires a type specifier for all declarations}}
  //long long ll; // expected-error {{'long' is a reserved keyword in HLSL}} expected-error {{'long' is a reserved keyword in HLSL}} expected-error {{HLSL requires a type specifier for all declarations}}
  return is_supported();
}
