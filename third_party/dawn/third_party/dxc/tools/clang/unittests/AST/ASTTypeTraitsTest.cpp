//===- unittest/AST/ASTTypeTraits.cpp - AST type traits unit tests ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//

#include "clang/AST/ASTTypeTraits.h"
#include "MatchVerifier.h"
#include "gtest/gtest.h"

using namespace clang::ast_matchers;

namespace clang {
namespace ast_type_traits {

TEST(ASTNodeKind, NoKind) {
  EXPECT_FALSE(ASTNodeKind().isBaseOf(ASTNodeKind()));
  EXPECT_FALSE(ASTNodeKind().isSame(ASTNodeKind()));
}

template <typename T> static ASTNodeKind DNT() {
  return ASTNodeKind::getFromNodeKind<T>();
}

TEST(ASTNodeKind, IsNone) {
  EXPECT_TRUE(ASTNodeKind().isNone());
  EXPECT_FALSE(DNT<Decl>().isNone());
  EXPECT_FALSE(DNT<VarDecl>().isNone());
}

TEST(ASTNodeKind, Bases) {
  EXPECT_TRUE(DNT<Decl>().isBaseOf(DNT<VarDecl>()));
  EXPECT_FALSE(DNT<Decl>().isSame(DNT<VarDecl>()));
  EXPECT_FALSE(DNT<VarDecl>().isBaseOf(DNT<Decl>()));

  EXPECT_TRUE(DNT<Decl>().isSame(DNT<Decl>()));
}

TEST(ASTNodeKind, BaseDistances) {
  unsigned Distance = 1;
  EXPECT_TRUE(DNT<Expr>().isBaseOf(DNT<Expr>(), &Distance));
  EXPECT_EQ(0u, Distance);

  EXPECT_TRUE(DNT<Stmt>().isBaseOf(DNT<IfStmt>(), &Distance));
  EXPECT_EQ(1u, Distance);

  Distance = 3;
  EXPECT_TRUE(DNT<DeclaratorDecl>().isBaseOf(DNT<ParmVarDecl>(), &Distance));
  EXPECT_EQ(2u, Distance);
}

TEST(ASTNodeKind, SameBase) {
  EXPECT_TRUE(DNT<Expr>().isBaseOf(DNT<CallExpr>()));
  EXPECT_TRUE(DNT<Expr>().isBaseOf(DNT<BinaryOperator>()));
  EXPECT_FALSE(DNT<CallExpr>().isBaseOf(DNT<BinaryOperator>()));
  EXPECT_FALSE(DNT<BinaryOperator>().isBaseOf(DNT<CallExpr>()));
}

TEST(ASTNodeKind, DiffBase) {
  EXPECT_FALSE(DNT<Expr>().isBaseOf(DNT<ArrayType>()));
  EXPECT_FALSE(DNT<QualType>().isBaseOf(DNT<FunctionDecl>()));
  EXPECT_FALSE(DNT<Type>().isSame(DNT<QualType>()));
}

TEST(ASTNodeKind, MostDerivedType) {
  EXPECT_TRUE(DNT<BinaryOperator>().isSame(
      ASTNodeKind::getMostDerivedType(DNT<Expr>(), DNT<BinaryOperator>())));
  EXPECT_TRUE(DNT<BinaryOperator>().isSame(
      ASTNodeKind::getMostDerivedType(DNT<BinaryOperator>(), DNT<Expr>())));
  EXPECT_TRUE(DNT<VarDecl>().isSame(
      ASTNodeKind::getMostDerivedType(DNT<VarDecl>(), DNT<VarDecl>())));

  // Not related. Returns nothing.
  EXPECT_TRUE(
      ASTNodeKind::getMostDerivedType(DNT<IfStmt>(), DNT<VarDecl>()).isNone());
  EXPECT_TRUE(ASTNodeKind::getMostDerivedType(DNT<IfStmt>(),
                                              DNT<BinaryOperator>()).isNone());
}

TEST(ASTNodeKind, MostDerivedCommonAncestor) {
  EXPECT_TRUE(DNT<Expr>().isSame(ASTNodeKind::getMostDerivedCommonAncestor(
      DNT<Expr>(), DNT<BinaryOperator>())));
  EXPECT_TRUE(DNT<Expr>().isSame(ASTNodeKind::getMostDerivedCommonAncestor(
      DNT<BinaryOperator>(), DNT<Expr>())));
  EXPECT_TRUE(DNT<VarDecl>().isSame(ASTNodeKind::getMostDerivedCommonAncestor(
      DNT<VarDecl>(), DNT<VarDecl>())));

  // A little related. Returns the ancestor.
  EXPECT_TRUE(
      DNT<NamedDecl>().isSame(ASTNodeKind::getMostDerivedCommonAncestor(
          DNT<CXXMethodDecl>(), DNT<RecordDecl>())));

  // Not related. Returns nothing.
  EXPECT_TRUE(ASTNodeKind::getMostDerivedCommonAncestor(
                  DNT<IfStmt>(), DNT<VarDecl>()).isNone());
}

struct Foo {};

TEST(ASTNodeKind, UnknownKind) {
  // We can construct one, but it is nowhere in the hierarchy.
  EXPECT_FALSE(DNT<Foo>().isSame(DNT<Foo>()));
}

TEST(ASTNodeKind, Name) {
  EXPECT_EQ("Decl", DNT<Decl>().asStringRef());
  EXPECT_EQ("CallExpr", DNT<CallExpr>().asStringRef());
  EXPECT_EQ("ConstantArrayType", DNT<ConstantArrayType>().asStringRef());
  EXPECT_EQ("<None>", ASTNodeKind().asStringRef());
}

TEST(DynTypedNode, DeclSourceRange) {
  RangeVerifier<DynTypedNode> Verifier;
  Verifier.expectRange(1, 1, 1, 11);
  EXPECT_TRUE(Verifier.match("void f() {}", decl()));
}

TEST(DynTypedNode, StmtSourceRange) {
  RangeVerifier<DynTypedNode> Verifier;
  Verifier.expectRange(1, 10, 1, 11);
  EXPECT_TRUE(Verifier.match("void f() {}", stmt()));
}

TEST(DynTypedNode, TypeLocSourceRange) {
  RangeVerifier<DynTypedNode> Verifier;
  Verifier.expectRange(1, 1, 1, 8);
  EXPECT_TRUE(Verifier.match("void f() {}", typeLoc(loc(functionType()))));
}

TEST(DynTypedNode, NNSLocSourceRange) {
  RangeVerifier<DynTypedNode> Verifier;
  Verifier.expectRange(1, 33, 1, 34);
  EXPECT_TRUE(Verifier.match("namespace N { typedef void T; } N::T f() {}",
                             nestedNameSpecifierLoc()));
}

TEST(DynTypedNode, DeclDump) {
  DumpVerifier Verifier;
  Verifier.expectSubstring("FunctionDecl");
  EXPECT_TRUE(Verifier.match("void f() {}", functionDecl()));
}

TEST(DynTypedNode, StmtDump) {
  DumpVerifier Verifier;
  Verifier.expectSubstring("CompoundStmt");
  EXPECT_TRUE(Verifier.match("void f() {}", stmt()));
}

TEST(DynTypedNode, DeclPrint) {
  PrintVerifier Verifier;
  Verifier.expectString("void f() {\n}\n\n");
  EXPECT_TRUE(Verifier.match("void f() {}", functionDecl()));
}

TEST(DynTypedNode, StmtPrint) {
  PrintVerifier Verifier;
  Verifier.expectString("{\n}\n");
  EXPECT_TRUE(Verifier.match("void f() {}", stmt()));
}

}  // namespace ast_type_traits
}  // namespace clang
