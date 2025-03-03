// RUN: %clang_cc1 -fsyntax-only -fblocks -Wnullable-to-nonnull-conversion -Wno-nullability-declspec %s -verify

#if __has_feature(nullability)
#else
#  error nullability feature should be defined
#endif

typedef int * int_ptr;

// Parse nullability type specifiers.
typedef int * _Nonnull nonnull_int_ptr; // expected-note{{'_Nonnull' specified here}}
typedef int * _Nullable nullable_int_ptr;
typedef int * _Null_unspecified null_unspecified_int_ptr;

// Redundant nullability type specifiers.
typedef int * _Nonnull _Nonnull redundant_1; // expected-warning{{duplicate nullability specifier '_Nonnull'}}

// Conflicting nullability type specifiers.
typedef int * _Nonnull _Nullable conflicting_1; // expected-error{{nullability specifier '_Nonnull' conflicts with existing specifier '_Nullable'}}
typedef int * _Null_unspecified _Nonnull conflicting_2; // expected-error{{nullability specifier '_Null_unspecified' conflicts with existing specifier '_Nonnull'}}

// Redundant nullability specifiers via a typedef are okay.
typedef nonnull_int_ptr _Nonnull redundant_okay_1;

// Conflicting nullability specifiers via a typedef are not.
typedef nonnull_int_ptr _Nullable conflicting_2; // expected-error{{nullability specifier '_Nullable' conflicts with existing specifier '_Nonnull'}}
typedef nonnull_int_ptr nonnull_int_ptr_typedef;
typedef nonnull_int_ptr_typedef _Nullable conflicting_2; // expected-error{{nullability specifier '_Nullable' conflicts with existing specifier '_Nonnull'}}
typedef nonnull_int_ptr_typedef nonnull_int_ptr_typedef_typedef;
typedef nonnull_int_ptr_typedef_typedef _Null_unspecified conflicting_3; // expected-error{{nullability specifier '_Null_unspecified' conflicts with existing specifier '_Nonnull'}}

// Nullability applies to all pointer types.
typedef int (* _Nonnull function_pointer_type_1)(int, int);
typedef int (^ _Nonnull block_type_1)(int, int);

// Nullability must be on a pointer type.
typedef int _Nonnull int_type_1; // expected-error{{nullability specifier '_Nonnull' cannot be applied to non-pointer type 'int'}}

// Nullability can move out to a pointer/block pointer declarator
// (with a suppressed warning).
typedef _Nonnull int * nonnull_int_ptr_2;
typedef int _Nullable * nullable_int_ptr_2;
typedef _Nonnull int (* function_pointer_type_2)(int, int);
typedef _Nonnull int (^ block_type_2)(int, int);
typedef _Nonnull int * * _Nullable nonnull_int_ptr_ptr_1;
typedef _Nonnull int *(^ block_type_3)(int, int);
typedef _Nonnull int *(* function_pointer_type_3)(int, int);
typedef _Nonnull int_ptr (^ block_type_4)(int, int);
typedef _Nonnull int_ptr (* function_pointer_type_4)(int, int);

void acceptFunctionPtr(_Nonnull int *(*)(void));
void acceptBlockPtr(_Nonnull int *(^)(void));

void testBlockFunctionPtrNullability() {
  float *fp;
  fp = (function_pointer_type_3)0; // expected-warning{{from 'function_pointer_type_3' (aka 'int * _Nonnull (*)(int, int)')}}
  fp = (block_type_3)0; // expected-error{{from incompatible type 'block_type_3' (aka 'int * _Nonnull (^)(int, int)')}}
  fp = (function_pointer_type_4)0; // expected-warning{{from 'function_pointer_type_4' (aka 'int_ptr  _Nonnull (*)(int, int)')}}
  fp = (block_type_4)0; // expected-error{{from incompatible type 'block_type_4' (aka 'int_ptr  _Nonnull (^)(int, int)')}}

  acceptFunctionPtr(0); // no-warning
  acceptBlockPtr(0); // no-warning
}

// Moving nullability where it creates a conflict.
typedef _Nonnull int * _Nullable *  conflict_int_ptr_ptr_2; // expected-error{{nullability specifier '_Nonnull' cannot be applied to non-pointer type 'int'}}

// Nullability is not part of the canonical type.
typedef int * _Nonnull ambiguous_int_ptr;
typedef int * ambiguous_int_ptr;
typedef int * _Nullable ambiguous_int_ptr;

// Printing of nullability.
float f;
int * _Nonnull ip_1 = &f; // expected-warning{{incompatible pointer types initializing 'int * _Nonnull' with an expression of type 'float *'}}

// Check printing of nullability specifiers.
void printing_nullability(void) {
  int * _Nonnull iptr;
  float *fptr = iptr; // expected-warning{{incompatible pointer types initializing 'float *' with an expression of type 'int * _Nonnull'}}

  int * * _Nonnull iptrptr;
  float **fptrptr = iptrptr; // expected-warning{{incompatible pointer types initializing 'float **' with an expression of type 'int ** _Nonnull'}}

  int * _Nullable * _Nonnull iptrptr2;
  float * *fptrptr2 = iptrptr2; // expected-warning{{incompatible pointer types initializing 'float **' with an expression of type 'int * _Nullable * _Nonnull'}}
}

// Check passing null to a _Nonnull argument.
void accepts_nonnull_1(_Nonnull int *ptr);
void (*accepts_nonnull_2)(_Nonnull int *ptr);
void (^accepts_nonnull_3)(_Nonnull int *ptr);

void test_accepts_nonnull_null_pointer_literal() {
  accepts_nonnull_1(0); // expected-warning{{null passed to a callee that requires a non-null argument}}
  accepts_nonnull_2(0); // expected-warning{{null passed to a callee that requires a non-null argument}}
  accepts_nonnull_3(0); // expected-warning{{null passed to a callee that requires a non-null argument}}
}

// Check returning nil from a _Nonnull-returning function.
_Nonnull int *returns_int_ptr(int x) {
  if (x) {
    return 0; // expected-warning{{null returned from function that requires a non-null return value}}
  }

  return (_Nonnull int *)0;
}

// Check nullable-to-nonnull conversions.
void nullable_to_nonnull(_Nullable int *ptr) {
  int *a = ptr; // okay
  _Nonnull int *b = ptr; // expected-warning{{implicit conversion from nullable pointer 'int * _Nullable' to non-nullable pointer type 'int * _Nonnull'}}
}
