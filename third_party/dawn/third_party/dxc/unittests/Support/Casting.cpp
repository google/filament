//===---------- llvm/unittest/Support/Casting.cpp - Casting tests ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Casting.h"
#include "llvm/IR/User.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <cstdlib>

namespace llvm {
// Used to test illegal cast. If a cast doesn't match any of the "real" ones,
// it will match this one.
struct IllegalCast;
template <typename T> IllegalCast *cast(...) { return nullptr; }

// set up two example classes
// with conversion facility
//
struct bar {
  bar() {}
  struct foo *baz();
  struct foo *caz();
  struct foo *daz();
  struct foo *naz();
private:
  bar(const bar &);
};
struct foo {
  void ext() const;
  /*  static bool classof(const bar *X) {
    cerr << "Classof: " << X << "\n";
    return true;
    }*/
};

template <> struct isa_impl<foo, bar> {
  static inline bool doit(const bar &Val) {
    dbgs() << "Classof: " << &Val << "\n";
    return true;
  }
};

foo *bar::baz() {
    return cast<foo>(this);
}

foo *bar::caz() {
    return cast_or_null<foo>(this);
}

foo *bar::daz() {
    return dyn_cast<foo>(this);
}

foo *bar::naz() {
    return dyn_cast_or_null<foo>(this);
}


bar *fub();

template <> struct simplify_type<foo> {
  typedef int SimpleType;
  static SimpleType getSimplifiedValue(foo &Val) { return 0; }
};

} // End llvm namespace

using namespace llvm;


// Test the peculiar behavior of Use in simplify_type.
static_assert(std::is_same<simplify_type<Use>::SimpleType, Value *>::value,
              "Use doesn't simplify correctly!");
static_assert(std::is_same<simplify_type<Use *>::SimpleType, Value *>::value,
              "Use doesn't simplify correctly!");

// Test that a regular class behaves as expected.
static_assert(std::is_same<simplify_type<foo>::SimpleType, int>::value,
              "Unexpected simplify_type result!");
static_assert(std::is_same<simplify_type<foo *>::SimpleType, foo *>::value,
              "Unexpected simplify_type result!");

namespace {

const foo *null_foo = nullptr;

bar B;
extern bar &B1;
bar &B1 = B;
extern const bar *B2;
// test various configurations of const
const bar &B3 = B1;
const bar *const B4 = B2;

TEST(CastingTest, isa) {
  EXPECT_TRUE(isa<foo>(B1));
  EXPECT_TRUE(isa<foo>(B2));
  EXPECT_TRUE(isa<foo>(B3));
  EXPECT_TRUE(isa<foo>(B4));
}

TEST(CastingTest, cast) {
  foo &F1 = cast<foo>(B1);
  EXPECT_NE(&F1, null_foo);
  const foo *F3 = cast<foo>(B2);
  EXPECT_NE(F3, null_foo);
  const foo *F4 = cast<foo>(B2);
  EXPECT_NE(F4, null_foo);
  const foo &F5 = cast<foo>(B3);
  EXPECT_NE(&F5, null_foo);
  const foo *F6 = cast<foo>(B4);
  EXPECT_NE(F6, null_foo);
  // Can't pass null pointer to cast<>.
  // foo *F7 = cast<foo>(fub());
  // EXPECT_EQ(F7, null_foo);
  foo *F8 = B1.baz();
  EXPECT_NE(F8, null_foo);
}

TEST(CastingTest, cast_or_null) {
  const foo *F11 = cast_or_null<foo>(B2);
  EXPECT_NE(F11, null_foo);
  const foo *F12 = cast_or_null<foo>(B2);
  EXPECT_NE(F12, null_foo);
  const foo *F13 = cast_or_null<foo>(B4);
  EXPECT_NE(F13, null_foo);
  const foo *F14 = cast_or_null<foo>(fub());  // Shouldn't print.
  EXPECT_EQ(F14, null_foo);
  foo *F15 = B1.caz();
  EXPECT_NE(F15, null_foo);
}

TEST(CastingTest, dyn_cast) {
  const foo *F1 = dyn_cast<foo>(B2);
  EXPECT_NE(F1, null_foo);
  const foo *F2 = dyn_cast<foo>(B2);
  EXPECT_NE(F2, null_foo);
  const foo *F3 = dyn_cast<foo>(B4);
  EXPECT_NE(F3, null_foo);
  // Can't pass null pointer to dyn_cast<>.
  // foo *F4 = dyn_cast<foo>(fub());
  // EXPECT_EQ(F4, null_foo);
  foo *F5 = B1.daz();
  EXPECT_NE(F5, null_foo);
}

TEST(CastingTest, dyn_cast_or_null) {
  const foo *F1 = dyn_cast_or_null<foo>(B2);
  EXPECT_NE(F1, null_foo);
  const foo *F2 = dyn_cast_or_null<foo>(B2);
  EXPECT_NE(F2, null_foo);
  const foo *F3 = dyn_cast_or_null<foo>(B4);
  EXPECT_NE(F3, null_foo);
  foo *F4 = dyn_cast_or_null<foo>(fub());
  EXPECT_EQ(F4, null_foo);
  foo *F5 = B1.naz();
  EXPECT_NE(F5, null_foo);
}

// These lines are errors...
//foo *F20 = cast<foo>(B2);  // Yields const foo*
//foo &F21 = cast<foo>(B3);  // Yields const foo&
//foo *F22 = cast<foo>(B4);  // Yields const foo*
//foo &F23 = cast_or_null<foo>(B1);
//const foo &F24 = cast_or_null<foo>(B3);

const bar *B2 = &B;
}  // anonymous namespace

