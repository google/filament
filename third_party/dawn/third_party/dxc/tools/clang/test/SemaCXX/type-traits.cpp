// RUN: %clang_cc1 -triple x86_64-apple-darwin10 -fsyntax-only -verify -std=gnu++11 -fms-extensions -Wno-microsoft %s
#define T(b) (b) ? 1 : -1
#define F(b) (b) ? -1 : 1

struct NonPOD { NonPOD(int); };

// PODs
enum Enum { EV };
struct POD { Enum e; int i; float f; NonPOD* p; };
struct Empty {};
typedef Empty EmptyAr[10];
typedef int Int;
typedef Int IntAr[10];
typedef Int IntArNB[];
class Statics { static int priv; static NonPOD np; };
union EmptyUnion {};
union Union { int i; float f; };
struct HasFunc { void f (); };
struct HasOp { void operator *(); };
struct HasConv { operator int(); };
struct HasAssign { void operator =(int); };

struct HasAnonymousUnion {
  union {
    int i;
    float f;
  };
};

typedef int Vector __attribute__((vector_size(16)));
typedef int VectorExt __attribute__((ext_vector_type(4)));

// Not PODs
typedef const void cvoid;
struct Derives : POD {};
typedef Derives DerivesAr[10];
typedef Derives DerivesArNB[];
struct DerivesEmpty : Empty {};
struct HasCons { HasCons(int); };
struct HasCopyAssign { HasCopyAssign operator =(const HasCopyAssign&); };
struct HasMoveAssign { HasMoveAssign operator =(const HasMoveAssign&&); };
struct HasNoThrowMoveAssign { 
  HasNoThrowMoveAssign& operator=(
    const HasNoThrowMoveAssign&&) throw(); };
struct HasNoExceptNoThrowMoveAssign { 
  HasNoExceptNoThrowMoveAssign& operator=(
    const HasNoExceptNoThrowMoveAssign&&) noexcept; 
};
struct HasThrowMoveAssign { 
  HasThrowMoveAssign& operator=(
    const HasThrowMoveAssign&&) throw(POD); };
struct HasNoExceptFalseMoveAssign { 
  HasNoExceptFalseMoveAssign& operator=(
    const HasNoExceptFalseMoveAssign&&) noexcept(false); };
struct HasMoveCtor { HasMoveCtor(const HasMoveCtor&&); };
struct HasMemberMoveCtor { HasMoveCtor member; };
struct HasMemberMoveAssign { HasMoveAssign member; };
struct HasStaticMemberMoveCtor { static HasMoveCtor member; };
struct HasStaticMemberMoveAssign { static HasMoveAssign member; };
struct HasMemberThrowMoveAssign { HasThrowMoveAssign member; };
struct HasMemberNoExceptFalseMoveAssign { 
  HasNoExceptFalseMoveAssign member; };
struct HasMemberNoThrowMoveAssign { HasNoThrowMoveAssign member; };
struct HasMemberNoExceptNoThrowMoveAssign { 
  HasNoExceptNoThrowMoveAssign member; };

struct HasDefaultTrivialCopyAssign { 
  HasDefaultTrivialCopyAssign &operator=(
    const HasDefaultTrivialCopyAssign&) = default; 
};
struct TrivialMoveButNotCopy { 
  TrivialMoveButNotCopy &operator=(TrivialMoveButNotCopy&&) = default;
  TrivialMoveButNotCopy &operator=(const TrivialMoveButNotCopy&);
};
struct NonTrivialDefault {
  NonTrivialDefault();
};

struct HasDest { ~HasDest(); };
class  HasPriv { int priv; };
class  HasProt { protected: int prot; };
struct HasRef { int i; int& ref; HasRef() : i(0), ref(i) {} };
struct HasNonPOD { NonPOD np; };
struct HasVirt { virtual void Virt() {}; };
typedef NonPOD NonPODAr[10];
typedef HasVirt VirtAr[10];
typedef NonPOD NonPODArNB[];
union NonPODUnion { int i; Derives n; };
struct DerivesHasCons : HasCons {};
struct DerivesHasCopyAssign : HasCopyAssign {};
struct DerivesHasMoveAssign : HasMoveAssign {};
struct DerivesHasDest : HasDest {};
struct DerivesHasPriv : HasPriv {};
struct DerivesHasProt : HasProt {};
struct DerivesHasRef : HasRef {};
struct DerivesHasVirt : HasVirt {};
struct DerivesHasMoveCtor : HasMoveCtor {};

struct HasNoThrowCopyAssign {
  void operator =(const HasNoThrowCopyAssign&) throw();
};
struct HasMultipleCopyAssign {
  void operator =(const HasMultipleCopyAssign&) throw();
  void operator =(volatile HasMultipleCopyAssign&);
};
struct HasMultipleNoThrowCopyAssign {
  void operator =(const HasMultipleNoThrowCopyAssign&) throw();
  void operator =(volatile HasMultipleNoThrowCopyAssign&) throw();
};

struct HasNoThrowConstructor { HasNoThrowConstructor() throw(); };
struct HasNoThrowConstructorWithArgs {
  HasNoThrowConstructorWithArgs(HasCons i = HasCons(0)) throw();
};
struct HasMultipleDefaultConstructor1 {
  HasMultipleDefaultConstructor1() throw();
  HasMultipleDefaultConstructor1(int i = 0);
};
struct HasMultipleDefaultConstructor2 {
  HasMultipleDefaultConstructor2(int i = 0);
  HasMultipleDefaultConstructor2() throw();
};

struct HasNoThrowCopy { HasNoThrowCopy(const HasNoThrowCopy&) throw(); };
struct HasMultipleCopy {
  HasMultipleCopy(const HasMultipleCopy&) throw();
  HasMultipleCopy(volatile HasMultipleCopy&);
};
struct HasMultipleNoThrowCopy {
  HasMultipleNoThrowCopy(const HasMultipleNoThrowCopy&) throw();
  HasMultipleNoThrowCopy(volatile HasMultipleNoThrowCopy&) throw();
};

struct HasVirtDest { virtual ~HasVirtDest(); };
struct DerivedVirtDest : HasVirtDest {};
typedef HasVirtDest VirtDestAr[1];

class AllPrivate {
  AllPrivate() throw();
  AllPrivate(const AllPrivate&) throw();
  AllPrivate &operator=(const AllPrivate &) throw();
  ~AllPrivate() throw();
};

struct ThreeArgCtor {
  ThreeArgCtor(int*, char*, int);
};

struct VariadicCtor {
  template<typename...T> VariadicCtor(T...);
};

void is_pod()
{
  { int arr[T(__is_pod(int))]; }
  { int arr[T(__is_pod(Enum))]; }
  { int arr[T(__is_pod(POD))]; }
  { int arr[T(__is_pod(Int))]; }
  { int arr[T(__is_pod(IntAr))]; }
  { int arr[T(__is_pod(Statics))]; }
  { int arr[T(__is_pod(Empty))]; }
  { int arr[T(__is_pod(EmptyUnion))]; }
  { int arr[T(__is_pod(Union))]; }
  { int arr[T(__is_pod(HasFunc))]; }
  { int arr[T(__is_pod(HasOp))]; }
  { int arr[T(__is_pod(HasConv))]; }
  { int arr[T(__is_pod(HasAssign))]; }
  { int arr[T(__is_pod(IntArNB))]; }
  { int arr[T(__is_pod(HasAnonymousUnion))]; }
  { int arr[T(__is_pod(Vector))]; }
  { int arr[T(__is_pod(VectorExt))]; }
  { int arr[T(__is_pod(Derives))]; }
  { int arr[T(__is_pod(DerivesAr))]; }
  { int arr[T(__is_pod(DerivesArNB))]; }
  { int arr[T(__is_pod(DerivesEmpty))]; }
  { int arr[T(__is_pod(HasPriv))]; }
  { int arr[T(__is_pod(HasProt))]; }
  { int arr[T(__is_pod(DerivesHasPriv))]; }
  { int arr[T(__is_pod(DerivesHasProt))]; }

  { int arr[F(__is_pod(HasCons))]; }
  { int arr[F(__is_pod(HasCopyAssign))]; }
  { int arr[F(__is_pod(HasMoveAssign))]; }
  { int arr[F(__is_pod(HasDest))]; }
  { int arr[F(__is_pod(HasRef))]; }
  { int arr[F(__is_pod(HasVirt))]; }
  { int arr[F(__is_pod(DerivesHasCons))]; }
  { int arr[F(__is_pod(DerivesHasCopyAssign))]; }
  { int arr[F(__is_pod(DerivesHasMoveAssign))]; }
  { int arr[F(__is_pod(DerivesHasDest))]; }
  { int arr[F(__is_pod(DerivesHasRef))]; }
  { int arr[F(__is_pod(DerivesHasVirt))]; }
  { int arr[F(__is_pod(NonPOD))]; }
  { int arr[F(__is_pod(HasNonPOD))]; }
  { int arr[F(__is_pod(NonPODAr))]; }
  { int arr[F(__is_pod(NonPODArNB))]; }
  { int arr[F(__is_pod(void))]; }
  { int arr[F(__is_pod(cvoid))]; }
// { int arr[F(__is_pod(NonPODUnion))]; }
}

typedef Empty EmptyAr[10];
struct Bit0 { int : 0; };
struct Bit0Cons { int : 0; Bit0Cons(); };
struct BitOnly { int x : 3; };
struct DerivesVirt : virtual POD {};

void is_empty()
{
  { int arr[T(__is_empty(Empty))]; }
  { int arr[T(__is_empty(DerivesEmpty))]; }
  { int arr[T(__is_empty(HasCons))]; }
  { int arr[T(__is_empty(HasCopyAssign))]; }
  { int arr[T(__is_empty(HasMoveAssign))]; }
  { int arr[T(__is_empty(HasDest))]; }
  { int arr[T(__is_empty(HasFunc))]; }
  { int arr[T(__is_empty(HasOp))]; }
  { int arr[T(__is_empty(HasConv))]; }
  { int arr[T(__is_empty(HasAssign))]; }
  { int arr[T(__is_empty(Bit0))]; }
  { int arr[T(__is_empty(Bit0Cons))]; }

  { int arr[F(__is_empty(Int))]; }
  { int arr[F(__is_empty(POD))]; }
  { int arr[F(__is_empty(EmptyUnion))]; }
  { int arr[F(__is_empty(EmptyAr))]; }
  { int arr[F(__is_empty(HasRef))]; }
  { int arr[F(__is_empty(HasVirt))]; }
  { int arr[F(__is_empty(BitOnly))]; }
  { int arr[F(__is_empty(void))]; }
  { int arr[F(__is_empty(IntArNB))]; }
  { int arr[F(__is_empty(HasAnonymousUnion))]; }
//  { int arr[F(__is_empty(DerivesVirt))]; }
}

typedef Derives ClassType;

void is_class()
{
  { int arr[T(__is_class(Derives))]; }
  { int arr[T(__is_class(HasPriv))]; }
  { int arr[T(__is_class(ClassType))]; }
  { int arr[T(__is_class(HasAnonymousUnion))]; }

  { int arr[F(__is_class(int))]; }
  { int arr[F(__is_class(Enum))]; }
  { int arr[F(__is_class(Int))]; }
  { int arr[F(__is_class(IntAr))]; }
  { int arr[F(__is_class(DerivesAr))]; }
  { int arr[F(__is_class(Union))]; }
  { int arr[F(__is_class(cvoid))]; }
  { int arr[F(__is_class(IntArNB))]; }
}

typedef Union UnionAr[10];
typedef Union UnionType;

void is_union()
{
  { int arr[T(__is_union(Union))]; }
  { int arr[T(__is_union(UnionType))]; }

  { int arr[F(__is_union(int))]; }
  { int arr[F(__is_union(Enum))]; }
  { int arr[F(__is_union(Int))]; }
  { int arr[F(__is_union(IntAr))]; }
  { int arr[F(__is_union(UnionAr))]; }
  { int arr[F(__is_union(cvoid))]; }
  { int arr[F(__is_union(IntArNB))]; }
  { int arr[F(__is_union(HasAnonymousUnion))]; }
}

