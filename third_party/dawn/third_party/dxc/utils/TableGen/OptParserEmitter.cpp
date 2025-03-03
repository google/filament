//===- OptParserEmitter.cpp - Table Driven Command Line Parsing -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/TableGen/Error.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <cctype>
#include <cstring>
#include <map>

using namespace llvm;

// Ordering on Info. The logic should match with the consumer-side function in
// llvm/Option/OptTable.h.
static int StrCmpOptionName(const char *A, const char *B) {
  const char *X = A, *Y = B;
  char a = tolower(*A), b = tolower(*B);
  while (a == b) {
    if (a == '\0')
      return strcmp(A, B);

    a = tolower(*++X);
    b = tolower(*++Y);
  }

  if (a == '\0') // A is a prefix of B.
    return 1;
  if (b == '\0') // B is a prefix of A.
    return -1;

  // Otherwise lexicographic.
  return (a < b) ? -1 : 1;
}

// HLSL Change: changed calling convention to __cdecl
static int __cdecl CompareOptionRecords(Record *const *Av, Record *const *Bv) {
  const Record *A = *Av;
  const Record *B = *Bv;

  // Sentinel options precede all others and are only ordered by precedence.
  bool ASent = A->getValueAsDef("Kind")->getValueAsBit("Sentinel");
  bool BSent = B->getValueAsDef("Kind")->getValueAsBit("Sentinel");
  if (ASent != BSent)
    return ASent ? -1 : 1;

  // Compare options by name, unless they are sentinels.
  if (!ASent)
    if (int Cmp = StrCmpOptionName(A->getValueAsString("Name").c_str(),
                                   B->getValueAsString("Name").c_str()))
      return Cmp;

  if (!ASent) {
    std::vector<std::string> APrefixes = A->getValueAsListOfStrings("Prefixes");
    std::vector<std::string> BPrefixes = B->getValueAsListOfStrings("Prefixes");

    for (std::vector<std::string>::const_iterator APre = APrefixes.begin(),
                                                  AEPre = APrefixes.end(),
                                                  BPre = BPrefixes.begin(),
                                                  BEPre = BPrefixes.end();
                                                  APre != AEPre &&
                                                  BPre != BEPre;
                                                  ++APre, ++BPre) {
      if (int Cmp = StrCmpOptionName(APre->c_str(), BPre->c_str()))
        return Cmp;
    }
  }

  // Then by the kind precedence;
  int APrec = A->getValueAsDef("Kind")->getValueAsInt("Precedence");
  int BPrec = B->getValueAsDef("Kind")->getValueAsInt("Precedence");
  if (APrec == BPrec &&
      A->getValueAsListOfStrings("Prefixes") ==
      B->getValueAsListOfStrings("Prefixes")) {
    PrintError(A->getLoc(), Twine("Option is equivalent to"));
    PrintError(B->getLoc(), Twine("Other defined here"));
    PrintFatalError("Equivalent Options found.");
  }
  return APrec < BPrec ? -1 : 1;
}

static const std::string getOptionName(const Record &R) {
  // Use the record name unless EnumName is defined.
  if (isa<UnsetInit>(R.getValueInit("EnumName")))
    return R.getName();

  return R.getValueAsString("EnumName");
}

static raw_ostream &write_cstring(raw_ostream &OS, llvm::StringRef Str) {
  OS << '"';
  OS.write_escaped(Str);
  OS << '"';
  return OS;
}