bar *llvm::fub() { return nullptr; }

namespace {
namespace inferred_upcasting {
// This test case verifies correct behavior of inferred upcasts when the
// types are statically known to be OK to upcast. This is the case when,
// for example, Derived inherits from Base, and we do `isa<Base>(Derived)`.

// Note: This test will actually fail to compile without inferred
// upcasting.

class Base {
public:
  // No classof. We are testing that the upcast is inferred.
  Base() {}
};

class Derived : public Base {
public:
  Derived() {}
};

// Even with no explicit classof() in Base, we should still be able to cast
// Derived to its base class.
TEST(CastingTest, UpcastIsInferred) {
  Derived D;
  EXPECT_TRUE(isa<Base>(D));
  Base *BP = dyn_cast<Base>(&D);
  EXPECT_TRUE(BP != nullptr);
}


// This test verifies that the inferred upcast takes precedence over an
// explicitly written one. This is important because it verifies that the
// dynamic check gets optimized away.
class UseInferredUpcast {
public:
  int Dummy;
  static bool classof(const UseInferredUpcast *) {
    return false;
  }
};

TEST(CastingTest, InferredUpcastTakesPrecedence) {
  UseInferredUpcast UIU;
  // Since the explicit classof() returns false, this will fail if the
  // explicit one is used.
  EXPECT_TRUE(isa<UseInferredUpcast>(&UIU));
}

} // end namespace inferred_upcasting
} // end anonymous namespace
// Test that we reject casts of temporaries (and so the illegal cast gets used).
namespace TemporaryCast {
struct pod {};
IllegalCast *testIllegalCast() { return cast<foo>(pod()); }
}

namespace {
namespace pointer_wrappers {

struct Base {
  bool IsDerived;
  Base(bool IsDerived = false) : IsDerived(IsDerived) {}
};

struct Derived : Base {
  Derived() : Base(true) {}
  static bool classof(const Base *B) { return B->IsDerived; }
};

class PTy {
  Base *B;
public:
  PTy(Base *B) : B(B) {}
  explicit operator bool() const { return get(); }
  Base *get() const { return B; }
};

} // end namespace pointer_wrappers
} // end namespace

namespace llvm {

template <> struct simplify_type<pointer_wrappers::PTy> {
  typedef pointer_wrappers::Base *SimpleType;
  static SimpleType getSimplifiedValue(pointer_wrappers::PTy &P) {
    return P.get();
  }
};
template <> struct simplify_type<const pointer_wrappers::PTy> {
  typedef pointer_wrappers::Base *SimpleType;
  static SimpleType getSimplifiedValue(const pointer_wrappers::PTy &P) {
    return P.get();
  }
};

} // end namespace llvm

namespace {
namespace pointer_wrappers {

// Some objects.
pointer_wrappers::Base B;
pointer_wrappers::Derived D;

// Mutable "smart" pointers.
pointer_wrappers::PTy MN(nullptr);
pointer_wrappers::PTy MB(&B);
pointer_wrappers::PTy MD(&D);

// Const "smart" pointers.
const pointer_wrappers::PTy CN(nullptr);
const pointer_wrappers::PTy CB(&B);
const pointer_wrappers::PTy CD(&D);

TEST(CastingTest, smart_isa) {
  EXPECT_TRUE(!isa<pointer_wrappers::Derived>(MB));
  EXPECT_TRUE(!isa<pointer_wrappers::Derived>(CB));
  EXPECT_TRUE(isa<pointer_wrappers::Derived>(MD));
  EXPECT_TRUE(isa<pointer_wrappers::Derived>(CD));
}

TEST(CastingTest, smart_cast) {
  EXPECT_TRUE(cast<pointer_wrappers::Derived>(MD) == &D);
  EXPECT_TRUE(cast<pointer_wrappers::Derived>(CD) == &D);
}

TEST(CastingTest, smart_cast_or_null) {
  EXPECT_TRUE(cast_or_null<pointer_wrappers::Derived>(MN) == nullptr);
  EXPECT_TRUE(cast_or_null<pointer_wrappers::Derived>(CN) == nullptr);
  EXPECT_TRUE(cast_or_null<pointer_wrappers::Derived>(MD) == &D);
  EXPECT_TRUE(cast_or_null<pointer_wrappers::Derived>(CD) == &D);
}

TEST(CastingTest, smart_dyn_cast) {
  EXPECT_TRUE(dyn_cast<pointer_wrappers::Derived>(MB) == nullptr);
  EXPECT_TRUE(dyn_cast<pointer_wrappers::Derived>(CB) == nullptr);
  EXPECT_TRUE(dyn_cast<pointer_wrappers::Derived>(MD) == &D);
  EXPECT_TRUE(dyn_cast<pointer_wrappers::Derived>(CD) == &D);
}

TEST(CastingTest, smart_dyn_cast_or_null) {
  EXPECT_TRUE(dyn_cast_or_null<pointer_wrappers::Derived>(MN) == nullptr);
  EXPECT_TRUE(dyn_cast_or_null<pointer_wrappers::Derived>(CN) == nullptr);
  EXPECT_TRUE(dyn_cast_or_null<pointer_wrappers::Derived>(MB) == nullptr);
  EXPECT_TRUE(dyn_cast_or_null<pointer_wrappers::Derived>(CB) == nullptr);
  EXPECT_TRUE(dyn_cast_or_null<pointer_wrappers::Derived>(MD) == &D);
  EXPECT_TRUE(dyn_cast_or_null<pointer_wrappers::Derived>(CD) == &D);
}

} // end namespace pointer_wrappers
} // end namespace