typedef Enum EnumType;

void is_enum()
{
  { int arr[T(__is_enum(Enum))]; }
  { int arr[T(__is_enum(EnumType))]; }

  { int arr[F(__is_enum(int))]; }
  { int arr[F(__is_enum(Union))]; }
  { int arr[F(__is_enum(Int))]; }
  { int arr[F(__is_enum(IntAr))]; }
  { int arr[F(__is_enum(UnionAr))]; }
  { int arr[F(__is_enum(Derives))]; }
  { int arr[F(__is_enum(ClassType))]; }
  { int arr[F(__is_enum(cvoid))]; }
  { int arr[F(__is_enum(IntArNB))]; }
  { int arr[F(__is_enum(HasAnonymousUnion))]; }
}

struct FinalClass final {
};

template<typename T> 
struct PotentiallyFinal { };

template<typename T>
struct PotentiallyFinal<T*> final { };

template<>
struct PotentiallyFinal<int> final { };

void is_final()
{
	{ int arr[T(__is_final(FinalClass))]; }
	{ int arr[T(__is_final(PotentiallyFinal<float*>))]; }
	{ int arr[T(__is_final(PotentiallyFinal<int>))]; }

	{ int arr[F(__is_final(int))]; }
	{ int arr[F(__is_final(Union))]; }
	{ int arr[F(__is_final(Int))]; }
	{ int arr[F(__is_final(IntAr))]; }
	{ int arr[F(__is_final(UnionAr))]; }
	{ int arr[F(__is_final(Derives))]; }
	{ int arr[F(__is_final(ClassType))]; }
	{ int arr[F(__is_final(cvoid))]; }
	{ int arr[F(__is_final(IntArNB))]; }
	{ int arr[F(__is_final(HasAnonymousUnion))]; }
	{ int arr[F(__is_final(PotentiallyFinal<float>))]; }
}

struct SealedClass sealed {
};

template<typename T>
struct PotentiallySealed { };

template<typename T>
struct PotentiallySealed<T*> sealed { };

template<>
struct PotentiallySealed<int> sealed { };

void is_sealed()
{
	{ int arr[T(__is_sealed(SealedClass))]; }
	{ int arr[T(__is_sealed(PotentiallySealed<float*>))]; }
	{ int arr[T(__is_sealed(PotentiallySealed<int>))]; }

	{ int arr[F(__is_sealed(int))]; }
	{ int arr[F(__is_sealed(Union))]; }
	{ int arr[F(__is_sealed(Int))]; }
	{ int arr[F(__is_sealed(IntAr))]; }
	{ int arr[F(__is_sealed(UnionAr))]; }
	{ int arr[F(__is_sealed(Derives))]; }
	{ int arr[F(__is_sealed(ClassType))]; }
	{ int arr[F(__is_sealed(cvoid))]; }
	{ int arr[F(__is_sealed(IntArNB))]; }
	{ int arr[F(__is_sealed(HasAnonymousUnion))]; }
	{ int arr[F(__is_sealed(PotentiallyFinal<float>))]; }
}

typedef HasVirt Polymorph;
struct InheritPolymorph : Polymorph {};

void is_polymorphic()
{
  { int arr[T(__is_polymorphic(Polymorph))]; }
  { int arr[T(__is_polymorphic(InheritPolymorph))]; }

  { int arr[F(__is_polymorphic(int))]; }
  { int arr[F(__is_polymorphic(Union))]; }
  { int arr[F(__is_polymorphic(Int))]; }
  { int arr[F(__is_polymorphic(IntAr))]; }
  { int arr[F(__is_polymorphic(UnionAr))]; }
  { int arr[F(__is_polymorphic(Derives))]; }
  { int arr[F(__is_polymorphic(ClassType))]; }
  { int arr[F(__is_polymorphic(Enum))]; }
  { int arr[F(__is_polymorphic(cvoid))]; }
  { int arr[F(__is_polymorphic(IntArNB))]; }
}

void is_integral()
{
  int t01[T(__is_integral(bool))];
  int t02[T(__is_integral(char))];
  int t03[T(__is_integral(signed char))];
  int t04[T(__is_integral(unsigned char))];
  //int t05[T(__is_integral(char16_t))];
  //int t06[T(__is_integral(char32_t))];
  int t07[T(__is_integral(wchar_t))];
  int t08[T(__is_integral(short))];
  int t09[T(__is_integral(unsigned short))];
  int t10[T(__is_integral(int))];
  int t11[T(__is_integral(unsigned int))];
  int t12[T(__is_integral(long))];
  int t13[T(__is_integral(unsigned long))];

  int t21[F(__is_integral(float))];
  int t22[F(__is_integral(double))];
  int t23[F(__is_integral(long double))];
  int t24[F(__is_integral(Union))];
  int t25[F(__is_integral(UnionAr))];
  int t26[F(__is_integral(Derives))];
  int t27[F(__is_integral(ClassType))];
  int t28[F(__is_integral(Enum))];
  int t29[F(__is_integral(void))];
  int t30[F(__is_integral(cvoid))];
  int t31[F(__is_integral(IntArNB))];
}

void is_floating_point()
{
  int t01[T(__is_floating_point(float))];
  int t02[T(__is_floating_point(double))];
  int t03[T(__is_floating_point(long double))];

  int t11[F(__is_floating_point(bool))];
  int t12[F(__is_floating_point(char))];
  int t13[F(__is_floating_point(signed char))];
  int t14[F(__is_floating_point(unsigned char))];
  //int t15[F(__is_floating_point(char16_t))];
  //int t16[F(__is_floating_point(char32_t))];
  int t17[F(__is_floating_point(wchar_t))];
  int t18[F(__is_floating_point(short))];
  int t19[F(__is_floating_point(unsigned short))];
  int t20[F(__is_floating_point(int))];
  int t21[F(__is_floating_point(unsigned int))];
  int t22[F(__is_floating_point(long))];
  int t23[F(__is_floating_point(unsigned long))];
  int t24[F(__is_floating_point(Union))];
  int t25[F(__is_floating_point(UnionAr))];
  int t26[F(__is_floating_point(Derives))];
  int t27[F(__is_floating_point(ClassType))];
  int t28[F(__is_floating_point(Enum))];
  int t29[F(__is_floating_point(void))];
  int t30[F(__is_floating_point(cvoid))];
  int t31[F(__is_floating_point(IntArNB))];
}

void is_arithmetic()
{
  int t01[T(__is_arithmetic(float))];
  int t02[T(__is_arithmetic(double))];
  int t03[T(__is_arithmetic(long double))];
  int t11[T(__is_arithmetic(bool))];
  int t12[T(__is_arithmetic(char))];
  int t13[T(__is_arithmetic(signed char))];
  int t14[T(__is_arithmetic(unsigned char))];
  //int t15[T(__is_arithmetic(char16_t))];
  //int t16[T(__is_arithmetic(char32_t))];
  int t17[T(__is_arithmetic(wchar_t))];
  int t18[T(__is_arithmetic(short))];
  int t19[T(__is_arithmetic(unsigned short))];
  int t20[T(__is_arithmetic(int))];
  int t21[T(__is_arithmetic(unsigned int))];
  int t22[T(__is_arithmetic(long))];
  int t23[T(__is_arithmetic(unsigned long))];

  int t24[F(__is_arithmetic(Union))];
  int t25[F(__is_arithmetic(UnionAr))];
  int t26[F(__is_arithmetic(Derives))];
  int t27[F(__is_arithmetic(ClassType))];
  int t28[F(__is_arithmetic(Enum))];
  int t29[F(__is_arithmetic(void))];
  int t30[F(__is_arithmetic(cvoid))];
  int t31[F(__is_arithmetic(IntArNB))];
}

struct ACompleteType {};
struct AnIncompleteType;

void is_complete_type()
{
  int t01[T(__is_complete_type(float))];
  int t02[T(__is_complete_type(double))];
  int t03[T(__is_complete_type(long double))];
  int t11[T(__is_complete_type(bool))];
  int t12[T(__is_complete_type(char))];
  int t13[T(__is_complete_type(signed char))];
  int t14[T(__is_complete_type(unsigned char))];
  //int t15[T(__is_complete_type(char16_t))];
  //int t16[T(__is_complete_type(char32_t))];
  int t17[T(__is_complete_type(wchar_t))];
  int t18[T(__is_complete_type(short))];
  int t19[T(__is_complete_type(unsigned short))];
  int t20[T(__is_complete_type(int))];
  int t21[T(__is_complete_type(unsigned int))];
  int t22[T(__is_complete_type(long))];
  int t23[T(__is_complete_type(unsigned long))];
  int t24[T(__is_complete_type(ACompleteType))];

  int t30[F(__is_complete_type(AnIncompleteType))];
}

void is_void()
{
  int t01[T(__is_void(void))];
  int t02[T(__is_void(cvoid))];

  int t10[F(__is_void(float))];
  int t11[F(__is_void(double))];
  int t12[F(__is_void(long double))];
  int t13[F(__is_void(bool))];
  int t14[F(__is_void(char))];
  int t15[F(__is_void(signed char))];
  int t16[F(__is_void(unsigned char))];
  int t17[F(__is_void(wchar_t))];
  int t18[F(__is_void(short))];
  int t19[F(__is_void(unsigned short))];
  int t20[F(__is_void(int))];
  int t21[F(__is_void(unsigned int))];
  int t22[F(__is_void(long))];
  int t23[F(__is_void(unsigned long))];
  int t24[F(__is_void(Union))];
  int t25[F(__is_void(UnionAr))];
  int t26[F(__is_void(Derives))];
  int t27[F(__is_void(ClassType))];
  int t28[F(__is_void(Enum))];
  int t29[F(__is_void(IntArNB))];
  int t30[F(__is_void(void*))];
  int t31[F(__is_void(cvoid*))];
}

void is_array()
{
  int t01[T(__is_array(IntAr))];
  int t02[T(__is_array(IntArNB))];
  int t03[T(__is_array(UnionAr))];

  int t10[F(__is_array(void))];
  int t11[F(__is_array(cvoid))];
  int t12[F(__is_array(float))];
  int t13[F(__is_array(double))];
  int t14[F(__is_array(long double))];
  int t15[F(__is_array(bool))];
  int t16[F(__is_array(char))];
  int t17[F(__is_array(signed char))];
  int t18[F(__is_array(unsigned char))];
  int t19[F(__is_array(wchar_t))];
  int t20[F(__is_array(short))];
  int t21[F(__is_array(unsigned short))];
  int t22[F(__is_array(int))];
  int t23[F(__is_array(unsigned int))];
  int t24[F(__is_array(long))];
  int t25[F(__is_array(unsigned long))];
  int t26[F(__is_array(Union))];
  int t27[F(__is_array(Derives))];
  int t28[F(__is_array(ClassType))];
  int t29[F(__is_array(Enum))];
  int t30[F(__is_array(void*))];
  int t31[F(__is_array(cvoid*))];
}

template <typename T> void tmpl_func(T&) {}

template <typename T> struct type_wrapper {
  typedef T type;
  typedef T* ptrtype;
  typedef T& reftype;
};

void is_function()
{
  int t01[T(__is_function(type_wrapper<void(void)>::type))];
  int t02[T(__is_function(typeof(tmpl_func<int>)))];

  typedef void (*ptr_to_func_type)(void);

  int t10[F(__is_function(void))];
  int t11[F(__is_function(cvoid))];
  int t12[F(__is_function(float))];
  int t13[F(__is_function(double))];
  int t14[F(__is_function(long double))];
  int t15[F(__is_function(bool))];
  int t16[F(__is_function(char))];
  int t17[F(__is_function(signed char))];
  int t18[F(__is_function(unsigned char))];
  int t19[F(__is_function(wchar_t))];
  int t20[F(__is_function(short))];
  int t21[F(__is_function(unsigned short))];
  int t22[F(__is_function(int))];
  int t23[F(__is_function(unsigned int))];
  int t24[F(__is_function(long))];
  int t25[F(__is_function(unsigned long))];
  int t26[F(__is_function(Union))];
  int t27[F(__is_function(Derives))];
  int t28[F(__is_function(ClassType))];
  int t29[F(__is_function(Enum))];
  int t30[F(__is_function(void*))];
  int t31[F(__is_function(cvoid*))];
  int t32[F(__is_function(void(*)()))];
  int t33[F(__is_function(ptr_to_func_type))];
  int t34[F(__is_function(type_wrapper<void(void)>::ptrtype))];
  int t35[F(__is_function(type_wrapper<void(void)>::reftype))];
}

