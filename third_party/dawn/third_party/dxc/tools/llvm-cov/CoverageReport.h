//===- CoverageReport.h - Code coverage report ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class implements rendering of a code coverage report.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_COV_COVERAGEREPORT_H
#define LLVM_COV_COVERAGEREPORT_H

#include "CoverageSummaryInfo.h"
#include "CoverageViewOptions.h"

namespace llvm {

/// \brief Displays the code coverage report.
class CoverageReport {
  const CoverageViewOptions &Options;
  std::unique_ptr<coverage::CoverageMapping> Coverage;

  void render(const FileCoverageSummary &File, raw_ostream &OS);
  void render(const FunctionCoverageSummary &Function, raw_ostream &OS);

public:
  CoverageReport(const CoverageViewOptions &Options,
                 std::unique_ptr<coverage::CoverageMapping> Coverage)
      : Options(Options), Coverage(std::move(Coverage)) {}

  void renderFunctionReports(ArrayRef<std::string> Files, raw_ostream &OS);

  void renderFileReports(raw_ostream &OS);
};
}

#endif // LLVM_COV_COVERAGEREPORT_H
