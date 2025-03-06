//===- unittest/AST/SourceLocationTest.cpp - AST source loc unit tests ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains tests for SourceLocation and SourceRange fields
// in AST nodes.
//
// FIXME: In the long-term, when we test more than source locations, we may
// want to have a unit test file for an AST node (or group of related nodes),
// rather than a unit test file for source locations for all AST nodes.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "MatchVerifier.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"
#include "gtest/gtest.h"

namespace clang {
namespace ast_matchers {

// FIXME: Pull the *Verifier tests into their own test file.

TEST(MatchVerifier, ParseError) {
  LocationVerifier<VarDecl> Verifier;
  Verifier.expectLocation(1, 1);
  EXPECT_FALSE(Verifier.match("int i", varDecl()));
}

TEST(MatchVerifier, NoMatch) {
  LocationVerifier<VarDecl> Verifier;
  Verifier.expectLocation(1, 1);
  EXPECT_FALSE(Verifier.match("int i;", recordDecl()));
}

TEST(MatchVerifier, WrongType) {
  LocationVerifier<RecordDecl> Verifier;
  Verifier.expectLocation(1, 1);
  EXPECT_FALSE(Verifier.match("int i;", varDecl()));
}

TEST(LocationVerifier, WrongLocation) {
  LocationVerifier<VarDecl> Verifier;
  Verifier.expectLocation(1, 1);
  EXPECT_FALSE(Verifier.match("int i;", varDecl()));
}

TEST(RangeVerifier, WrongRange) {
  RangeVerifier<VarDecl> Verifier;
  Verifier.expectRange(1, 1, 1, 1);
  EXPECT_FALSE(Verifier.match("int i;", varDecl()));
}

class LabelDeclRangeVerifier : public RangeVerifier<LabelStmt> {
protected:
  SourceRange getRange(const LabelStmt &Node) override {
    return Node.getDecl()->getSourceRange();
  }
};

TEST(LabelDecl, Range) {
  LabelDeclRangeVerifier Verifier;
  Verifier.expectRange(1, 12, 1, 12);
  EXPECT_TRUE(Verifier.match("void f() { l: return; }", labelStmt()));
}

TEST(LabelStmt, Range) {
  RangeVerifier<LabelStmt> Verifier;
  Verifier.expectRange(1, 12, 1, 15);
  EXPECT_TRUE(Verifier.match("void f() { l: return; }", labelStmt()));
}

TEST(ParmVarDecl, KNRLocation) {
  LocationVerifier<ParmVarDecl> Verifier;
  Verifier.expectLocation(1, 8);
  EXPECT_TRUE(Verifier.match("void f(i) {}", varDecl(), Lang_C));
}

TEST(ParmVarDecl, KNRRange) {
  RangeVerifier<ParmVarDecl> Verifier;
  Verifier.expectRange(1, 8, 1, 8);
  EXPECT_TRUE(Verifier.match("void f(i) {}", varDecl(), Lang_C));
}

TEST(CXXNewExpr, ArrayRange) {
  RangeVerifier<CXXNewExpr> Verifier;
  Verifier.expectRange(1, 12, 1, 22);
  EXPECT_TRUE(Verifier.match("void f() { new int[10]; }", newExpr()));
}

TEST(CXXNewExpr, ParenRange) {
  RangeVerifier<CXXNewExpr> Verifier;
  Verifier.expectRange(1, 12, 1, 20);
  EXPECT_TRUE(Verifier.match("void f() { new int(); }", newExpr()));
}

TEST(MemberExpr, ImplicitMemberRange) {
  RangeVerifier<MemberExpr> Verifier;
  Verifier.expectRange(2, 30, 2, 30);
  EXPECT_TRUE(Verifier.match("struct S { operator int() const; };\n"
                             "int foo(const S& s) { return s; }",
                             memberExpr()));
}

class MemberExprArrowLocVerifier : public RangeVerifier<MemberExpr> {
protected:
  SourceRange getRange(const MemberExpr &Node) override {
     return Node.getOperatorLoc();
  }
};

TEST(MemberExpr, ArrowRange) {
  MemberExprArrowLocVerifier Verifier;
  Verifier.expectRange(2, 19, 2, 19);
  EXPECT_TRUE(Verifier.match("struct S { int x; };\n"
                             "void foo(S *s) { s->x = 0; }",
                             memberExpr()));
}

TEST(MemberExpr, MacroArrowRange) {
  MemberExprArrowLocVerifier Verifier;
  Verifier.expectRange(1, 24, 1, 24);
  EXPECT_TRUE(Verifier.match("#define MEMBER(a, b) (a->b)\n"
                             "struct S { int x; };\n"
                             "void foo(S *s) { MEMBER(s, x) = 0; }",
                             memberExpr()));
}

TEST(MemberExpr, ImplicitArrowRange) {
  MemberExprArrowLocVerifier Verifier;
  Verifier.expectRange(0, 0, 0, 0);
  EXPECT_TRUE(Verifier.match("struct S { int x; void Test(); };\n"
                             "void S::Test() { x = 1; }",
                             memberExpr()));
}

TEST(VarDecl, VMTypeFixedVarDeclRange) {
  RangeVerifier<VarDecl> Verifier;
  Verifier.expectRange(1, 1, 1, 23);
  EXPECT_TRUE(Verifier.match("int a[(int)(void*)1234];",
                             varDecl(), Lang_C89));
}

TEST(CXXConstructorDecl, NoRetFunTypeLocRange) {
  RangeVerifier<CXXConstructorDecl> Verifier;
  Verifier.expectRange(1, 11, 1, 13);
  EXPECT_TRUE(Verifier.match("class C { C(); };", functionDecl()));
}

TEST(CXXConstructorDecl, DefaultedCtorLocRange) {
  RangeVerifier<CXXConstructorDecl> Verifier;
  Verifier.expectRange(1, 11, 1, 23);
  EXPECT_TRUE(Verifier.match("class C { C() = default; };", functionDecl()));
}

TEST(CXXConstructorDecl, DeletedCtorLocRange) {
  RangeVerifier<CXXConstructorDecl> Verifier;
  Verifier.expectRange(1, 11, 1, 22);
  EXPECT_TRUE(Verifier.match("class C { C() = delete; };", functionDecl()));
}

TEST(CompoundLiteralExpr, CompoundVectorLiteralRange) {
  RangeVerifier<CompoundLiteralExpr> Verifier;
  Verifier.expectRange(2, 11, 2, 22);
  EXPECT_TRUE(Verifier.match(
                  "typedef int int2 __attribute__((ext_vector_type(2)));\n"
                  "int2 i2 = (int2){1, 2};", compoundLiteralExpr()));
}

TEST(CompoundLiteralExpr, ParensCompoundVectorLiteralRange) {
  RangeVerifier<CompoundLiteralExpr> Verifier;
  Verifier.expectRange(2, 20, 2, 31);
  EXPECT_TRUE(Verifier.match(
                  "typedef int int2 __attribute__((ext_vector_type(2)));\n"
                  "constant int2 i2 = (int2)(1, 2);", 
                  compoundLiteralExpr(), Lang_OpenCL));
}

TEST(InitListExpr, VectorLiteralListBraceRange) {
  RangeVerifier<InitListExpr> Verifier;
  Verifier.expectRange(2, 17, 2, 22);
  EXPECT_TRUE(Verifier.match(
                  "typedef int int2 __attribute__((ext_vector_type(2)));\n"
                  "int2 i2 = (int2){1, 2};", initListExpr()));
}

TEST(InitListExpr, VectorLiteralInitListParens) {
  RangeVerifier<InitListExpr> Verifier;
  Verifier.expectRange(2, 26, 2, 31);
  EXPECT_TRUE(Verifier.match(
                  "typedef int int2 __attribute__((ext_vector_type(2)));\n"
                  "constant int2 i2 = (int2)(1, 2);", initListExpr(), Lang_OpenCL));
}

class TemplateAngleBracketLocRangeVerifier : public RangeVerifier<TypeLoc> {
protected:
  SourceRange getRange(const TypeLoc &Node) override {
    TemplateSpecializationTypeLoc T =
        Node.getUnqualifiedLoc().castAs<TemplateSpecializationTypeLoc>();
    assert(!T.isNull());
    return SourceRange(T.getLAngleLoc(), T.getRAngleLoc());
  }
};

TEST(TemplateSpecializationTypeLoc, AngleBracketLocations) {
  TemplateAngleBracketLocRangeVerifier Verifier;
  Verifier.expectRange(2, 8, 2, 10);
  EXPECT_TRUE(Verifier.match(
      "template<typename T> struct A {}; struct B{}; void f(\n"
      "const A<B>&);",
      loc(templateSpecializationType())));
}

TEST(CXXNewExpr, TypeParenRange) {
  RangeVerifier<CXXNewExpr> Verifier;
  Verifier.expectRange(1, 10, 1, 18);
  EXPECT_TRUE(Verifier.match("int* a = new (int);", newExpr()));
}

class UnaryTransformTypeLocParensRangeVerifier : public RangeVerifier<TypeLoc> {
protected:
  SourceRange getRange(const TypeLoc &Node) override {
    UnaryTransformTypeLoc T =
        Node.getUnqualifiedLoc().castAs<UnaryTransformTypeLoc>();
    assert(!T.isNull());
    return SourceRange(T.getLParenLoc(), T.getRParenLoc());
  }
};

TEST(UnaryTransformTypeLoc, ParensRange) {
  UnaryTransformTypeLocParensRangeVerifier Verifier;
  Verifier.expectRange(3, 26, 3, 28);
  EXPECT_TRUE(Verifier.match(
      "template <typename T>\n"
      "struct S {\n"
      "typedef __underlying_type(T) type;\n"
      "};",
      loc(unaryTransformType())));
}

TEST(CXXFunctionalCastExpr, SourceRange) {
  RangeVerifier<CXXFunctionalCastExpr> Verifier;
  Verifier.expectRange(2, 10, 2, 14);
  EXPECT_TRUE(Verifier.match(
      "int foo() {\n"
      "  return int{};\n"
      "}",
      functionalCastExpr(), Lang_CXX11));
}

TEST(CXXConstructExpr, SourceRange) {
  RangeVerifier<CXXConstructExpr> Verifier;
  Verifier.expectRange(3, 14, 3, 19);
  EXPECT_TRUE(Verifier.match(
      "struct A { A(int, int); };\n"
      "void f(A a);\n"
      "void g() { f({0, 0}); }",
      constructExpr(), Lang_CXX11));
}

TEST(CXXTemporaryObjectExpr, SourceRange) {
  RangeVerifier<CXXTemporaryObjectExpr> Verifier;
  Verifier.expectRange(2, 6, 2, 12);
  EXPECT_TRUE(Verifier.match(
      "struct A { A(int, int); };\n"
      "A a( A{0, 0} );",
      temporaryObjectExpr(), Lang_CXX11));
}

TEST(CXXUnresolvedConstructExpr, SourceRange) {
  RangeVerifier<CXXUnresolvedConstructExpr> Verifier;
  Verifier.expectRange(3, 10, 3, 12);
  std::vector<std::string> Args;
  Args.push_back("-fno-delayed-template-parsing");
  EXPECT_TRUE(Verifier.match(
      "template <typename U>\n"
      "U foo() {\n"
      "  return U{};\n"
      "}",
      unresolvedConstructExpr(), Args, Lang_CXX11));
}

TEST(UsingDecl, SourceRange) {
  RangeVerifier<UsingDecl> Verifier;
  Verifier.expectRange(2, 22, 2, 25);
  EXPECT_TRUE(Verifier.match(
      "class B { protected: int i; };\n"
      "class D : public B { B::i; };",
      usingDecl()));
}

TEST(UnresolvedUsingValueDecl, SourceRange) {
  RangeVerifier<UnresolvedUsingValueDecl> Verifier;
  Verifier.expectRange(3, 3, 3, 6);
  EXPECT_TRUE(Verifier.match(
      "template <typename B>\n"
      "class D : public B {\n"
      "  B::i;\n"
      "};",
      unresolvedUsingValueDecl()));
}

TEST(FriendDecl, FriendNonMemberFunctionLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(2, 13);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "friend void f();\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendNonMemberFunctionRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(2, 1, 2, 15);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "friend void f();\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendNonMemberFunctionDefinitionLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(2, 12);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "friend int f() { return 0; }\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendNonMemberFunctionDefinitionRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(2, 1, 2, 28);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "friend int f() { return 0; }\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendElaboratedTypeLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(2, 8);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "friend class B;\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendElaboratedTypeRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(2, 1, 2, 14);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "friend class B;\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendSimpleTypeLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(3, 8);
  EXPECT_TRUE(Verifier.match("class B;\n"
                             "struct A {\n"
                             "friend B;\n"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, FriendSimpleTypeRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(3, 1, 3, 8);
  EXPECT_TRUE(Verifier.match("class B;\n"
                             "struct A {\n"
                             "friend B;\n"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, FriendTemplateParameterLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(3, 8);
  EXPECT_TRUE(Verifier.match("template <typename T>\n"
                             "struct A {\n"
                             "friend T;\n"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, FriendTemplateParameterRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(3, 1, 3, 8);
  EXPECT_TRUE(Verifier.match("template <typename T>\n"
                             "struct A {\n"
                             "friend T;\n"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, FriendDecltypeLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(4, 8);
  EXPECT_TRUE(Verifier.match("struct A;\n"
                             "A foo();\n"
                             "struct A {\n"
                             "friend decltype(foo());\n"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, FriendDecltypeRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(4, 1, 4, 8);
  EXPECT_TRUE(Verifier.match("struct A;\n"
                             "A foo();\n"
                             "struct A {\n"
                             "friend decltype(foo());\n"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, FriendConstructorDestructorLocation) {
  const std::string Code = "struct B {\n"
                           "B();\n"
                           "~B();\n"
                           "};\n"
                           "struct A {\n"
                           "friend B::B(), B::~B();\n"
                           "};\n";
  LocationVerifier<FriendDecl> ConstructorVerifier;
  ConstructorVerifier.expectLocation(6, 11);
  EXPECT_TRUE(ConstructorVerifier.match(
      Code, friendDecl(has(constructorDecl(ofClass(hasName("B")))))));
  LocationVerifier<FriendDecl> DestructorVerifier;
  DestructorVerifier.expectLocation(6, 19);
  EXPECT_TRUE(DestructorVerifier.match(
      Code, friendDecl(has(destructorDecl(ofClass(hasName("B")))))));
}

TEST(FriendDecl, FriendConstructorDestructorRange) {
  const std::string Code = "struct B {\n"
                           "B();\n"
                           "~B();\n"
                           "};\n"
                           "struct A {\n"
                           "friend B::B(), B::~B();\n"
                           "};\n";
  RangeVerifier<FriendDecl> ConstructorVerifier;
  ConstructorVerifier.expectRange(6, 1, 6, 13);
  EXPECT_TRUE(ConstructorVerifier.match(
      Code, friendDecl(has(constructorDecl(ofClass(hasName("B")))))));
  RangeVerifier<FriendDecl> DestructorVerifier;
  DestructorVerifier.expectRange(6, 1, 6, 22);
  EXPECT_TRUE(DestructorVerifier.match(
      Code, friendDecl(has(destructorDecl(ofClass(hasName("B")))))));
}

TEST(FriendDecl, FriendTemplateFunctionLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(3, 13);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "template <typename T>\n"
                             "friend void f();\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendTemplateFunctionRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(2, 1, 3, 15);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "template <typename T>\n"
                             "friend void f();\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendTemplateClassLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(3, 14);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "template <typename T>\n"
                             "friend class B;\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendTemplateClassRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(2, 1, 3, 14);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "template <typename T>\n"
                             "friend class B;\n"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendInlineFunctionLocation) {
  LocationVerifier<FriendDecl> Verifier;
  Verifier.expectLocation(2, 19);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "int inline friend f() { return 0; }"
                             "};\n",
                             friendDecl()));
}

TEST(FriendDecl, FriendInlineFunctionRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(2, 1, 2, 35);
  EXPECT_TRUE(Verifier.match("struct A {\n"
                             "int inline friend f() { return 0; }"
                             "};\n",
                             friendDecl(), Lang_CXX11));
}

TEST(FriendDecl, InstantiationSourceRange) {
  RangeVerifier<FriendDecl> Verifier;
  Verifier.expectRange(4, 3, 4, 35);
  EXPECT_TRUE(Verifier.match(
      "template <typename T> class S;\n"
      "template<class T> void operator+(S<T> x);\n"
      "template<class T> struct S {\n"
      "  friend void operator+<>(S<T> src);\n"
      "};\n"
      "void test(S<double> s) { +s; }",
      friendDecl(hasParent(recordDecl(isTemplateInstantiation())))));
}

TEST(ObjCMessageExpr, CXXConstructExprRange) {
  RangeVerifier<CXXConstructExpr> Verifier;
  Verifier.expectRange(5, 25, 5, 27);
  EXPECT_TRUE(Verifier.match(
      "struct A { int a; };\n"
      "@interface B {}\n"
      "+ (void) f1: (A)arg;\n"
      "@end\n"
      "void f2() { A a; [B f1: (a)]; }\n",
      constructExpr(), Lang_OBJCXX));
}

} // end namespace ast_matchers
} // end namespace clang