void is_reference()
{
  int t01[T(__is_reference(int&))];
  int t02[T(__is_reference(const int&))];
  int t03[T(__is_reference(void *&))];

  int t10[F(__is_reference(int))];
  int t11[F(__is_reference(const int))];
  int t12[F(__is_reference(void *))];
}

void is_lvalue_reference()
{
  int t01[T(__is_lvalue_reference(int&))];
  int t02[T(__is_lvalue_reference(void *&))];
  int t03[T(__is_lvalue_reference(const int&))];
  int t04[T(__is_lvalue_reference(void * const &))];

  int t10[F(__is_lvalue_reference(int))];
  int t11[F(__is_lvalue_reference(const int))];
  int t12[F(__is_lvalue_reference(void *))];
}

#if __has_feature(cxx_rvalue_references)

void is_rvalue_reference()
{
  int t01[T(__is_rvalue_reference(const int&&))];
  int t02[T(__is_rvalue_reference(void * const &&))];

  int t10[F(__is_rvalue_reference(int&))];
  int t11[F(__is_rvalue_reference(void *&))];
  int t12[F(__is_rvalue_reference(const int&))];
  int t13[F(__is_rvalue_reference(void * const &))];
  int t14[F(__is_rvalue_reference(int))];
  int t15[F(__is_rvalue_reference(const int))];
  int t16[F(__is_rvalue_reference(void *))];
}

#endif

void is_fundamental()
{
  int t01[T(__is_fundamental(float))];
  int t02[T(__is_fundamental(double))];
  int t03[T(__is_fundamental(long double))];
  int t11[T(__is_fundamental(bool))];
  int t12[T(__is_fundamental(char))];
  int t13[T(__is_fundamental(signed char))];
  int t14[T(__is_fundamental(unsigned char))];
  //int t15[T(__is_fundamental(char16_t))];
  //int t16[T(__is_fundamental(char32_t))];
  int t17[T(__is_fundamental(wchar_t))];
  int t18[T(__is_fundamental(short))];
  int t19[T(__is_fundamental(unsigned short))];
  int t20[T(__is_fundamental(int))];
  int t21[T(__is_fundamental(unsigned int))];
  int t22[T(__is_fundamental(long))];
  int t23[T(__is_fundamental(unsigned long))];
  int t24[T(__is_fundamental(void))];
  int t25[T(__is_fundamental(cvoid))];

  int t30[F(__is_fundamental(Union))];
  int t31[F(__is_fundamental(UnionAr))];
  int t32[F(__is_fundamental(Derives))];
  int t33[F(__is_fundamental(ClassType))];
  int t34[F(__is_fundamental(Enum))];
  int t35[F(__is_fundamental(IntArNB))];
}

void is_object()
{
  int t01[T(__is_object(int))];
  int t02[T(__is_object(int *))];
  int t03[T(__is_object(void *))];
  int t04[T(__is_object(Union))];
  int t05[T(__is_object(UnionAr))];
  int t06[T(__is_object(ClassType))];
  int t07[T(__is_object(Enum))];

  int t10[F(__is_object(type_wrapper<void(void)>::type))];
  int t11[F(__is_object(int&))];
  int t12[F(__is_object(void))];
}

void is_scalar()
{
  int t01[T(__is_scalar(float))];
  int t02[T(__is_scalar(double))];
  int t03[T(__is_scalar(long double))];
  int t04[T(__is_scalar(bool))];
  int t05[T(__is_scalar(char))];
  int t06[T(__is_scalar(signed char))];
  int t07[T(__is_scalar(unsigned char))];
  int t08[T(__is_scalar(wchar_t))];
  int t09[T(__is_scalar(short))];
  int t10[T(__is_scalar(unsigned short))];
  int t11[T(__is_scalar(int))];
  int t12[T(__is_scalar(unsigned int))];
  int t13[T(__is_scalar(long))];
  int t14[T(__is_scalar(unsigned long))];
  int t15[T(__is_scalar(Enum))];
  int t16[T(__is_scalar(void*))];
  int t17[T(__is_scalar(cvoid*))];

  int t20[F(__is_scalar(void))];
  int t21[F(__is_scalar(cvoid))];
  int t22[F(__is_scalar(Union))];
  int t23[F(__is_scalar(UnionAr))];
  int t24[F(__is_scalar(Derives))];
  int t25[F(__is_scalar(ClassType))];
  int t26[F(__is_scalar(IntArNB))];
}

struct StructWithMembers {
  int member;
  void method() {}
};

void is_compound()
{
  int t01[T(__is_compound(void*))];
  int t02[T(__is_compound(cvoid*))];
  int t03[T(__is_compound(void (*)()))];
  int t04[T(__is_compound(int StructWithMembers::*))];
  int t05[T(__is_compound(void (StructWithMembers::*)()))];
  int t06[T(__is_compound(int&))];
  int t07[T(__is_compound(Union))];
  int t08[T(__is_compound(UnionAr))];
  int t09[T(__is_compound(Derives))];
  int t10[T(__is_compound(ClassType))];
  int t11[T(__is_compound(IntArNB))];
  int t12[T(__is_compound(Enum))];

  int t20[F(__is_compound(float))];
  int t21[F(__is_compound(double))];
  int t22[F(__is_compound(long double))];
  int t23[F(__is_compound(bool))];
  int t24[F(__is_compound(char))];
  int t25[F(__is_compound(signed char))];
  int t26[F(__is_compound(unsigned char))];
  int t27[F(__is_compound(wchar_t))];
  int t28[F(__is_compound(short))];
  int t29[F(__is_compound(unsigned short))];
  int t30[F(__is_compound(int))];
  int t31[F(__is_compound(unsigned int))];
  int t32[F(__is_compound(long))];
  int t33[F(__is_compound(unsigned long))];
  int t34[F(__is_compound(void))];
  int t35[F(__is_compound(cvoid))];
}

void is_pointer()
{
  StructWithMembers x;

  int t01[T(__is_pointer(void*))];
  int t02[T(__is_pointer(cvoid*))];
  int t03[T(__is_pointer(cvoid*))];
  int t04[T(__is_pointer(char*))];
  int t05[T(__is_pointer(int*))];
  int t06[T(__is_pointer(int**))];
  int t07[T(__is_pointer(ClassType*))];
  int t08[T(__is_pointer(Derives*))];
  int t09[T(__is_pointer(Enum*))];
  int t10[T(__is_pointer(IntArNB*))];
  int t11[T(__is_pointer(Union*))];
  int t12[T(__is_pointer(UnionAr*))];
  int t13[T(__is_pointer(StructWithMembers*))];
  int t14[T(__is_pointer(void (*)()))];

  int t20[F(__is_pointer(void))];
  int t21[F(__is_pointer(cvoid))];
  int t22[F(__is_pointer(cvoid))];
  int t23[F(__is_pointer(char))];
  int t24[F(__is_pointer(int))];
  int t25[F(__is_pointer(int))];
  int t26[F(__is_pointer(ClassType))];
  int t27[F(__is_pointer(Derives))];
  int t28[F(__is_pointer(Enum))];
  int t29[F(__is_pointer(IntArNB))];
  int t30[F(__is_pointer(Union))];
  int t31[F(__is_pointer(UnionAr))];
  int t32[F(__is_pointer(StructWithMembers))];
  int t33[F(__is_pointer(int StructWithMembers::*))];
  int t34[F(__is_pointer(void (StructWithMembers::*) ()))];
}

void is_member_object_pointer()
{
  StructWithMembers x;

  int t01[T(__is_member_object_pointer(int StructWithMembers::*))];

  int t10[F(__is_member_object_pointer(void (StructWithMembers::*) ()))];
  int t11[F(__is_member_object_pointer(void*))];
  int t12[F(__is_member_object_pointer(cvoid*))];
  int t13[F(__is_member_object_pointer(cvoid*))];
  int t14[F(__is_member_object_pointer(char*))];
  int t15[F(__is_member_object_pointer(int*))];
  int t16[F(__is_member_object_pointer(int**))];
  int t17[F(__is_member_object_pointer(ClassType*))];
  int t18[F(__is_member_object_pointer(Derives*))];
  int t19[F(__is_member_object_pointer(Enum*))];
  int t20[F(__is_member_object_pointer(IntArNB*))];
  int t21[F(__is_member_object_pointer(Union*))];
  int t22[F(__is_member_object_pointer(UnionAr*))];
  int t23[F(__is_member_object_pointer(StructWithMembers*))];
  int t24[F(__is_member_object_pointer(void))];
  int t25[F(__is_member_object_pointer(cvoid))];
  int t26[F(__is_member_object_pointer(cvoid))];
  int t27[F(__is_member_object_pointer(char))];
  int t28[F(__is_member_object_pointer(int))];
  int t29[F(__is_member_object_pointer(int))];
  int t30[F(__is_member_object_pointer(ClassType))];
  int t31[F(__is_member_object_pointer(Derives))];
  int t32[F(__is_member_object_pointer(Enum))];
  int t33[F(__is_member_object_pointer(IntArNB))];
  int t34[F(__is_member_object_pointer(Union))];
  int t35[F(__is_member_object_pointer(UnionAr))];
  int t36[F(__is_member_object_pointer(StructWithMembers))];
  int t37[F(__is_member_object_pointer(void (*)()))];
}

void is_member_function_pointer()
{
  StructWithMembers x;

  int t01[T(__is_member_function_pointer(void (StructWithMembers::*) ()))];

  int t10[F(__is_member_function_pointer(int StructWithMembers::*))];
  int t11[F(__is_member_function_pointer(void*))];
  int t12[F(__is_member_function_pointer(cvoid*))];
  int t13[F(__is_member_function_pointer(cvoid*))];
  int t14[F(__is_member_function_pointer(char*))];
  int t15[F(__is_member_function_pointer(int*))];
  int t16[F(__is_member_function_pointer(int**))];
  int t17[F(__is_member_function_pointer(ClassType*))];
  int t18[F(__is_member_function_pointer(Derives*))];
  int t19[F(__is_member_function_pointer(Enum*))];
  int t20[F(__is_member_function_pointer(IntArNB*))];
  int t21[F(__is_member_function_pointer(Union*))];
  int t22[F(__is_member_function_pointer(UnionAr*))];
  int t23[F(__is_member_function_pointer(StructWithMembers*))];
  int t24[F(__is_member_function_pointer(void))];
  int t25[F(__is_member_function_pointer(cvoid))];
  int t26[F(__is_member_function_pointer(cvoid))];
  int t27[F(__is_member_function_pointer(char))];
  int t28[F(__is_member_function_pointer(int))];
  int t29[F(__is_member_function_pointer(int))];
  int t30[F(__is_member_function_pointer(ClassType))];
  int t31[F(__is_member_function_pointer(Derives))];
  int t32[F(__is_member_function_pointer(Enum))];
  int t33[F(__is_member_function_pointer(IntArNB))];
  int t34[F(__is_member_function_pointer(Union))];
  int t35[F(__is_member_function_pointer(UnionAr))];
  int t36[F(__is_member_function_pointer(StructWithMembers))];
  int t37[F(__is_member_function_pointer(void (*)()))];
}

