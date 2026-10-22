set(LLVM_LINK_COMPONENTS hlsl support)

set (HLSL_IGNORE_SOURCES
  NSAPI.cpp
  )

add_clang_library(clangAST
  APValue.cpp
  ASTConsumer.cpp
  ASTContext.cpp
  ASTContextHLSL.cpp # HLSL Change
  ASTDiagnostic.cpp
  ASTDumper.cpp
  ASTImporter.cpp
  ASTTypeTraits.cpp
  AttrImpl.cpp
  CXXInheritance.cpp
  Comment.cpp
  CommentBriefParser.cpp
  CommentCommandTraits.cpp
  CommentLexer.cpp
  CommentParser.cpp
  CommentSema.cpp
  Decl.cpp
  DeclarationName.cpp
  DeclBase.cpp
  DeclCXX.cpp
  DeclFriend.cpp
  DeclGroup.cpp
  DeclObjC.cpp
  DeclOpenMP.cpp
  DeclPrinter.cpp
  DeclTemplate.cpp
  Expr.cpp
  ExprClassification.cpp
  ExprConstant.cpp
  ExprCXX.cpp
  ExternalASTSource.cpp
  HlslBuiltinTypeDeclBuilder.cpp # HLSL Change
  HlslTypes.cpp # HLSL Change
  InheritViz.cpp
  ItaniumCXXABI.cpp
  ItaniumMangle.cpp
  Mangle.cpp
  MicrosoftCXXABI.cpp
  MicrosoftMangle.cpp
  NestedNameSpecifier.cpp
  ParentMap.cpp
  RawCommentList.cpp
  RecordLayout.cpp
  RecordLayoutBuilder.cpp
  SelectorLocationsKind.cpp
  Stmt.cpp
  StmtIterator.cpp
  StmtPrinter.cpp
  StmtProfile.cpp
  StmtViz.cpp
  TemplateBase.cpp
  TemplateName.cpp
  Type.cpp
  TypeLoc.cpp
  TypePrinter.cpp
  VTableBuilder.cpp
  VTTBuilder.cpp

  LINK_LIBS
  clangCodeGen
  clangBasic
  clangLex
  clangSema
  )

# MERGE REVIEW:
#  ClangAttrParsedAttrList  # MS Change - AST will refer to Sema headers
#  ClangAttrParsedAttrKinds # MS Change - AST will refer to Sema headers
#  ClangAttrParsedAttrImpl  # MS Change - AST will refer to Sema headers
