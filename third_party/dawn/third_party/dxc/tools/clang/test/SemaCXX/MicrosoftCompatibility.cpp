// RUN: %clang_cc1 %s -triple i686-pc-win32 -fsyntax-only -std=c++11 -Wmicrosoft -verify -fms-compatibility -fexceptions -fcxx-exceptions -fms-compatibility-version=19.00
// RUN: %clang_cc1 %s -triple i686-pc-win32 -fsyntax-only -std=c++11 -Wmicrosoft -verify -fms-compatibility -fexceptions -fcxx-exceptions -fms-compatibility-version=18.00

#if defined(_HAS_CHAR16_T_LANGUAGE_SUPPORT) && _HAS_CHAR16_T_LANGUAGE_SUPPORT
char16_t x;
char32_t y;
#else
typedef unsigned short char16_t;
typedef unsigned int char32_t;
#endif

#if _MSC_VER >= 1900
_Atomic(int) z;
#else
struct _Atomic {};
#endif

typename decltype(3) a; // expected-warning {{expected a qualified name after 'typename'}}

namespace ms_conversion_rules {

void f(float a);
void f(int a);

void test()
{
    long a = 0;
    f((long)0);
	f(a);
}

}


namespace ms_predefined_types {
  // ::type_info is a built-in forward class declaration.
  void f(const type_info &a);
  void f(size_t);
}


namespace ms_protected_scope {
  struct C { C(); };

  int jump_over_variable_init(bool b) {
    if (b)
      goto foo; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    C c; // expected-note {{jump bypasses variable initialization}}
  foo:
    return 1;
  }

struct Y {
  ~Y();
};

void jump_over_var_with_dtor() {
  goto end; // expected-warning{{jump from this goto statement to its label is a Microsoft extension}}
  Y y; // expected-note {{jump bypasses variable with a non-trivial destructor}}
 end:
    ;
}

  void jump_over_variable_case(int c) {
    switch (c) {
    case 0:
      int x = 56; // expected-note {{jump bypasses variable initialization}}
    case 1:       // expected-error {{cannot jump}}
      x = 10;
    }
  }

 
void exception_jump() {
  goto l2; // expected-error {{cannot jump}}
  try { // expected-note {{jump bypasses initialization of try block}}
     l2: ;
  } catch(int) {
  }
}

int jump_over_indirect_goto() {
  static void *ps[] = { &&a0 };
  goto *&&a0; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
  int a = 3; // expected-note {{jump bypasses variable initialization}}
 a0:
  return 0;
}
  
}

namespace PR11826 {
  struct pair {
    pair(int v) { }
    void operator=(pair&& rhs) { }
  };
  void f() {
    pair p0(3);
    pair p = p0;
  }
}

namespace PR11826_for_symmetry {
  struct pair {
    pair(int v) { }
    pair(pair&& rhs) { }
  };
  void f() {
    pair p0(3);
    pair p(4);
    p = p0;
  }
}

namespace ms_using_declaration_bug {

class A {
public: 
  int f(); 
};

class B : public A {
private:   
  using A::f;
  void g() {
    f(); // no diagnostic
  }
};

class C : public B { 
private:   
  using B::f; // expected-warning {{using declaration referring to inaccessible member 'ms_using_declaration_bug::B::f' (which refers to accessible member 'ms_using_declaration_bug::A::f') is a Microsoft compatibility extension}}
};

}

namespace using_tag_redeclaration
{
  struct S;
  namespace N {
    using ::using_tag_redeclaration::S;
    struct S {}; // expected-note {{previous definition is here}}
  }
  void f() {
    N::S s1;
    S s2;
  }
  void g() {
    struct S; // expected-note {{forward declaration of 'S'}}
    S s3; // expected-error {{variable has incomplete type 'S'}}
  }
  void h() {
    using ::using_tag_redeclaration::S;
    struct S {}; // expected-error {{redefinition of 'S'}}
  }
}


namespace MissingTypename {

template<class T> class A {
public:
	 typedef int TYPE;
};

template<class T> class B {
public:
	 typedef int TYPE;
};


template<class T, class U>
class C : private A<T>, public B<U> {
public:
   typedef A<T> Base1;
   typedef B<U> Base2;
   typedef A<U> Base3;

   A<T>::TYPE a1; // expected-warning {{missing 'typename' prior to dependent type name}}
   Base1::TYPE a2; // expected-warning {{missing 'typename' prior to dependent type name}}

   B<U>::TYPE a3; // expected-warning {{missing 'typename' prior to dependent type name}}
   Base2::TYPE a4; // expected-warning {{missing 'typename' prior to dependent type name}}

   A<U>::TYPE a5; // expected-error {{missing 'typename' prior to dependent type name}}
   Base3::TYPE a6; // expected-error {{missing 'typename' prior to dependent type name}}
 };

class D {
public:
    typedef int Type;
};

template <class T>
void function_missing_typename(const T::Type param)// expected-warning {{missing 'typename' prior to dependent type name}}
{
    const T::Type var = 2; // expected-warning {{missing 'typename' prior to dependent type name}}
}

template void function_missing_typename<D>(const D::Type param);

}

enum ENUM2 {
	ENUM2_a = (enum ENUM2) 4,
	ENUM2_b = 0x9FFFFFFF, // expected-warning {{enumerator value is not representable in the underlying type 'int'}}
	ENUM2_c = 0x100000000 // expected-warning {{enumerator value is not representable in the underlying type 'int'}}
};


namespace PR11791 {
  template<class _Ty>
  void del(_Ty *_Ptr) {
    _Ptr->~_Ty();  // expected-warning {{pseudo-destructors on type void are a Microsoft extension}}
  }

  void f() {
    int* a = 0;
    del((void*)a);  // expected-note {{in instantiation of function template specialization}}
  }
}

namespace IntToNullPtrConv {
  struct Foo {
    static const int ZERO = 0;
    typedef void (Foo::*MemberFcnPtr)();
  };

  struct Bar {
    const Foo::MemberFcnPtr pB;
  };

  Bar g_bar = { (Foo::MemberFcnPtr)Foo::ZERO };

  template<int N> int *get_n() { return N; }   // expected-warning {{expression which evaluates to zero treated as a null pointer constant}}
  int *g_nullptr = get_n<0>();  // expected-note {{in instantiation of function template specialization}}
}