void is_member_pointer()
{
  StructWithMembers x;

  int t01[T(__is_member_pointer(int StructWithMembers::*))];
  int t02[T(__is_member_pointer(void (StructWithMembers::*) ()))];

  int t10[F(__is_member_pointer(void*))];
  int t11[F(__is_member_pointer(cvoid*))];
  int t12[F(__is_member_pointer(cvoid*))];
  int t13[F(__is_member_pointer(char*))];
  int t14[F(__is_member_pointer(int*))];
  int t15[F(__is_member_pointer(int**))];
  int t16[F(__is_member_pointer(ClassType*))];
  int t17[F(__is_member_pointer(Derives*))];
  int t18[F(__is_member_pointer(Enum*))];
  int t19[F(__is_member_pointer(IntArNB*))];
  int t20[F(__is_member_pointer(Union*))];
  int t21[F(__is_member_pointer(UnionAr*))];
  int t22[F(__is_member_pointer(StructWithMembers*))];
  int t23[F(__is_member_pointer(void))];
  int t24[F(__is_member_pointer(cvoid))];
  int t25[F(__is_member_pointer(cvoid))];
  int t26[F(__is_member_pointer(char))];
  int t27[F(__is_member_pointer(int))];
  int t28[F(__is_member_pointer(int))];
  int t29[F(__is_member_pointer(ClassType))];
  int t30[F(__is_member_pointer(Derives))];
  int t31[F(__is_member_pointer(Enum))];
  int t32[F(__is_member_pointer(IntArNB))];
  int t33[F(__is_member_pointer(Union))];
  int t34[F(__is_member_pointer(UnionAr))];
  int t35[F(__is_member_pointer(StructWithMembers))];
  int t36[F(__is_member_pointer(void (*)()))];
}

void is_const()
{
  int t01[T(__is_const(cvoid))];
  int t02[T(__is_const(const char))];
  int t03[T(__is_const(const int))];
  int t04[T(__is_const(const long))];
  int t05[T(__is_const(const short))];
  int t06[T(__is_const(const signed char))];
  int t07[T(__is_const(const wchar_t))];
  int t08[T(__is_const(const bool))];
  int t09[T(__is_const(const float))];
  int t10[T(__is_const(const double))];
  int t11[T(__is_const(const long double))];
  int t12[T(__is_const(const unsigned char))];
  int t13[T(__is_const(const unsigned int))];
  int t14[T(__is_const(const unsigned long long))];
  int t15[T(__is_const(const unsigned long))];
  int t16[T(__is_const(const unsigned short))];
  int t17[T(__is_const(const void))];
  int t18[T(__is_const(const ClassType))];
  int t19[T(__is_const(const Derives))];
  int t20[T(__is_const(const Enum))];
  int t21[T(__is_const(const IntArNB))];
  int t22[T(__is_const(const Union))];
  int t23[T(__is_const(const UnionAr))];

  int t30[F(__is_const(char))];
  int t31[F(__is_const(int))];
  int t32[F(__is_const(long))];
  int t33[F(__is_const(short))];
  int t34[F(__is_const(signed char))];
  int t35[F(__is_const(wchar_t))];
  int t36[F(__is_const(bool))];
  int t37[F(__is_const(float))];
  int t38[F(__is_const(double))];
  int t39[F(__is_const(long double))];
  int t40[F(__is_const(unsigned char))];
  int t41[F(__is_const(unsigned int))];
  int t42[F(__is_const(unsigned long long))];
  int t43[F(__is_const(unsigned long))];
  int t44[F(__is_const(unsigned short))];
  int t45[F(__is_const(void))];
  int t46[F(__is_const(ClassType))];
  int t47[F(__is_const(Derives))];
  int t48[F(__is_const(Enum))];
  int t49[F(__is_const(IntArNB))];
  int t50[F(__is_const(Union))];
  int t51[F(__is_const(UnionAr))];
}

void is_volatile()
{
  int t02[T(__is_volatile(volatile char))];
  int t03[T(__is_volatile(volatile int))];
  int t04[T(__is_volatile(volatile long))];
  int t05[T(__is_volatile(volatile short))];
  int t06[T(__is_volatile(volatile signed char))];
  int t07[T(__is_volatile(volatile wchar_t))];
  int t08[T(__is_volatile(volatile bool))];
  int t09[T(__is_volatile(volatile float))];
  int t10[T(__is_volatile(volatile double))];
  int t11[T(__is_volatile(volatile long double))];
  int t12[T(__is_volatile(volatile unsigned char))];
  int t13[T(__is_volatile(volatile unsigned int))];
  int t14[T(__is_volatile(volatile unsigned long long))];
  int t15[T(__is_volatile(volatile unsigned long))];
  int t16[T(__is_volatile(volatile unsigned short))];
  int t17[T(__is_volatile(volatile void))];
  int t18[T(__is_volatile(volatile ClassType))];
  int t19[T(__is_volatile(volatile Derives))];
  int t20[T(__is_volatile(volatile Enum))];
  int t21[T(__is_volatile(volatile IntArNB))];
  int t22[T(__is_volatile(volatile Union))];
  int t23[T(__is_volatile(volatile UnionAr))];

  int t30[F(__is_volatile(char))];
  int t31[F(__is_volatile(int))];
  int t32[F(__is_volatile(long))];
  int t33[F(__is_volatile(short))];
  int t34[F(__is_volatile(signed char))];
  int t35[F(__is_volatile(wchar_t))];
  int t36[F(__is_volatile(bool))];
  int t37[F(__is_volatile(float))];
  int t38[F(__is_volatile(double))];
  int t39[F(__is_volatile(long double))];
  int t40[F(__is_volatile(unsigned char))];
  int t41[F(__is_volatile(unsigned int))];
  int t42[F(__is_volatile(unsigned long long))];
  int t43[F(__is_volatile(unsigned long))];
  int t44[F(__is_volatile(unsigned short))];
  int t45[F(__is_volatile(void))];
  int t46[F(__is_volatile(ClassType))];
  int t47[F(__is_volatile(Derives))];
  int t48[F(__is_volatile(Enum))];
  int t49[F(__is_volatile(IntArNB))];
  int t50[F(__is_volatile(Union))];
  int t51[F(__is_volatile(UnionAr))];
}

struct TrivialStruct {
  int member;
};

struct NonTrivialStruct {
  int member;
  NonTrivialStruct() {
    member = 0;
  }
};

struct SuperNonTrivialStruct {
  SuperNonTrivialStruct() { }
  ~SuperNonTrivialStruct() { }
};

struct NonTCStruct {
  NonTCStruct(const NonTCStruct&) {}
};

struct AllDefaulted {
  AllDefaulted() = default;
  AllDefaulted(const AllDefaulted &) = default;
  AllDefaulted(AllDefaulted &&) = default;
  AllDefaulted &operator=(const AllDefaulted &) = default;
  AllDefaulted &operator=(AllDefaulted &&) = default;
  ~AllDefaulted() = default;
};

struct NoDefaultMoveAssignDueToUDCopyCtor {
  NoDefaultMoveAssignDueToUDCopyCtor(const NoDefaultMoveAssignDueToUDCopyCtor&);
};

struct NoDefaultMoveAssignDueToUDCopyAssign {
  NoDefaultMoveAssignDueToUDCopyAssign& operator=(
    const NoDefaultMoveAssignDueToUDCopyAssign&);
};

struct NoDefaultMoveAssignDueToDtor {
  ~NoDefaultMoveAssignDueToDtor();
};

struct AllDeleted {
  AllDeleted() = delete;
  AllDeleted(const AllDeleted &) = delete;
  AllDeleted(AllDeleted &&) = delete;
  AllDeleted &operator=(const AllDeleted &) = delete;
  AllDeleted &operator=(AllDeleted &&) = delete;
  ~AllDeleted() = delete;
};

struct ExtDefaulted {
  ExtDefaulted();
  ExtDefaulted(const ExtDefaulted &);
  ExtDefaulted(ExtDefaulted &&);
  ExtDefaulted &operator=(const ExtDefaulted &);
  ExtDefaulted &operator=(ExtDefaulted &&);
  ~ExtDefaulted();
};

// Despite being defaulted, these functions are not trivial.
ExtDefaulted::ExtDefaulted() = default;
ExtDefaulted::ExtDefaulted(const ExtDefaulted &) = default;
ExtDefaulted::ExtDefaulted(ExtDefaulted &&) = default;
ExtDefaulted &ExtDefaulted::operator=(const ExtDefaulted &) = default;
ExtDefaulted &ExtDefaulted::operator=(ExtDefaulted &&) = default;
ExtDefaulted::~ExtDefaulted() = default;

void is_trivial2()
{
  int t01[T(__is_trivial(char))];
  int t02[T(__is_trivial(int))];
  int t03[T(__is_trivial(long))];
  int t04[T(__is_trivial(short))];
  int t05[T(__is_trivial(signed char))];
  int t06[T(__is_trivial(wchar_t))];
  int t07[T(__is_trivial(bool))];
  int t08[T(__is_trivial(float))];
  int t09[T(__is_trivial(double))];
  int t10[T(__is_trivial(long double))];
  int t11[T(__is_trivial(unsigned char))];
  int t12[T(__is_trivial(unsigned int))];
  int t13[T(__is_trivial(unsigned long long))];
  int t14[T(__is_trivial(unsigned long))];
  int t15[T(__is_trivial(unsigned short))];
  int t16[T(__is_trivial(ClassType))];
  int t17[T(__is_trivial(Derives))];
  int t18[T(__is_trivial(Enum))];
  int t19[T(__is_trivial(IntAr))];
  int t20[T(__is_trivial(Union))];
  int t21[T(__is_trivial(UnionAr))];
  int t22[T(__is_trivial(TrivialStruct))];
  int t23[T(__is_trivial(AllDefaulted))];
  int t24[T(__is_trivial(AllDeleted))];

  int t30[F(__is_trivial(void))];
  int t31[F(__is_trivial(NonTrivialStruct))];
  int t32[F(__is_trivial(SuperNonTrivialStruct))];
  int t33[F(__is_trivial(NonTCStruct))];
  int t34[F(__is_trivial(ExtDefaulted))];
}

void is_trivially_copyable2()
{
  int t01[T(__is_trivially_copyable(char))];
  int t02[T(__is_trivially_copyable(int))];
  int t03[T(__is_trivially_copyable(long))];
  int t04[T(__is_trivially_copyable(short))];
  int t05[T(__is_trivially_copyable(signed char))];
  int t06[T(__is_trivially_copyable(wchar_t))];
  int t07[T(__is_trivially_copyable(bool))];
  int t08[T(__is_trivially_copyable(float))];
  int t09[T(__is_trivially_copyable(double))];
  int t10[T(__is_trivially_copyable(long double))];
  int t11[T(__is_trivially_copyable(unsigned char))];
  int t12[T(__is_trivially_copyable(unsigned int))];
  int t13[T(__is_trivially_copyable(unsigned long long))];
  int t14[T(__is_trivially_copyable(unsigned long))];
  int t15[T(__is_trivially_copyable(unsigned short))];
  int t16[T(__is_trivially_copyable(ClassType))];
  int t17[T(__is_trivially_copyable(Derives))];
  int t18[T(__is_trivially_copyable(Enum))];
  int t19[T(__is_trivially_copyable(IntAr))];
  int t20[T(__is_trivially_copyable(Union))];
  int t21[T(__is_trivially_copyable(UnionAr))];
  int t22[T(__is_trivially_copyable(TrivialStruct))];
  int t23[T(__is_trivially_copyable(NonTrivialStruct))];
  int t24[T(__is_trivially_copyable(AllDefaulted))];
  int t25[T(__is_trivially_copyable(AllDeleted))];

  int t30[F(__is_trivially_copyable(void))];
  int t31[F(__is_trivially_copyable(SuperNonTrivialStruct))];
  int t32[F(__is_trivially_copyable(NonTCStruct))];
  int t33[F(__is_trivially_copyable(ExtDefaulted))];

  int t34[T(__is_trivially_copyable(const int))];
  int t35[F(__is_trivially_copyable(volatile int))];
}