/// OptParserEmitter - This tablegen backend takes an input .td file
/// describing a list of options and emits a data structure for parsing and
/// working with those options when given an input command line.
namespace llvm {
void EmitOptParser(RecordKeeper &Records, raw_ostream &OS) {
  // Get the option groups and options.
  const std::vector<Record*> &Groups =
    Records.getAllDerivedDefinitions("OptionGroup");
  std::vector<Record*> Opts = Records.getAllDerivedDefinitions("Option");

  emitSourceFileHeader("Option Parsing Definitions", OS);

  array_pod_sort(Opts.begin(), Opts.end(), CompareOptionRecords);
  // Generate prefix groups.
  typedef SmallVector<SmallString<2>, 2> PrefixKeyT;
  typedef std::map<PrefixKeyT, std::string> PrefixesT;
  PrefixesT Prefixes;
  Prefixes.insert(std::make_pair(PrefixKeyT(), "prefix_0"));
  unsigned CurPrefix = 0;
  for (unsigned i = 0, e = Opts.size(); i != e; ++i) {
    const Record &R = *Opts[i];
    std::vector<std::string> prf = R.getValueAsListOfStrings("Prefixes");
    PrefixKeyT prfkey(prf.begin(), prf.end());
    unsigned NewPrefix = CurPrefix + 1;
    if (Prefixes.insert(std::make_pair(prfkey, (Twine("prefix_") +
                                              Twine(NewPrefix)).str())).second)
      CurPrefix = NewPrefix;
  }

  // Dump prefixes.

  OS << "/////////\n";
  OS << "// Prefixes\n\n";
  OS << "#ifdef PREFIX\n";
  OS << "#define COMMA ,\n";
  for (PrefixesT::const_iterator I = Prefixes.begin(), E = Prefixes.end();
                                  I != E; ++I) {
    OS << "PREFIX(";

    // Prefix name.
    OS << I->second;

    // Prefix values.
    OS << ", {";
    for (PrefixKeyT::const_iterator PI = I->first.begin(),
                                    PE = I->first.end(); PI != PE; ++PI) {
      OS << "\"" << *PI << "\" COMMA ";
    }
    OS << "0})\n";
  }
  OS << "#undef COMMA\n";
  OS << "#endif\n\n";

  OS << "/////////\n";
  OS << "// Groups\n\n";
  OS << "#ifdef OPTION\n";
  for (unsigned i = 0, e = Groups.size(); i != e; ++i) {
    const Record &R = *Groups[i];

    // Start a single option entry.
    OS << "OPTION(";

    // The option prefix;
    OS << "0";

    // The option string.
    OS << ", \"" << R.getValueAsString("Name") << '"';

    // The option identifier name.
    OS  << ", "<< getOptionName(R);

    // The option kind.
    OS << ", Group";

    // The containing option group (if any).
    OS << ", ";
    if (const DefInit *DI = dyn_cast<DefInit>(R.getValueInit("Group")))
      OS << getOptionName(*DI->getDef());
    else
      OS << "INVALID";

    // The other option arguments (unused for groups).
    OS << ", INVALID, 0, 0, 0";

    // The option help text.
    if (!isa<UnsetInit>(R.getValueInit("HelpText"))) {
      OS << ",\n";
      OS << "       ";
      write_cstring(OS, R.getValueAsString("HelpText"));
    } else
      OS << ", 0";

    // The option meta-variable name (unused).
    OS << ", 0)\n";
  }
  OS << "\n";

  OS << "//////////\n";
  OS << "// Options\n\n";
  for (unsigned i = 0, e = Opts.size(); i != e; ++i) {
    const Record &R = *Opts[i];

    // Start a single option entry.
    OS << "OPTION(";

    // The option prefix;
    std::vector<std::string> prf = R.getValueAsListOfStrings("Prefixes");
    OS << Prefixes[PrefixKeyT(prf.begin(), prf.end())] << ", ";

    // The option string.
    write_cstring(OS, R.getValueAsString("Name"));

    // The option identifier name.
    OS  << ", "<< getOptionName(R);

    // The option kind.
    OS << ", " << R.getValueAsDef("Kind")->getValueAsString("Name");

    // The containing option group (if any).
    OS << ", ";
    const ListInit *GroupFlags = nullptr;
    if (const DefInit *DI = dyn_cast<DefInit>(R.getValueInit("Group"))) {
      GroupFlags = DI->getDef()->getValueAsListInit("Flags");
      OS << getOptionName(*DI->getDef());
    } else
      OS << "INVALID";

    // The option alias (if any).
    OS << ", ";
    if (const DefInit *DI = dyn_cast<DefInit>(R.getValueInit("Alias")))
      OS << getOptionName(*DI->getDef());
    else
      OS << "INVALID";

    // The option alias arguments (if any).
    // Emitted as a \0 separated list in a string, e.g. ["foo", "bar"]
    // would become "foo\0bar\0". Note that the compiler adds an implicit
    // terminating \0 at the end.
    OS << ", ";
    std::vector<std::string> AliasArgs = R.getValueAsListOfStrings("AliasArgs");
    if (AliasArgs.size() == 0) {
      OS << "0";
    } else {
      OS << "\"";
      for (size_t i = 0, e = AliasArgs.size(); i != e; ++i)
        OS << AliasArgs[i] << "\\0";
      OS << "\"";
    }

    // The option flags.
    OS << ", ";
    int NumFlags = 0;
    const ListInit *LI = R.getValueAsListInit("Flags");
    for (Init *I : *LI)
      OS << (NumFlags++ ? " | " : "")
         << cast<DefInit>(I)->getDef()->getName();
    if (GroupFlags) {
      for (Init *I : *GroupFlags)
        OS << (NumFlags++ ? " | " : "")
           << cast<DefInit>(I)->getDef()->getName();
    }
    if (NumFlags == 0)
      OS << '0';

    // The option parameter field.
    OS << ", " << R.getValueAsInt("NumArgs");

    // The option help text.
    if (!isa<UnsetInit>(R.getValueInit("HelpText"))) {
      OS << ",\n";
      OS << "       ";
      write_cstring(OS, R.getValueAsString("HelpText"));
    } else
      OS << ", 0";

    // The option meta-variable name.
    OS << ", ";
    if (!isa<UnsetInit>(R.getValueInit("MetaVarName")))
      write_cstring(OS, R.getValueAsString("MetaVarName"));
    else
      OS << "0";

    OS << ")\n";
  }
  OS << "#endif\n";
}
} // end namespace llvm
