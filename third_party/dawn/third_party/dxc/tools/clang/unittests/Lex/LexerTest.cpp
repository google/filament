//===- unittests/Lex/LexerTest.cpp ------ Lexer tests ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace clang;

namespace {

class VoidModuleLoader : public ModuleLoader {
  ModuleLoadResult loadModule(SourceLocation ImportLoc, 
                              ModuleIdPath Path,
                              Module::NameVisibilityKind Visibility,
                              bool IsInclusionDirective) override {
    return ModuleLoadResult();
  }

  void makeModuleVisible(Module *Mod,
                         Module::NameVisibilityKind Visibility,
                         SourceLocation ImportLoc) override { }

  GlobalModuleIndex *loadGlobalModuleIndex(SourceLocation TriggerLoc) override
    { return nullptr; }
  bool lookupMissingImports(StringRef Name, SourceLocation TriggerLoc) override
    { return 0; };
};

// The test fixture.
class LexerTest : public ::testing::Test {
protected:
  LexerTest()
    : FileMgr(FileMgrOpts),
      DiagID(new DiagnosticIDs()),
      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
      SourceMgr(Diags, FileMgr),
      TargetOpts(new TargetOptions) 
  {
    TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
    Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  std::vector<Token> CheckLex(StringRef Source,
                              ArrayRef<tok::TokenKind> ExpectedTokens) {
    std::unique_ptr<MemoryBuffer> Buf = MemoryBuffer::getMemBuffer(Source);
    SourceMgr.setMainFileID(SourceMgr.createFileID(std::move(Buf)));

    VoidModuleLoader ModLoader;
    HeaderSearch HeaderInfo(new HeaderSearchOptions, SourceMgr, Diags, LangOpts,
                            Target.get());
    Preprocessor PP(new PreprocessorOptions(), Diags, LangOpts, SourceMgr,
                    HeaderInfo, ModLoader, /*IILookup =*/nullptr,
                    /*OwnsHeaderSearch =*/false);
    PP.Initialize(*Target);
    PP.EnterMainSourceFile();

    std::vector<Token> toks;
    while (1) {
      Token tok;
      PP.Lex(tok);
      if (tok.is(tok::eof))
        break;
      toks.push_back(tok);
    }

    EXPECT_EQ(ExpectedTokens.size(), toks.size());
    for (unsigned i = 0, e = ExpectedTokens.size(); i != e; ++i) {
      EXPECT_EQ(ExpectedTokens[i], toks[i].getKind());
    }

    return toks;
  }

  std::string getSourceText(Token Begin, Token End) {
    bool Invalid;
    StringRef Str =
        Lexer::getSourceText(CharSourceRange::getTokenRange(SourceRange(
                                    Begin.getLocation(), End.getLocation())),
                             SourceMgr, LangOpts, &Invalid);
    if (Invalid)
      return "<INVALID>";
    return Str;
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
};

TEST_F(LexerTest, GetSourceTextExpandsToMaximumInMacroArgument) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "M(f(M(i)))",
                                     ExpectedTokens);

  EXPECT_EQ("M(i)", getSourceText(toks[2], toks[2]));
}

TEST_F(LexerTest, GetSourceTextExpandsToMaximumInMacroArgumentForEndOfMacro) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "M(M(i) c)",
                                     ExpectedTokens);

  EXPECT_EQ("M(i)", getSourceText(toks[0], toks[0]));
}

TEST_F(LexerTest, GetSourceTextExpandsInMacroArgumentForBeginOfMacro) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "M(c c M(i))",
                                     ExpectedTokens);

  EXPECT_EQ("c M(i)", getSourceText(toks[1], toks[2]));
}

TEST_F(LexerTest, GetSourceTextExpandsInMacroArgumentForEndOfMacro) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "M(M(i) c c)",
                                     ExpectedTokens);

  EXPECT_EQ("M(i) c", getSourceText(toks[0], toks[1]));
}

TEST_F(LexerTest, GetSourceTextInSeparateFnMacros) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "M(c M(i)) M(M(i) c)",
                                     ExpectedTokens);

  EXPECT_EQ("<INVALID>", getSourceText(toks[1], toks[2]));
}

TEST_F(LexerTest, GetSourceTextWorksAcrossTokenPastes) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "#define C(x) M(x##c)\n"
                                     "M(f(C(i)))",
                                     ExpectedTokens);

  EXPECT_EQ("C(i)", getSourceText(toks[2], toks[2]));
}

TEST_F(LexerTest, GetSourceTextExpandsAcrossMultipleMacroCalls) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "f(M(M(i)))",
                                     ExpectedTokens);
  EXPECT_EQ("M(M(i))", getSourceText(toks[2], toks[2]));
}

TEST_F(LexerTest, GetSourceTextInMiddleOfMacroArgument) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "M(f(i))",
                                     ExpectedTokens);
  EXPECT_EQ("i", getSourceText(toks[2], toks[2]));
}

TEST_F(LexerTest, GetSourceTextExpandsAroundDifferentMacroCalls) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "#define C(x) x\n"
                                     "f(C(M(i)))",
                                     ExpectedTokens);
  EXPECT_EQ("C(M(i))", getSourceText(toks[2], toks[2]));
}

TEST_F(LexerTest, GetSourceTextOnlyExpandsIfFirstTokenInMacro) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "#define C(x) c x\n"
                                     "f(C(M(i)))",
                                     ExpectedTokens);
  EXPECT_EQ("M(i)", getSourceText(toks[3], toks[3]));
}