struct CStruct {
  int one;
  int two;
};

struct CEmptyStruct {};

struct CppEmptyStruct : CStruct {};
struct CppStructStandard : CEmptyStruct {
  int three;
  int four;
};
struct CppStructNonStandardByBase : CStruct {
  int three;
  int four;
};
struct CppStructNonStandardByVirt : CStruct {
  virtual void method() {}
};
struct CppStructNonStandardByMemb : CStruct {
  CppStructNonStandardByVirt member;
};
struct CppStructNonStandardByProt : CStruct {
  int five;
protected:
  int six;
};
struct CppStructNonStandardByVirtBase : virtual CStruct {
};
struct CppStructNonStandardBySameBase : CEmptyStruct {
  CEmptyStruct member;
};
struct CppStructNonStandardBy2ndVirtBase : CEmptyStruct {
  CEmptyStruct member;
};

void is_standard_layout()
{
  typedef const int ConstInt;
  typedef ConstInt ConstIntAr[4];
  typedef CppStructStandard CppStructStandardAr[4];

  int t01[T(__is_standard_layout(int))];
  int t02[T(__is_standard_layout(ConstInt))];
  int t03[T(__is_standard_layout(ConstIntAr))];
  int t04[T(__is_standard_layout(CStruct))];
  int t05[T(__is_standard_layout(CppStructStandard))];
  int t06[T(__is_standard_layout(CppStructStandardAr))];
  int t07[T(__is_standard_layout(Vector))];
  int t08[T(__is_standard_layout(VectorExt))];

  typedef CppStructNonStandardByBase CppStructNonStandardByBaseAr[4];

  int t10[F(__is_standard_layout(CppStructNonStandardByVirt))];
  int t11[F(__is_standard_layout(CppStructNonStandardByMemb))];
  int t12[F(__is_standard_layout(CppStructNonStandardByProt))];
  int t13[F(__is_standard_layout(CppStructNonStandardByVirtBase))];
  int t14[F(__is_standard_layout(CppStructNonStandardByBase))];
  int t15[F(__is_standard_layout(CppStructNonStandardByBaseAr))];
  int t16[F(__is_standard_layout(CppStructNonStandardBySameBase))];
  int t17[F(__is_standard_layout(CppStructNonStandardBy2ndVirtBase))];
}

void is_signed()
{
  //int t01[T(__is_signed(char))];
  int t02[T(__is_signed(int))];
  int t03[T(__is_signed(long))];
  int t04[T(__is_signed(short))];
  int t05[T(__is_signed(signed char))];
  int t06[T(__is_signed(wchar_t))];

  int t10[F(__is_signed(bool))];
  int t11[F(__is_signed(cvoid))];
  int t12[F(__is_signed(float))];
  int t13[F(__is_signed(double))];
  int t14[F(__is_signed(long double))];
  int t15[F(__is_signed(unsigned char))];
  int t16[F(__is_signed(unsigned int))];
  int t17[F(__is_signed(unsigned long long))];
  int t18[F(__is_signed(unsigned long))];
  int t19[F(__is_signed(unsigned short))];
  int t20[F(__is_signed(void))];
  int t21[F(__is_signed(ClassType))];
  int t22[F(__is_signed(Derives))];
  int t23[F(__is_signed(Enum))];
  int t24[F(__is_signed(IntArNB))];
  int t25[F(__is_signed(Union))];
  int t26[F(__is_signed(UnionAr))];
}

void is_unsigned()
{
  int t01[T(__is_unsigned(bool))];
  int t02[T(__is_unsigned(unsigned char))];
  int t03[T(__is_unsigned(unsigned short))];
  int t04[T(__is_unsigned(unsigned int))];
  int t05[T(__is_unsigned(unsigned long))];
  int t06[T(__is_unsigned(unsigned long long))];
  int t07[T(__is_unsigned(Enum))];

  int t10[F(__is_unsigned(void))];
  int t11[F(__is_unsigned(cvoid))];
  int t12[F(__is_unsigned(float))];
  int t13[F(__is_unsigned(double))];
  int t14[F(__is_unsigned(long double))];
  int t16[F(__is_unsigned(char))];
  int t17[F(__is_unsigned(signed char))];
  int t18[F(__is_unsigned(wchar_t))];
  int t19[F(__is_unsigned(short))];
  int t20[F(__is_unsigned(int))];
  int t21[F(__is_unsigned(long))];
  int t22[F(__is_unsigned(Union))];
  int t23[F(__is_unsigned(UnionAr))];
  int t24[F(__is_unsigned(Derives))];
  int t25[F(__is_unsigned(ClassType))];
  int t26[F(__is_unsigned(IntArNB))];
}

typedef Int& IntRef;
typedef const IntAr ConstIntAr;
typedef ConstIntAr ConstIntArAr[4];

struct HasCopy {
  HasCopy(HasCopy& cp);
};

struct HasMove {
  HasMove(HasMove&& cp);
};

struct HasTemplateCons {
  HasVirt Annoying;

  template <typename T>
  HasTemplateCons(const T&);
};

void has_trivial_default_constructor() {
  { int arr[T(__has_trivial_constructor(Int))]; }
  { int arr[T(__has_trivial_constructor(IntAr))]; }
  { int arr[T(__has_trivial_constructor(Union))]; }
  { int arr[T(__has_trivial_constructor(UnionAr))]; }
  { int arr[T(__has_trivial_constructor(POD))]; }
  { int arr[T(__has_trivial_constructor(Derives))]; }
  { int arr[T(__has_trivial_constructor(DerivesAr))]; }
  { int arr[T(__has_trivial_constructor(ConstIntAr))]; }
  { int arr[T(__has_trivial_constructor(ConstIntArAr))]; }
  { int arr[T(__has_trivial_constructor(HasDest))]; }
  { int arr[T(__has_trivial_constructor(HasPriv))]; }
  { int arr[T(__has_trivial_constructor(HasCopyAssign))]; }
  { int arr[T(__has_trivial_constructor(HasMoveAssign))]; }
  { int arr[T(__has_trivial_constructor(const Int))]; }
  { int arr[T(__has_trivial_constructor(AllDefaulted))]; }
  { int arr[T(__has_trivial_constructor(AllDeleted))]; }

  { int arr[F(__has_trivial_constructor(HasCons))]; }
  { int arr[F(__has_trivial_constructor(HasRef))]; }
  { int arr[F(__has_trivial_constructor(HasCopy))]; }
  { int arr[F(__has_trivial_constructor(IntRef))]; }
  { int arr[F(__has_trivial_constructor(VirtAr))]; }
  { int arr[F(__has_trivial_constructor(void))]; }
  { int arr[F(__has_trivial_constructor(cvoid))]; }
  { int arr[F(__has_trivial_constructor(HasTemplateCons))]; }
  { int arr[F(__has_trivial_constructor(AllPrivate))]; }
  { int arr[F(__has_trivial_constructor(ExtDefaulted))]; }
}

void has_trivial_move_constructor() {
  // n3376 12.8 [class.copy]/12
  // A copy/move constructor for class X is trivial if it is not
  // user-provided, its declared parameter type is the same as
  // if it had been implicitly declared, and if
  //   - class X has no virtual functions (10.3) and no virtual
  //     base classes (10.1), and
  //   - the constructor selected to copy/move each direct base
  //     class subobject is trivial, and
  //   - for each non-static data member of X that is of class
  //     type (or array thereof), the constructor selected
  //     to copy/move that member is trivial;
  // otherwise the copy/move constructor is non-trivial.
  { int arr[T(__has_trivial_move_constructor(POD))]; }
  { int arr[T(__has_trivial_move_constructor(Union))]; }
  { int arr[T(__has_trivial_move_constructor(HasCons))]; }
  { int arr[T(__has_trivial_move_constructor(HasStaticMemberMoveCtor))]; }
  { int arr[T(__has_trivial_move_constructor(AllDeleted))]; }
  
  { int arr[F(__has_trivial_move_constructor(HasVirt))]; }
  { int arr[F(__has_trivial_move_constructor(DerivesVirt))]; }
  { int arr[F(__has_trivial_move_constructor(HasMoveCtor))]; }
  { int arr[F(__has_trivial_move_constructor(DerivesHasMoveCtor))]; }
  { int arr[F(__has_trivial_move_constructor(HasMemberMoveCtor))]; }
}

void has_trivial_copy_constructor() {
  { int arr[T(__has_trivial_copy(Int))]; }
  { int arr[T(__has_trivial_copy(IntAr))]; }
  { int arr[T(__has_trivial_copy(Union))]; }
  { int arr[T(__has_trivial_copy(UnionAr))]; }
  { int arr[T(__has_trivial_copy(POD))]; }
  { int arr[T(__has_trivial_copy(Derives))]; }
  { int arr[T(__has_trivial_copy(ConstIntAr))]; }
  { int arr[T(__has_trivial_copy(ConstIntArAr))]; }
  { int arr[T(__has_trivial_copy(HasDest))]; }
  { int arr[T(__has_trivial_copy(HasPriv))]; }
  { int arr[T(__has_trivial_copy(HasCons))]; }
  { int arr[T(__has_trivial_copy(HasRef))]; }
  { int arr[T(__has_trivial_copy(HasMove))]; }
  { int arr[T(__has_trivial_copy(IntRef))]; }
  { int arr[T(__has_trivial_copy(HasCopyAssign))]; }
  { int arr[T(__has_trivial_copy(HasMoveAssign))]; }
  { int arr[T(__has_trivial_copy(const Int))]; }
  { int arr[T(__has_trivial_copy(AllDefaulted))]; }
  { int arr[T(__has_trivial_copy(AllDeleted))]; }
  { int arr[T(__has_trivial_copy(DerivesAr))]; }
  { int arr[T(__has_trivial_copy(DerivesHasRef))]; }

  { int arr[F(__has_trivial_copy(HasCopy))]; }
  { int arr[F(__has_trivial_copy(HasTemplateCons))]; }
  { int arr[F(__has_trivial_copy(VirtAr))]; }
  { int arr[F(__has_trivial_copy(void))]; }
  { int arr[F(__has_trivial_copy(cvoid))]; }
  { int arr[F(__has_trivial_copy(AllPrivate))]; }
  { int arr[F(__has_trivial_copy(ExtDefaulted))]; }
}

void has_trivial_copy_assignment() {
  { int arr[T(__has_trivial_assign(Int))]; }
  { int arr[T(__has_trivial_assign(IntAr))]; }
  { int arr[T(__has_trivial_assign(Union))]; }
  { int arr[T(__has_trivial_assign(UnionAr))]; }
  { int arr[T(__has_trivial_assign(POD))]; }
  { int arr[T(__has_trivial_assign(Derives))]; }
  { int arr[T(__has_trivial_assign(HasDest))]; }
  { int arr[T(__has_trivial_assign(HasPriv))]; }
  { int arr[T(__has_trivial_assign(HasCons))]; }
  { int arr[T(__has_trivial_assign(HasRef))]; }
  { int arr[T(__has_trivial_assign(HasCopy))]; }
  { int arr[T(__has_trivial_assign(HasMove))]; }
  { int arr[T(__has_trivial_assign(HasMoveAssign))]; }
  { int arr[T(__has_trivial_assign(AllDefaulted))]; }
  { int arr[T(__has_trivial_assign(AllDeleted))]; }
  { int arr[T(__has_trivial_assign(DerivesAr))]; }
  { int arr[T(__has_trivial_assign(DerivesHasRef))]; }

  { int arr[F(__has_trivial_assign(IntRef))]; }
  { int arr[F(__has_trivial_assign(HasCopyAssign))]; }
  { int arr[F(__has_trivial_assign(const Int))]; }
  { int arr[F(__has_trivial_assign(ConstIntAr))]; }
  { int arr[F(__has_trivial_assign(ConstIntArAr))]; }
  { int arr[F(__has_trivial_assign(VirtAr))]; }
  { int arr[F(__has_trivial_assign(void))]; }
  { int arr[F(__has_trivial_assign(cvoid))]; }
  { int arr[F(__has_trivial_assign(AllPrivate))]; }
  { int arr[F(__has_trivial_assign(ExtDefaulted))]; }
}

