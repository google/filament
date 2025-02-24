//===- unittest/IceELFSectionTest.cpp - ELF Section unit tests ------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <algorithm>

#include "gtest/gtest.h"

#include "IceDefs.h"
#include "IceELFSection.h"

#include "llvm/Support/raw_os_ostream.h"

namespace Ice {
namespace {

// Test string table layout for various permutations. Test that pop,
// lollipop, and lipop are able to share data, while the other strings do not.
void CheckStringTablePermLayout(const ELFStringTableSection &Strtab) {
  size_t pop_index = Strtab.getIndex("pop");
  size_t pop_size = std::string("pop").size();
  size_t lollipop_index = Strtab.getIndex("lollipop");
  size_t lollipop_size = std::string("lollipop").size();
  size_t lipop_index = Strtab.getIndex("lipop");
  size_t lipop_size = std::string("lipop").size();
  size_t pops_index = Strtab.getIndex("pops");
  size_t pops_size = std::string("pops").size();
  size_t unpop_index = Strtab.getIndex("unpop");
  size_t unpop_size = std::string("unpop").size();
  size_t popular_index = Strtab.getIndex("popular");
  size_t popular_size = std::string("popular").size();
  size_t strtab_index = Strtab.getIndex(".strtab");
  size_t strtab_size = std::string(".strtab").size();
  size_t shstrtab_index = Strtab.getIndex(".shstrtab");
  size_t shstrtab_size = std::string(".shstrtab").size();
  size_t symtab_index = Strtab.getIndex(".symtab");
  size_t symtab_size = std::string(".symtab").size();

  // Check that some sharing exists.
  EXPECT_EQ(pop_index, lollipop_index + (lollipop_size - pop_size));
  EXPECT_EQ(lipop_index, lollipop_index + (lollipop_size - lipop_size));

  // Check that .strtab does not share with .shstrtab (the dot throws it off).
  EXPECT_NE(strtab_index, shstrtab_index + (shstrtab_size - strtab_size));

  // Check contents make sense.
  EXPECT_EQ(Strtab.getSectionData().slice(pop_index, pop_index + pop_size),
            llvm::StringRef("pop"));
  EXPECT_EQ(Strtab.getSectionData().slice(lollipop_index,
                                          lollipop_index + lollipop_size),
            llvm::StringRef("lollipop"));
  EXPECT_EQ(Strtab.getSectionData().slice(pops_index, pops_index + pops_size),
            llvm::StringRef("pops"));
  EXPECT_EQ(
      Strtab.getSectionData().slice(unpop_index, unpop_index + unpop_size),
      llvm::StringRef("unpop"));
  EXPECT_EQ(Strtab.getSectionData().slice(popular_index,
                                          popular_index + popular_size),
            llvm::StringRef("popular"));
  EXPECT_EQ(
      Strtab.getSectionData().slice(strtab_index, strtab_index + strtab_size),
      llvm::StringRef(".strtab"));
  EXPECT_EQ(Strtab.getSectionData().slice(shstrtab_index,
                                          shstrtab_index + shstrtab_size),
            llvm::StringRef(".shstrtab"));
  EXPECT_EQ(
      Strtab.getSectionData().slice(symtab_index, symtab_index + symtab_size),
      llvm::StringRef(".symtab"));
}

// Test that the order in which strings are added doesn't matter.
TEST(IceELFSectionTest, StringTableBuilderPermSeveral) {
  std::vector<std::string> Strings;
  Strings.push_back("pop");
  Strings.push_back("lollipop");
  Strings.push_back("lipop");
  Strings.push_back("pops");
  Strings.push_back("unpop");
  Strings.push_back("popular");
  Strings.push_back("a");
  Strings.push_back("z");
  Strings.push_back("foo");
  Strings.push_back("bar");
  Strings.push_back(".text");
  Strings.push_back(".symtab");
  Strings.push_back(".strtab");
  Strings.push_back(".shstrtab");
  Strings.push_back("_start");
  const SizeT NumTests = 128;
  const uint64_t RandomSeed = 12345; // arbitrary value for now
  RandomNumberGenerator R(RandomSeed);
  RandomNumberGeneratorWrapper RNG(R);
  for (SizeT i = 0; i < NumTests; ++i) {
    auto Str = std::unique_ptr<Ostream>(new llvm::raw_os_ostream(std::cout));
    RandomShuffle(Strings.begin(), Strings.end(), RNG);
    ELFStringTableSection Strtab(".strtab", SHT_STRTAB, 0, 1, 0);
    for (auto &S : Strings) {
      Strtab.add(S);
    }
    Strtab.doLayout();
    CheckStringTablePermLayout(Strtab);
  }
}

// Test that adding duplicate strings is fine.
TEST(IceELFSectionTest, StringTableBuilderDuplicates) {
  auto Str = std::unique_ptr<Ostream>(new llvm::raw_os_ostream(std::cout));
  ELFStringTableSection Strtab(".strtab", SHT_STRTAB, 0, 1, 0);
  Strtab.add("unpop");
  Strtab.add("pop");
  Strtab.add("lollipop");
  Strtab.add("a");
  Strtab.add("popular");
  Strtab.add("pops");
  Strtab.add("lipop");
  Strtab.add(".strtab");
  Strtab.add(".shstrtab");
  Strtab.add(".symtab");

  Strtab.add(".symtab");
  Strtab.add(".shstrtab");
  Strtab.add(".strtab");
  Strtab.add("lipop");
  Strtab.add("pops");
  Strtab.add("popular");
  Strtab.add("a");
  Strtab.add("lollipop");
  Strtab.add("pop");
  Strtab.add("unpop");

  Strtab.doLayout();
  CheckStringTablePermLayout(Strtab);
}

} // end of anonymous namespace
} // end of namespace Ice