TEST_F(LexerTest, GetSourceTextExpandsRecursively) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::l_paren);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_paren);

  std::vector<Token> toks = CheckLex("#define M(x) x\n"
                                     "#define C(x) c M(x)\n"
                                     "C(f(M(i)))",
                                     ExpectedTokens);
  EXPECT_EQ("M(i)", getSourceText(toks[3], toks[3]));
}

TEST_F(LexerTest, LexAPI) {
  std::vector<tok::TokenKind> ExpectedTokens;
  ExpectedTokens.push_back(tok::l_square);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_square);
  ExpectedTokens.push_back(tok::l_square);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::r_square);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);
  ExpectedTokens.push_back(tok::identifier);

  std::vector<Token> toks = CheckLex("#define M(x) [x]\n"
                                     "#define N(x) x\n"
                                     "#define INN(x) x\n"
                                     "#define NOF1 INN(val)\n"
                                     "#define NOF2 val\n"
                                     "M(foo) N([bar])\n"
                                     "N(INN(val)) N(NOF1) N(NOF2) N(val)",
                                     ExpectedTokens);

  SourceLocation lsqrLoc = toks[0].getLocation();
  SourceLocation idLoc = toks[1].getLocation();
  SourceLocation rsqrLoc = toks[2].getLocation();
  std::pair<SourceLocation,SourceLocation>
    macroPair = SourceMgr.getExpansionRange(lsqrLoc);
  SourceRange macroRange = SourceRange(macroPair.first, macroPair.second);

  SourceLocation Loc;
  EXPECT_TRUE(Lexer::isAtStartOfMacroExpansion(lsqrLoc, SourceMgr, LangOpts, &Loc));
  EXPECT_EQ(Loc, macroRange.getBegin());
  EXPECT_FALSE(Lexer::isAtStartOfMacroExpansion(idLoc, SourceMgr, LangOpts));
  EXPECT_FALSE(Lexer::isAtEndOfMacroExpansion(idLoc, SourceMgr, LangOpts));
  EXPECT_TRUE(Lexer::isAtEndOfMacroExpansion(rsqrLoc, SourceMgr, LangOpts, &Loc));
  EXPECT_EQ(Loc, macroRange.getEnd());

  CharSourceRange range = Lexer::makeFileCharRange(
           CharSourceRange::getTokenRange(lsqrLoc, idLoc), SourceMgr, LangOpts);
  EXPECT_TRUE(range.isInvalid());
  range = Lexer::makeFileCharRange(CharSourceRange::getTokenRange(idLoc, rsqrLoc),
                                   SourceMgr, LangOpts);
  EXPECT_TRUE(range.isInvalid());
  range = Lexer::makeFileCharRange(CharSourceRange::getTokenRange(lsqrLoc, rsqrLoc),
                                   SourceMgr, LangOpts);
  EXPECT_TRUE(!range.isTokenRange());
  EXPECT_EQ(range.getAsRange(),
            SourceRange(macroRange.getBegin(),
                        macroRange.getEnd().getLocWithOffset(1)));

  StringRef text = Lexer::getSourceText(
                               CharSourceRange::getTokenRange(lsqrLoc, rsqrLoc),
                               SourceMgr, LangOpts);
  EXPECT_EQ(text, "M(foo)");

  SourceLocation macroLsqrLoc = toks[3].getLocation();
  SourceLocation macroIdLoc = toks[4].getLocation();
  SourceLocation macroRsqrLoc = toks[5].getLocation();
  SourceLocation fileLsqrLoc = SourceMgr.getSpellingLoc(macroLsqrLoc);
  SourceLocation fileIdLoc = SourceMgr.getSpellingLoc(macroIdLoc);
  SourceLocation fileRsqrLoc = SourceMgr.getSpellingLoc(macroRsqrLoc);

  range = Lexer::makeFileCharRange(
      CharSourceRange::getTokenRange(macroLsqrLoc, macroIdLoc),
      SourceMgr, LangOpts);
  EXPECT_EQ(SourceRange(fileLsqrLoc, fileIdLoc.getLocWithOffset(3)),
            range.getAsRange());

  range = Lexer::makeFileCharRange(CharSourceRange::getTokenRange(macroIdLoc, macroRsqrLoc),
                                   SourceMgr, LangOpts);
  EXPECT_EQ(SourceRange(fileIdLoc, fileRsqrLoc.getLocWithOffset(1)),
            range.getAsRange());

  macroPair = SourceMgr.getExpansionRange(macroLsqrLoc);
  range = Lexer::makeFileCharRange(
                     CharSourceRange::getTokenRange(macroLsqrLoc, macroRsqrLoc),
                     SourceMgr, LangOpts);
  EXPECT_EQ(SourceRange(macroPair.first, macroPair.second.getLocWithOffset(1)),
            range.getAsRange());

  text = Lexer::getSourceText(
          CharSourceRange::getTokenRange(SourceRange(macroLsqrLoc, macroIdLoc)),
          SourceMgr, LangOpts);
  EXPECT_EQ(text, "[bar");


  SourceLocation idLoc1 = toks[6].getLocation();
  SourceLocation idLoc2 = toks[7].getLocation();
  SourceLocation idLoc3 = toks[8].getLocation();
  SourceLocation idLoc4 = toks[9].getLocation();
  EXPECT_EQ("INN", Lexer::getImmediateMacroName(idLoc1, SourceMgr, LangOpts));
  EXPECT_EQ("INN", Lexer::getImmediateMacroName(idLoc2, SourceMgr, LangOpts));
  EXPECT_EQ("NOF2", Lexer::getImmediateMacroName(idLoc3, SourceMgr, LangOpts));
  EXPECT_EQ("N", Lexer::getImmediateMacroName(idLoc4, SourceMgr, LangOpts));
}

} // anonymous namespace