void has_trivial_destructor() {
  { int arr[T(__has_trivial_destructor(Int))]; }
  { int arr[T(__has_trivial_destructor(IntAr))]; }
  { int arr[T(__has_trivial_destructor(Union))]; }
  { int arr[T(__has_trivial_destructor(UnionAr))]; }
  { int arr[T(__has_trivial_destructor(POD))]; }
  { int arr[T(__has_trivial_destructor(Derives))]; }
  { int arr[T(__has_trivial_destructor(ConstIntAr))]; }
  { int arr[T(__has_trivial_destructor(ConstIntArAr))]; }
  { int arr[T(__has_trivial_destructor(HasPriv))]; }
  { int arr[T(__has_trivial_destructor(HasCons))]; }
  { int arr[T(__has_trivial_destructor(HasRef))]; }
  { int arr[T(__has_trivial_destructor(HasCopy))]; }
  { int arr[T(__has_trivial_destructor(HasMove))]; }
  { int arr[T(__has_trivial_destructor(IntRef))]; }
  { int arr[T(__has_trivial_destructor(HasCopyAssign))]; }
  { int arr[T(__has_trivial_destructor(HasMoveAssign))]; }
  { int arr[T(__has_trivial_destructor(const Int))]; }
  { int arr[T(__has_trivial_destructor(DerivesAr))]; }
  { int arr[T(__has_trivial_destructor(VirtAr))]; }
  { int arr[T(__has_trivial_destructor(AllDefaulted))]; }
  { int arr[T(__has_trivial_destructor(AllDeleted))]; }
  { int arr[T(__has_trivial_destructor(DerivesHasRef))]; }

  { int arr[F(__has_trivial_destructor(HasDest))]; }
  { int arr[F(__has_trivial_destructor(void))]; }
  { int arr[F(__has_trivial_destructor(cvoid))]; }
  { int arr[F(__has_trivial_destructor(AllPrivate))]; }
  { int arr[F(__has_trivial_destructor(ExtDefaulted))]; }
}

struct A { ~A() {} };
template<typename> struct B : A { };

void f() {
  { int arr[F(__has_trivial_destructor(A))]; }
  { int arr[F(__has_trivial_destructor(B<int>))]; }
}

class PR11110 {
  template <int> int operator=( int );
  int operator=(PR11110);
};

class UsingAssign;

class UsingAssignBase {
protected:
  UsingAssign &operator=(const UsingAssign&) throw();
};

class UsingAssign : public UsingAssignBase {
public:
  using UsingAssignBase::operator=;
};

void has_nothrow_assign() {
  { int arr[T(__has_nothrow_assign(Int))]; }
  { int arr[T(__has_nothrow_assign(IntAr))]; }
  { int arr[T(__has_nothrow_assign(Union))]; }
  { int arr[T(__has_nothrow_assign(UnionAr))]; }
  { int arr[T(__has_nothrow_assign(POD))]; }
  { int arr[T(__has_nothrow_assign(Derives))]; }
  { int arr[T(__has_nothrow_assign(HasDest))]; }
  { int arr[T(__has_nothrow_assign(HasPriv))]; }
  { int arr[T(__has_nothrow_assign(HasCons))]; }
  { int arr[T(__has_nothrow_assign(HasRef))]; }
  { int arr[T(__has_nothrow_assign(HasCopy))]; }
  { int arr[T(__has_nothrow_assign(HasMove))]; }
  { int arr[T(__has_nothrow_assign(HasMoveAssign))]; }
  { int arr[T(__has_nothrow_assign(HasNoThrowCopyAssign))]; }
  { int arr[T(__has_nothrow_assign(HasMultipleNoThrowCopyAssign))]; }
  { int arr[T(__has_nothrow_assign(HasVirtDest))]; }
  { int arr[T(__has_nothrow_assign(AllPrivate))]; }
  { int arr[T(__has_nothrow_assign(UsingAssign))]; }
  { int arr[T(__has_nothrow_assign(DerivesAr))]; }

  { int arr[F(__has_nothrow_assign(IntRef))]; }
  { int arr[F(__has_nothrow_assign(HasCopyAssign))]; }
  { int arr[F(__has_nothrow_assign(HasMultipleCopyAssign))]; }
  { int arr[F(__has_nothrow_assign(const Int))]; }
  { int arr[F(__has_nothrow_assign(ConstIntAr))]; }
  { int arr[F(__has_nothrow_assign(ConstIntArAr))]; }
  { int arr[F(__has_nothrow_assign(VirtAr))]; }
  { int arr[F(__has_nothrow_assign(void))]; }
  { int arr[F(__has_nothrow_assign(cvoid))]; }
  { int arr[F(__has_nothrow_assign(PR11110))]; }
}

void has_nothrow_move_assign() {
  { int arr[T(__has_nothrow_move_assign(Int))]; }
  { int arr[T(__has_nothrow_move_assign(Enum))]; }
  { int arr[T(__has_nothrow_move_assign(Int*))]; }
  { int arr[T(__has_nothrow_move_assign(Enum POD::*))]; }
  { int arr[T(__has_nothrow_move_assign(POD))]; }
  { int arr[T(__has_nothrow_move_assign(HasPriv))]; }
  { int arr[T(__has_nothrow_move_assign(HasNoThrowMoveAssign))]; }
  { int arr[T(__has_nothrow_move_assign(HasNoExceptNoThrowMoveAssign))]; }
  { int arr[T(__has_nothrow_move_assign(HasMemberNoThrowMoveAssign))]; }
  { int arr[T(__has_nothrow_move_assign(HasMemberNoExceptNoThrowMoveAssign))]; }
  { int arr[T(__has_nothrow_move_assign(AllDeleted))]; }


  { int arr[F(__has_nothrow_move_assign(HasThrowMoveAssign))]; }
  { int arr[F(__has_nothrow_move_assign(HasNoExceptFalseMoveAssign))]; }
  { int arr[F(__has_nothrow_move_assign(HasMemberThrowMoveAssign))]; }
  { int arr[F(__has_nothrow_move_assign(HasMemberNoExceptFalseMoveAssign))]; }
  { int arr[F(__has_nothrow_move_assign(NoDefaultMoveAssignDueToUDCopyCtor))]; }
  { int arr[F(__has_nothrow_move_assign(NoDefaultMoveAssignDueToUDCopyAssign))]; }
  { int arr[F(__has_nothrow_move_assign(NoDefaultMoveAssignDueToDtor))]; }


  { int arr[T(__is_nothrow_assignable(HasNoThrowMoveAssign, HasNoThrowMoveAssign))]; }
  { int arr[F(__is_nothrow_assignable(HasThrowMoveAssign, HasThrowMoveAssign))]; }
}

void has_trivial_move_assign() {
  // n3376 12.8 [class.copy]/25
  // A copy/move assignment operator for class X is trivial if it
  // is not user-provided, its declared parameter type is the same
  // as if it had been implicitly declared, and if:
  //  - class X has no virtual functions (10.3) and no virtual base
  //    classes (10.1), and
  //  - the assignment operator selected to copy/move each direct
  //    base class subobject is trivial, and
  //  - for each non-static data member of X that is of class type
  //    (or array thereof), the assignment operator
  //    selected to copy/move that member is trivial;
  { int arr[T(__has_trivial_move_assign(Int))]; }
  { int arr[T(__has_trivial_move_assign(HasStaticMemberMoveAssign))]; }
  { int arr[T(__has_trivial_move_assign(AllDeleted))]; }

  { int arr[F(__has_trivial_move_assign(HasVirt))]; }
  { int arr[F(__has_trivial_move_assign(DerivesVirt))]; }
  { int arr[F(__has_trivial_move_assign(HasMoveAssign))]; }
  { int arr[F(__has_trivial_move_assign(DerivesHasMoveAssign))]; }
  { int arr[F(__has_trivial_move_assign(HasMemberMoveAssign))]; }
  { int arr[F(__has_nothrow_move_assign(NoDefaultMoveAssignDueToUDCopyCtor))]; }
  { int arr[F(__has_nothrow_move_assign(NoDefaultMoveAssignDueToUDCopyAssign))]; }
}

void has_nothrow_copy() {
  { int arr[T(__has_nothrow_copy(Int))]; }
  { int arr[T(__has_nothrow_copy(IntAr))]; }
  { int arr[T(__has_nothrow_copy(Union))]; }
  { int arr[T(__has_nothrow_copy(UnionAr))]; }
  { int arr[T(__has_nothrow_copy(POD))]; }
  { int arr[T(__has_nothrow_copy(const Int))]; }
  { int arr[T(__has_nothrow_copy(ConstIntAr))]; }
  { int arr[T(__has_nothrow_copy(ConstIntArAr))]; }
  { int arr[T(__has_nothrow_copy(Derives))]; }
  { int arr[T(__has_nothrow_copy(IntRef))]; }
  { int arr[T(__has_nothrow_copy(HasDest))]; }
  { int arr[T(__has_nothrow_copy(HasPriv))]; }
  { int arr[T(__has_nothrow_copy(HasCons))]; }
  { int arr[T(__has_nothrow_copy(HasRef))]; }
  { int arr[T(__has_nothrow_copy(HasMove))]; }
  { int arr[T(__has_nothrow_copy(HasCopyAssign))]; }
  { int arr[T(__has_nothrow_copy(HasMoveAssign))]; }
  { int arr[T(__has_nothrow_copy(HasNoThrowCopy))]; }
  { int arr[T(__has_nothrow_copy(HasMultipleNoThrowCopy))]; }
  { int arr[T(__has_nothrow_copy(HasVirtDest))]; }
  { int arr[T(__has_nothrow_copy(HasTemplateCons))]; }
  { int arr[T(__has_nothrow_copy(AllPrivate))]; }
  { int arr[T(__has_nothrow_copy(DerivesAr))]; }

  { int arr[F(__has_nothrow_copy(HasCopy))]; }
  { int arr[F(__has_nothrow_copy(HasMultipleCopy))]; }
  { int arr[F(__has_nothrow_copy(VirtAr))]; }
  { int arr[F(__has_nothrow_copy(void))]; }
  { int arr[F(__has_nothrow_copy(cvoid))]; }
}

void has_nothrow_constructor() {
  { int arr[T(__has_nothrow_constructor(Int))]; }
  { int arr[T(__has_nothrow_constructor(IntAr))]; }
  { int arr[T(__has_nothrow_constructor(Union))]; }
  { int arr[T(__has_nothrow_constructor(UnionAr))]; }
  { int arr[T(__has_nothrow_constructor(POD))]; }
  { int arr[T(__has_nothrow_constructor(Derives))]; }
  { int arr[T(__has_nothrow_constructor(DerivesAr))]; }
  { int arr[T(__has_nothrow_constructor(ConstIntAr))]; }
  { int arr[T(__has_nothrow_constructor(ConstIntArAr))]; }
  { int arr[T(__has_nothrow_constructor(HasDest))]; }
  { int arr[T(__has_nothrow_constructor(HasPriv))]; }
  { int arr[T(__has_nothrow_constructor(HasCopyAssign))]; }
  { int arr[T(__has_nothrow_constructor(const Int))]; }
  { int arr[T(__has_nothrow_constructor(HasNoThrowConstructor))]; }
  { int arr[T(__has_nothrow_constructor(HasVirtDest))]; }
  // { int arr[T(__has_nothrow_constructor(VirtAr))]; } // not implemented
  { int arr[T(__has_nothrow_constructor(AllPrivate))]; }

  { int arr[F(__has_nothrow_constructor(HasCons))]; }
  { int arr[F(__has_nothrow_constructor(HasRef))]; }
  { int arr[F(__has_nothrow_constructor(HasCopy))]; }
  { int arr[F(__has_nothrow_constructor(HasMove))]; }
  { int arr[F(__has_nothrow_constructor(HasNoThrowConstructorWithArgs))]; }
  { int arr[F(__has_nothrow_constructor(IntRef))]; }
  { int arr[F(__has_nothrow_constructor(void))]; }
  { int arr[F(__has_nothrow_constructor(cvoid))]; }
  { int arr[F(__has_nothrow_constructor(HasTemplateCons))]; }

  { int arr[F(__has_nothrow_constructor(HasMultipleDefaultConstructor1))]; }
  { int arr[F(__has_nothrow_constructor(HasMultipleDefaultConstructor2))]; }
}

void has_virtual_destructor() {
  { int arr[F(__has_virtual_destructor(Int))]; }
  { int arr[F(__has_virtual_destructor(IntAr))]; }
  { int arr[F(__has_virtual_destructor(Union))]; }
  { int arr[F(__has_virtual_destructor(UnionAr))]; }
  { int arr[F(__has_virtual_destructor(POD))]; }
  { int arr[F(__has_virtual_destructor(Derives))]; }
  { int arr[F(__has_virtual_destructor(DerivesAr))]; }
  { int arr[F(__has_virtual_destructor(const Int))]; }
  { int arr[F(__has_virtual_destructor(ConstIntAr))]; }
  { int arr[F(__has_virtual_destructor(ConstIntArAr))]; }
  { int arr[F(__has_virtual_destructor(HasDest))]; }
  { int arr[F(__has_virtual_destructor(HasPriv))]; }
  { int arr[F(__has_virtual_destructor(HasCons))]; }
  { int arr[F(__has_virtual_destructor(HasRef))]; }
  { int arr[F(__has_virtual_destructor(HasCopy))]; }
  { int arr[F(__has_virtual_destructor(HasMove))]; }
  { int arr[F(__has_virtual_destructor(HasCopyAssign))]; }
  { int arr[F(__has_virtual_destructor(HasMoveAssign))]; }
  { int arr[F(__has_virtual_destructor(IntRef))]; }
  { int arr[F(__has_virtual_destructor(VirtAr))]; }

  { int arr[T(__has_virtual_destructor(HasVirtDest))]; }
  { int arr[T(__has_virtual_destructor(DerivedVirtDest))]; }
  { int arr[F(__has_virtual_destructor(VirtDestAr))]; }
  { int arr[F(__has_virtual_destructor(void))]; }
  { int arr[F(__has_virtual_destructor(cvoid))]; }
  { int arr[F(__has_virtual_destructor(AllPrivate))]; }
}


class Base {};
class Derived : Base {};
class Derived2a : Derived {};
class Derived2b : Derived {};
class Derived3 : virtual Derived2a, virtual Derived2b {};
template<typename T> struct BaseA { T a;  };
template<typename T> struct DerivedB : BaseA<T> { };
template<typename T> struct CrazyDerived : T { };


class class_forward; // expected-note 2 {{forward declaration of 'class_forward'}}

template <typename Base, typename Derived>
void isBaseOfT() {
  int t[T(__is_base_of(Base, Derived))];
};
template <typename Base, typename Derived>
void isBaseOfF() {
  int t[F(__is_base_of(Base, Derived))];
};

template <class T> class DerivedTemp : Base {};
template <class T> class NonderivedTemp {};
template <class T> class UndefinedTemp; // expected-note {{declared here}}

void is_base_of() {
  { int arr[T(__is_base_of(Base, Derived))]; }
  { int arr[T(__is_base_of(const Base, Derived))]; }
  { int arr[F(__is_base_of(Derived, Base))]; }
  { int arr[F(__is_base_of(Derived, int))]; }
  { int arr[T(__is_base_of(Base, Base))]; }
  { int arr[T(__is_base_of(Base, Derived3))]; }
  { int arr[T(__is_base_of(Derived, Derived3))]; }
  { int arr[T(__is_base_of(Derived2b, Derived3))]; }
  { int arr[T(__is_base_of(Derived2a, Derived3))]; }
  { int arr[T(__is_base_of(BaseA<int>, DerivedB<int>))]; }
  { int arr[F(__is_base_of(DerivedB<int>, BaseA<int>))]; }
  { int arr[T(__is_base_of(Base, CrazyDerived<Base>))]; }
  { int arr[F(__is_base_of(Union, Union))]; }
  { int arr[T(__is_base_of(Empty, Empty))]; }
  { int arr[T(__is_base_of(class_forward, class_forward))]; }
  { int arr[F(__is_base_of(Empty, class_forward))]; } // expected-error {{incomplete type 'class_forward' used in type trait expression}}
  { int arr[F(__is_base_of(Base&, Derived&))]; }
  int t18[F(__is_base_of(Base[10], Derived[10]))];
  { int arr[F(__is_base_of(int, int))]; }
  { int arr[F(__is_base_of(long, int))]; }
  { int arr[T(__is_base_of(Base, DerivedTemp<int>))]; }
  { int arr[F(__is_base_of(Base, NonderivedTemp<int>))]; }
  { int arr[F(__is_base_of(Base, UndefinedTemp<int>))]; } // expected-error {{implicit instantiation of undefined template 'UndefinedTemp<int>'}}

  isBaseOfT<Base, Derived>();
  isBaseOfF<Derived, Base>();

  isBaseOfT<Base, CrazyDerived<Base> >();
  isBaseOfF<CrazyDerived<Base>, Base>();

  isBaseOfT<BaseA<int>, DerivedB<int> >();
  isBaseOfF<DerivedB<int>, BaseA<int> >();
}

template<class T, class U>
class TemplateClass {};

template<class T>
using TemplateAlias = TemplateClass<T, int>;

typedef class Base BaseTypedef;

void is_same()
{
  int t01[T(__is_same(Base, Base))];
  int t02[T(__is_same(Base, BaseTypedef))];
  int t03[T(__is_same(TemplateClass<int, int>, TemplateAlias<int>))];

  int t10[F(__is_same(Base, const Base))];
  int t11[F(__is_same(Base, Base&))];
  int t12[F(__is_same(Base, Derived))];
}

struct IntWrapper
{
  int value;
  IntWrapper(int _value) : value(_value) {}
  operator int() const {
    return value;
  }
};

struct FloatWrapper
{
  float value;
  FloatWrapper(float _value) : value(_value) {}
  FloatWrapper(const IntWrapper& obj)
    : value(static_cast<float>(obj.value)) {}
  operator float() const {
    return value;
  }
  operator IntWrapper() const {
    return IntWrapper(static_cast<int>(value));
  }
};

void is_convertible()
{
  int t01[T(__is_convertible(IntWrapper, IntWrapper))];
  int t02[T(__is_convertible(IntWrapper, const IntWrapper))];
  int t03[T(__is_convertible(IntWrapper, int))];
  int t04[T(__is_convertible(int, IntWrapper))];
  int t05[T(__is_convertible(IntWrapper, FloatWrapper))];
  int t06[T(__is_convertible(FloatWrapper, IntWrapper))];
  int t07[T(__is_convertible(FloatWrapper, float))];
  int t08[T(__is_convertible(float, FloatWrapper))];
}

struct FromInt { FromInt(int); };
struct ToInt { operator int(); };
typedef void Function();

void is_convertible_to();
class PrivateCopy {
  PrivateCopy(const PrivateCopy&);
  friend void is_convertible_to();
};

template<typename T>
struct X0 { 
  template<typename U> X0(const X0<U>&);
};

struct Abstract { virtual void f() = 0; };

void is_convertible_to() {
  { int arr[T(__is_convertible_to(Int, Int))]; }
  { int arr[F(__is_convertible_to(Int, IntAr))]; }
  { int arr[F(__is_convertible_to(IntAr, IntAr))]; }
  { int arr[T(__is_convertible_to(void, void))]; }
  { int arr[T(__is_convertible_to(cvoid, void))]; }
  { int arr[T(__is_convertible_to(void, cvoid))]; }
  { int arr[T(__is_convertible_to(cvoid, cvoid))]; }
  { int arr[T(__is_convertible_to(int, FromInt))]; }
  { int arr[T(__is_convertible_to(long, FromInt))]; }
  { int arr[T(__is_convertible_to(double, FromInt))]; }
  { int arr[T(__is_convertible_to(const int, FromInt))]; }
  { int arr[T(__is_convertible_to(const int&, FromInt))]; }
  { int arr[T(__is_convertible_to(ToInt, int))]; }
  { int arr[T(__is_convertible_to(ToInt, const int&))]; }
  { int arr[T(__is_convertible_to(ToInt, long))]; }
  { int arr[F(__is_convertible_to(ToInt, int&))]; }
  { int arr[F(__is_convertible_to(ToInt, FromInt))]; }
  { int arr[T(__is_convertible_to(IntAr&, IntAr&))]; }
  { int arr[T(__is_convertible_to(IntAr&, const IntAr&))]; }
  { int arr[F(__is_convertible_to(const IntAr&, IntAr&))]; }
  { int arr[F(__is_convertible_to(Function, Function))]; }
  { int arr[F(__is_convertible_to(PrivateCopy, PrivateCopy))]; }
  { int arr[T(__is_convertible_to(X0<int>, X0<float>))]; }
  { int arr[F(__is_convertible_to(Abstract, Abstract))]; }
}

namespace is_convertible_to_instantiate {
  // Make sure we don't try to instantiate the constructor.
  template<int x> class A { A(int) { int a[x]; } };
  int x = __is_convertible_to(int, A<-1>);
}

void is_trivial()
{
  { int arr[T(__is_trivial(int))]; }
  { int arr[T(__is_trivial(Enum))]; }
  { int arr[T(__is_trivial(POD))]; }
  { int arr[T(__is_trivial(Int))]; }
  { int arr[T(__is_trivial(IntAr))]; }
  { int arr[T(__is_trivial(IntArNB))]; }
  { int arr[T(__is_trivial(Statics))]; }
  { int arr[T(__is_trivial(Empty))]; }
  { int arr[T(__is_trivial(EmptyUnion))]; }
  { int arr[T(__is_trivial(Union))]; }
  { int arr[T(__is_trivial(Derives))]; }
  { int arr[T(__is_trivial(DerivesAr))]; }
  { int arr[T(__is_trivial(DerivesArNB))]; }
  { int arr[T(__is_trivial(DerivesEmpty))]; }
  { int arr[T(__is_trivial(HasFunc))]; }
  { int arr[T(__is_trivial(HasOp))]; }
  { int arr[T(__is_trivial(HasConv))]; }
  { int arr[T(__is_trivial(HasAssign))]; }
  { int arr[T(__is_trivial(HasAnonymousUnion))]; }
  { int arr[T(__is_trivial(HasPriv))]; }
  { int arr[T(__is_trivial(HasProt))]; }
  { int arr[T(__is_trivial(DerivesHasPriv))]; }
  { int arr[T(__is_trivial(DerivesHasProt))]; }
  { int arr[T(__is_trivial(Vector))]; }
  { int arr[T(__is_trivial(VectorExt))]; }

  { int arr[F(__is_trivial(HasCons))]; }
  { int arr[F(__is_trivial(HasCopyAssign))]; }
  { int arr[F(__is_trivial(HasMoveAssign))]; }
  { int arr[F(__is_trivial(HasDest))]; }
  { int arr[F(__is_trivial(HasRef))]; }
  { int arr[F(__is_trivial(HasNonPOD))]; }
  { int arr[F(__is_trivial(HasVirt))]; }
  { int arr[F(__is_trivial(DerivesHasCons))]; }
  { int arr[F(__is_trivial(DerivesHasCopyAssign))]; }
  { int arr[F(__is_trivial(DerivesHasMoveAssign))]; }
  { int arr[F(__is_trivial(DerivesHasDest))]; }
  { int arr[F(__is_trivial(DerivesHasRef))]; }
  { int arr[F(__is_trivial(DerivesHasVirt))]; }
  { int arr[F(__is_trivial(void))]; }
  { int arr[F(__is_trivial(cvoid))]; }
}

template<typename T> struct TriviallyConstructibleTemplate {};

void trivial_checks()
{
  { int arr[T(__is_trivially_copyable(int))]; }
  { int arr[T(__is_trivially_copyable(Enum))]; }
  { int arr[T(__is_trivially_copyable(POD))]; }
  { int arr[T(__is_trivially_copyable(Int))]; }
  { int arr[T(__is_trivially_copyable(IntAr))]; }
  { int arr[T(__is_trivially_copyable(IntArNB))]; }
  { int arr[T(__is_trivially_copyable(Statics))]; }
  { int arr[T(__is_trivially_copyable(Empty))]; }
  { int arr[T(__is_trivially_copyable(EmptyUnion))]; }
  { int arr[T(__is_trivially_copyable(Union))]; }
  { int arr[T(__is_trivially_copyable(Derives))]; }
  { int arr[T(__is_trivially_copyable(DerivesAr))]; }
  { int arr[T(__is_trivially_copyable(DerivesArNB))]; }
  { int arr[T(__is_trivially_copyable(DerivesEmpty))]; }
  { int arr[T(__is_trivially_copyable(HasFunc))]; }
  { int arr[T(__is_trivially_copyable(HasOp))]; }
  { int arr[T(__is_trivially_copyable(HasConv))]; }
  { int arr[T(__is_trivially_copyable(HasAssign))]; }
  { int arr[T(__is_trivially_copyable(HasAnonymousUnion))]; }
  { int arr[T(__is_trivially_copyable(HasPriv))]; }
  { int arr[T(__is_trivially_copyable(HasProt))]; }
  { int arr[T(__is_trivially_copyable(DerivesHasPriv))]; }
  { int arr[T(__is_trivially_copyable(DerivesHasProt))]; }
  { int arr[T(__is_trivially_copyable(Vector))]; }
  { int arr[T(__is_trivially_copyable(VectorExt))]; }
  { int arr[T(__is_trivially_copyable(HasCons))]; }
  { int arr[T(__is_trivially_copyable(HasRef))]; }
  { int arr[T(__is_trivially_copyable(HasNonPOD))]; }
  { int arr[T(__is_trivially_copyable(DerivesHasCons))]; }
  { int arr[T(__is_trivially_copyable(DerivesHasRef))]; }
  { int arr[T(__is_trivially_copyable(NonTrivialDefault))]; }
  { int arr[T(__is_trivially_copyable(NonTrivialDefault[]))]; }
  { int arr[T(__is_trivially_copyable(NonTrivialDefault[3]))]; }

  { int arr[F(__is_trivially_copyable(HasCopyAssign))]; }
  { int arr[F(__is_trivially_copyable(HasMoveAssign))]; }
  { int arr[F(__is_trivially_copyable(HasDest))]; }
  { int arr[F(__is_trivially_copyable(HasVirt))]; }
  { int arr[F(__is_trivially_copyable(DerivesHasCopyAssign))]; }
  { int arr[F(__is_trivially_copyable(DerivesHasMoveAssign))]; }
  { int arr[F(__is_trivially_copyable(DerivesHasDest))]; }
  { int arr[F(__is_trivially_copyable(DerivesHasVirt))]; }
  { int arr[F(__is_trivially_copyable(void))]; }
  { int arr[F(__is_trivially_copyable(cvoid))]; }

  { int arr[T((__is_trivially_constructible(int)))]; }
  { int arr[T((__is_trivially_constructible(int, int)))]; }
  { int arr[T((__is_trivially_constructible(int, float)))]; }
  { int arr[T((__is_trivially_constructible(int, int&)))]; }
  { int arr[T((__is_trivially_constructible(int, const int&)))]; }
  { int arr[T((__is_trivially_constructible(int, int)))]; }
  { int arr[T((__is_trivially_constructible(HasCopyAssign, HasCopyAssign)))]; }
  { int arr[T((__is_trivially_constructible(HasCopyAssign, const HasCopyAssign&)))]; }
  { int arr[T((__is_trivially_constructible(HasCopyAssign, HasCopyAssign&&)))]; }
  { int arr[T((__is_trivially_constructible(HasCopyAssign)))]; }
  { int arr[T((__is_trivially_constructible(NonTrivialDefault,
                                            const NonTrivialDefault&)))]; }
  { int arr[T((__is_trivially_constructible(NonTrivialDefault,
                                            NonTrivialDefault&&)))]; }
  { int arr[T((__is_trivially_constructible(AllDefaulted)))]; }
  { int arr[T((__is_trivially_constructible(AllDefaulted,
                                            const AllDefaulted &)))]; }
  { int arr[T((__is_trivially_constructible(AllDefaulted,
                                            AllDefaulted &&)))]; }

  { int arr[F((__is_trivially_constructible(int, int*)))]; }
  { int arr[F((__is_trivially_constructible(NonTrivialDefault)))]; }
  { int arr[F((__is_trivially_constructible(ThreeArgCtor, int*, char*, int&)))]; }
  { int arr[F((__is_trivially_constructible(AllDeleted)))]; }
  { int arr[F((__is_trivially_constructible(AllDeleted,
                                            const AllDeleted &)))]; }
  { int arr[F((__is_trivially_constructible(AllDeleted,
                                            AllDeleted &&)))]; }
  { int arr[F((__is_trivially_constructible(ExtDefaulted)))]; }
  { int arr[F((__is_trivially_constructible(ExtDefaulted,
                                            const ExtDefaulted &)))]; }
  { int arr[F((__is_trivially_constructible(ExtDefaulted,
                                            ExtDefaulted &&)))]; }

  { int arr[T((__is_trivially_constructible(TriviallyConstructibleTemplate<int>)))]; }
  { int arr[F((__is_trivially_constructible(class_forward)))]; } // expected-error {{incomplete type 'class_forward' used in type trait expression}}
  { int arr[F((__is_trivially_constructible(class_forward[])))]; }
  { int arr[F((__is_trivially_constructible(void)))]; }

  { int arr[T((__is_trivially_assignable(int&, int)))]; }
  { int arr[T((__is_trivially_assignable(int&, int&)))]; }
  { int arr[T((__is_trivially_assignable(int&, int&&)))]; }
  { int arr[T((__is_trivially_assignable(int&, const int&)))]; }
  { int arr[T((__is_trivially_assignable(POD&, POD)))]; }
  { int arr[T((__is_trivially_assignable(POD&, POD&)))]; }
  { int arr[T((__is_trivially_assignable(POD&, POD&&)))]; }
  { int arr[T((__is_trivially_assignable(POD&, const POD&)))]; }
  { int arr[T((__is_trivially_assignable(int*&, int*)))]; }
  { int arr[T((__is_trivially_assignable(AllDefaulted,
                                         const AllDefaulted &)))]; }
  { int arr[T((__is_trivially_assignable(AllDefaulted,
                                         AllDefaulted &&)))]; }

  { int arr[F((__is_trivially_assignable(int*&, float*)))]; }
  { int arr[F((__is_trivially_assignable(HasCopyAssign&, HasCopyAssign)))]; }
  { int arr[F((__is_trivially_assignable(HasCopyAssign&, HasCopyAssign&)))]; }
  { int arr[F((__is_trivially_assignable(HasCopyAssign&, const HasCopyAssign&)))]; }
  { int arr[F((__is_trivially_assignable(HasCopyAssign&, HasCopyAssign&&)))]; }
  { int arr[F((__is_trivially_assignable(TrivialMoveButNotCopy&,
                                        TrivialMoveButNotCopy&)))]; }
  { int arr[F((__is_trivially_assignable(TrivialMoveButNotCopy&,
                                        const TrivialMoveButNotCopy&)))]; }
  { int arr[F((__is_trivially_assignable(AllDeleted,
                                         const AllDeleted &)))]; }
  { int arr[F((__is_trivially_assignable(AllDeleted,
                                         AllDeleted &&)))]; }
  { int arr[F((__is_trivially_assignable(ExtDefaulted,
                                         const ExtDefaulted &)))]; }
  { int arr[F((__is_trivially_assignable(ExtDefaulted,
                                         ExtDefaulted &&)))]; }

  { int arr[T((__is_trivially_assignable(HasDefaultTrivialCopyAssign&,
                                         HasDefaultTrivialCopyAssign&)))]; }
  { int arr[T((__is_trivially_assignable(HasDefaultTrivialCopyAssign&,
                                       const HasDefaultTrivialCopyAssign&)))]; }
  { int arr[T((__is_trivially_assignable(TrivialMoveButNotCopy&,
                                         TrivialMoveButNotCopy)))]; }
  { int arr[T((__is_trivially_assignable(TrivialMoveButNotCopy&,
                                         TrivialMoveButNotCopy&&)))]; }
}

void constructible_checks() {
  { int arr[T(__is_constructible(HasNoThrowConstructorWithArgs))]; }
  { int arr[F(__is_nothrow_constructible(HasNoThrowConstructorWithArgs))]; } // MSVC doesn't look into default args and gets this wrong.

  { int arr[T(__is_constructible(HasNoThrowConstructorWithArgs, HasCons))]; }
  { int arr[T(__is_nothrow_constructible(HasNoThrowConstructorWithArgs, HasCons))]; }

  { int arr[T(__is_constructible(NonTrivialDefault))]; }
  { int arr[F(__is_nothrow_constructible(NonTrivialDefault))]; }

  { int arr[T(__is_constructible(int))]; }
  { int arr[T(__is_nothrow_constructible(int))]; }

  { int arr[F(__is_constructible(NonPOD))]; }
  { int arr[F(__is_nothrow_constructible(NonPOD))]; }

  { int arr[T(__is_constructible(NonPOD, int))]; }
  { int arr[F(__is_nothrow_constructible(NonPOD, int))]; }

  // PR19178
  { int arr[F(__is_constructible(Abstract))]; }
  { int arr[F(__is_nothrow_constructible(Abstract))]; }

  // PR20228
  { int arr[T(__is_constructible(VariadicCtor,
                                 int, int, int, int, int, int, int, int, int))]; }
}

// Instantiation of __is_trivially_constructible
template<typename T, typename ...Args>
struct is_trivially_constructible {
  static const bool value = __is_trivially_constructible(T, Args...);
};

void is_trivially_constructible_test() {
  { int arr[T((is_trivially_constructible<int>::value))]; }
  { int arr[T((is_trivially_constructible<int, int>::value))]; }
  { int arr[T((is_trivially_constructible<int, float>::value))]; }
  { int arr[T((is_trivially_constructible<int, int&>::value))]; }
  { int arr[T((is_trivially_constructible<int, const int&>::value))]; }
  { int arr[T((is_trivially_constructible<int, int>::value))]; }
  { int arr[T((is_trivially_constructible<HasCopyAssign, HasCopyAssign>::value))]; }
  { int arr[T((is_trivially_constructible<HasCopyAssign, const HasCopyAssign&>::value))]; }
  { int arr[T((is_trivially_constructible<HasCopyAssign, HasCopyAssign&&>::value))]; }
  { int arr[T((is_trivially_constructible<HasCopyAssign>::value))]; }
  { int arr[T((is_trivially_constructible<NonTrivialDefault,
                                            const NonTrivialDefault&>::value))]; }
  { int arr[T((is_trivially_constructible<NonTrivialDefault,
                                            NonTrivialDefault&&>::value))]; }

  { int arr[F((is_trivially_constructible<int, int*>::value))]; }
  { int arr[F((is_trivially_constructible<NonTrivialDefault>::value))]; }
  { int arr[F((is_trivially_constructible<ThreeArgCtor, int*, char*, int&>::value))]; }
  { int arr[F((is_trivially_constructible<Abstract>::value))]; } // PR19178
}

void array_rank() {
  int t01[T(__array_rank(IntAr) == 1)];
  int t02[T(__array_rank(ConstIntArAr) == 2)];
}

void array_extent() {
  int t01[T(__array_extent(IntAr, 0) == 10)];
  int t02[T(__array_extent(ConstIntArAr, 0) == 4)];
  int t03[T(__array_extent(ConstIntArAr, 1) == 10)];
}
