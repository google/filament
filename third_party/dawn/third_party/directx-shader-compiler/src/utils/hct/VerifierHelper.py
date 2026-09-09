# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
r"""VerifierHelper.py - help with test content used with:
    ClangHLSLTests /name:VerifierTest.*

    This script will produce an HLSL file with expected-error and expected-warning
    statements corresponding to actual errors/warnings produced from ClangHLSLTests.
    The new file will be located in %TEMP%, named after the original file, but with
    the added extension '.result'.
    This can then be compared with the original file (such as varmods-syntax.hlsl)
    to see the differences in errors.  It may also be used to replace the original
    file, once the correct output behavior is verified.

    This script can also be used to do the same with fxc, adding expected errors there too.
    If there were errors/warnings/notes reported by clang, but nothing reported by fxc, an
    "fxc-pass {{}}" entry will be added.  If copied to reference, it means that you sign
    off on the difference in behavior between clang and fxc.

    In ast mode, this will find the ast subtree corresponding to a line of code preceding
    a line containing only: "/*verify-ast", and insert a stripped subtree between this marker
    and a line containing only: "*/".  This relies on clang.exe in the build directory.

    This tool expects clang.exe and ClangHLSLTests.dll to be in %HLSL_BLD_DIR%\bin\Debug.

Usage:
    VerifierHelper.py clang <testname>  - run test through ClangHLSLTests and show differences
    VerifierHelper.py fxc <testname>    - run test through fxc and show differences
    VerifierHelper.py ast <testname>    - run test through ast-dump and show differences
    VerifierHelper.py all <testname>    - run test through ClangHLSLTests, ast-dump, and fxc, then show differences
<testname> - name of verifier test as passed to "te ClangHLSLTests.dll /name:VerifierTest::<testname>":
    Example: RunVarmodsSyntax
    Can also specify * to run all tests

Environment variables - set these to ensure this tool works properly:
    HLSL_SRC_DIR        - root path of HLSLonLLVM enlistment
    HLSL_BLD_DIR        - path to projects and build output
    HLSL_FXC_PATH       - fxc.exe to use for comparison purposes
    HLSL_DIFF_TOOL      - tool to use for file comparison (optional)
"""

import os, sys, re
import subprocess

try:
    DiffTool = os.environ["HLSL_DIFF_TOOL"]
except:
    DiffTool = None
try:
    FxcPath = os.environ["HLSL_FXC_PATH"]
except:
    FxcPath = "fxc"
HlslVerifierTestCpp = os.path.expandvars(
    r"${HLSL_SRC_DIR}\tools\clang\unittests\HLSL\VerifierTest.cpp"
)
HlslDataDir = os.path.expandvars(r"${HLSL_SRC_DIR}\tools\clang\test\HLSL")
HlslBinDir = os.path.expandvars(r"${HLSL_BLD_DIR}\Debug\bin")
VerifierTests = {
    "RunArrayConstAssign": "array-const-assign.hlsl",
    "RunArrayIndexOutOfBounds": "array-index-out-of-bounds-HV-2016.hlsl",
    "RunArrayLength": "array-length.hlsl",
    "RunAttributes": "attributes.hlsl",
    "RunBadInclude": "bad-include.hlsl",
    "RunBinopDims": "binop-dims.hlsl",
    "RunBitfields": "bitfields.hlsl",
    "RunBuiltinTypesNoInheritance": "builtin-types-no-inheritance.hlsl",
    "RunCXX11Attributes": "cxx11-attributes.hlsl",
    "RunConstAssign": "const-assign.hlsl",
    "RunConstDefault": "const-default.hlsl",
    "RunConstExpr": "const-expr.hlsl",
    "RunConversionsBetweenTypeShapes": "conversions-between-type-shapes.hlsl",
    "RunConversionsBetweenTypeShapesStrictUDT": "conversions-between-type-shapes-strictudt.hlsl",
    "RunConversionsNonNumericAggregates": "conversions-non-numeric-aggregates.hlsl",
    "RunCppErrors": "cpp-errors.hlsl",
    "RunCppErrorsHV2015": "cpp-errors-hv2015.hlsl",
    "RunDerivedToBaseCasts": "derived-to-base.hlsl",
    "RunEffectsSyntax": "effects-syntax.hlsl",
    "RunEnums": "enums.hlsl",
    "RunFunctions": "functions.hlsl",
    "RunImplicitCasts": "implicit-casts.hlsl",
    "RunIncompleteArray": "incomp_array_err.hlsl",
    "RunIncompleteType": "incomplete-type.hlsl",
    "RunIndexingOperator": "indexing-operator.hlsl",
    "RunInputPatchConst": "InputPatch-const.hlsl",
    "RunIntrinsicExamples": "intrinsic-examples.hlsl",
    "RunLiterals": "literals.hlsl",
    "RunMatrixAssignments": "matrix-assignments.hlsl",
    "RunMatrixSyntax": "matrix-syntax.hlsl",
    "RunMatrixSyntaxExactPrecision": "matrix-syntax-exact-precision.hlsl",
    "RunMintypesPromotionWarnings": "mintypes-promotion-warnings.hlsl",
    "RunMoreOperators": "more-operators.hlsl",
    "RunObjectOperators": "object-operators.hlsl",
    "RunObjectTemplateDiagDeferred": "object-template-diag-deferred.hlsl",
    "RunOperatorOverloadingForNewDelete": "overloading-new-delete-errors.hlsl",
    "RunOperatorOverloadingNotDefinedBinaryOp": "use-undefined-overloaded-operator.hlsl",
    "RunPackReg": "packreg.hlsl",
    "RunRayTracings": "raytracings.hlsl",
    "RunScalarAssignments": "scalar-assignments.hlsl",
    "RunScalarAssignmentsExactPrecision": "scalar-assignments-exact-precision.hlsl",
    "RunScalarOperators": "scalar-operators.hlsl",
    "RunScalarOperatorsAssign": "scalar-operators-assign.hlsl",
    "RunScalarOperatorsAssignExactPrecision": "scalar-operators-assign-exact-precision.hlsl",
    "RunScalarOperatorsExactPrecision": "scalar-operators-exact-precision.hlsl",
    "RunSemantics": "semantics.hlsl",
    "RunSizeof": "sizeof.hlsl",
    "RunString": "string.hlsl",
    "RunStructAssignments": "struct-assignments.hlsl",
    "RunSubobjects": "subobjects-syntax.hlsl",
    "RunTemplateChecks": "template-checks.hlsl",
    "RunTemplateLiteralSubstitutionFailure": "template-literal-substitution-failure.hlsl",
    "RunTypemodsSyntax": "typemods-syntax.hlsl",
    "RunUint4Add3": "uint4_add3.hlsl",
    "RunVarmodsSyntax": "varmods-syntax.hlsl",
    "RunVectorAnd": "vector-and.hlsl",
    "RunVectorAssignments": "vector-assignments.hlsl",
    "RunVectorConditional": "vector-conditional.hlsl",
    "RunVectorOr": "vector-or.hlsl",
    "RunVectorSelect": "vector-select.hlsl",
    "RunVectorSyntax": "vector-syntax.hlsl",
    "RunVectorSyntaxExactPrecision": "vector-syntax-exact-precision.hlsl",
    "RunVectorSyntaxMix": "vector-syntax-mix.hlsl",
    "RunWave": "wave.hlsl",
    "RunWriteConstArrays": "write-const-arrays.hlsl",
    "RunAtomicsOnBitfields": "atomics-on-bitfields.hlsl",
    "RunWorkGraphs": "work-graphs.hlsl",
    "RunUnboundedResourceArrays": "invalid-unbounded-resource-arrays.hlsl",
}

# The following test(s) do not work in fxc mode:
fxcExcludedTests = [
    "RunArrayLength",
    "RunBitfields",
    "RunCppErrors",
    "RunCppErrorsHV2015",
    "RunCXX11Attributes",
    "RunConversionsBetweenTypeShapesStrictUDT",
    "RunEnums",
    "RunIncompleteType",
    "RunIntrinsicExamples",
    "RunMatrixSyntaxExactPrecision",
    "RunObjectTemplateDiagDeferred",
    "RunOperatorOverloadingForNewDelete",
    "RunOperatorOverloadingNotDefinedBinaryOp",
    "RunRayTracings",
    "RunScalarAssignmentsExactPrecision",
    "RunScalarOperatorsAssignExactPrecision",
    "RunScalarOperatorsExactPrecision",
    "RunSizeof",
    "RunSubobjects",
    "RunTemplateChecks",
    "RunTemplateLiteralSubstitutionFailure",
    "RunVectorSyntaxExactPrecision",
    "RunWave",
    "RunAtomicsOnBitfields",
    "RunWorkGraphs",
]

# rxRUN = re.compile(r'[ RUN      ] VerifierTest.(\w+)')	# gtest syntax
rxRUN = re.compile(r"StartGroup: VerifierTest::(\w+)")  # TAEF syntax
rxEndGroup = re.compile(r"EndGroup: VerifierTest::(\w+)\s+\[(\w+)\]")  # TAEF syntax
rxForProgram = re.compile(r"^for program (.*?) with errors\:$")
# rxExpected = re.compile(r"^error\: \'(\w+)\' diagnostics (expected but not seen|seen but not expected)\: $")  # gtest syntax
rxExpected = re.compile(
    r"^\'(\w+)\' diagnostics (expected but not seen|seen but not expected)\: $"
)  # TAEF syntax
rxDiagReport = re.compile(r"  (?:File (.*?) )?Line (\d+): (.*)$")

rxDiag = re.compile(r"((expected|fxc)-(error|warning|note|pass)\s*\{\{(.*?)\}\}\s*)")

rxFxcErr = re.compile(
    r"(.+)\((\d+)(?:,(\d+)(?:\-(\d+))?)?\)\: (error|warning) (.*?)\: (.*)"
)
# groups = (filename, line, colstart, colend, ew, error_code, error_message)

rxCommentStart = re.compile(r"(//|/\*)")
rxStrings = re.compile(r"(\'|\").*?((?<!\\)\1)")
rxBraces = re.compile(r"(\(|\)|\{|\}|\[|\])")
rxStatementEndOrBlockBegin = re.compile(r"(\;|\{)")
rxLineContinued = re.compile(r".*\\$")
rxVerifyArguments = re.compile(r"\s*//\s*\:FXC_VERIFY_ARGUMENTS\:\s+(.*)")

rxVerifierTestMethod = re.compile(r"TEST_F\(VerifierTest,\s*(\w+)\)\s*")
rxVerifierTestCheckFile = re.compile(r'CheckVerifiesHLSL\s*\(\s*L?\"([^"]+)"\s*\)')

rxVerifyAst = re.compile(
    r"^\s*(\/\*verify\-ast)\s*$"
)  # must start with line containing only "/*verify-ast"
rxEndVerifyAst = re.compile(r"^\s*\*\/\s*$")  # ends with line containing only "*/"
rxAstSourceLocation = re.compile(
    r"""\<(?:(?P<Invalid>\<invalid\ sloc\>) |
             (?:
                (?:(?:(?P<FromFileLine>line|\S*):(?P<FromLine>\d+):(?P<FromLineCol>\d+)) |
                   col:(?P<FromCol>\d+)
                )
                (?:,\s+
                   (?:(?:(?P<ToFileLine>line|\S*):(?P<ToLine>\d+):(?P<ToLineCol>\d+)) |
                      col:(?P<ToCol>\d+)
                   )
                )?
             )
          )\>""",
    re.VERBOSE,
)
rxAstHexAddress = re.compile(r"\b(0x[0-9a-f]+) ?")
rxAstNode = re.compile(r"((?:\<\<\<NULL\>\>\>)|(?:\w+))\s*(.*)")
# matches ignored portion of line for first AST node in subgraph to match
rxAstIgnoredIndent = re.compile(r"^(\s+|\||\`|\-)*")

# The purpose of StripComments and CountBraces is to be used when commenting lines of code out to allow
# Fxc testing to continue even when it doesn't recover as well as clang.  Some error lines are on the
# beginning of a function, where commenting just that line will comment out the beginning of the function
# block, but not the body or end of the block, producing invalid syntax.  Here's an example:
#   void foo(error is here) { /* expected-error {{some expected clang error}} */
#     return;
#   }
# If the first line is commented without the rest of the function, it will be incorrect code.
# So the intent is to detect when the line being commented out results in an unbalanced brace matching.
# Then these functions will be used to comment additional lines until the braces match again.
# It's simple and won't handle the general case, but should handle the cases in the test files, and if
# not, the tests should be easily modifyable to work with it.
# This still does not handle preprocessor directives, or escaped characters (like line ends or escaped
# quotes), or other cases that a real parser would handle.


def StripComments(line, multiline_comment_continued=False):
    "Remove comments from line, returns stripped line and multiline_comment_continued if a multiline comment continues beyond the line"
    if multiline_comment_continued:
        # in multiline comment, only look for end of that
        idx = line.find("*/")
        if idx < 0:
            return "", True
        return StripComments(line[idx + 2 :])
    # look for start of multiline comment or eol comment:
    m = rxCommentStart.search(line)
    if m:
        if m.group(1) == "/*":
            line_end, multiline_comment_continued = StripComments(
                line[m.end(1) :], True
            )
            return line[: m.start(1)] + line_end, multiline_comment_continued
        elif m.group(1) == "//":
            return line[: m.start(1)], False
    return line, False


def CountBraces(line, bracestacks):
    m = rxStrings.search(line)
    if m:
        CountBraces(line[: m.start(1)], bracestacks)
        CountBraces(line[m.end(2) :], bracestacks)
        return
    for b in rxBraces.findall(line):
        if b in "()":
            bracestacks["()"] = bracestacks.get("()", 0) + ((b == "(") and 1 or -1)
        elif b in "{}":
            bracestacks["{}"] = bracestacks.get("{}", 0) + ((b == "{") and 1 or -1)
        elif b in "[]":
            bracestacks["[]"] = bracestacks.get("[]", 0) + ((b == "[") and 1 or -1)


def ProcessStatementOrBlock(lines, start, fn_process):
    num = 0
    # statement_continued initialized with whether line has non-whitespace content
    statement_continued = not not StripComments(lines[start], False)[0].strip()
    # Assumes start of line is not inside multiline comment
    multiline_comment_continued = False
    bracestacks = {}
    while start + num < len(lines):
        line = lines[start + num]
        lines[start + num] = fn_process(line)
        num += 1
        line, multiline_comment_continued = StripComments(
            line, multiline_comment_continued
        )
        CountBraces(line, bracestacks)
        if statement_continued and not rxStatementEndOrBlockBegin.search(line):
            continue
        statement_continued = False
        if rxLineContinued.match(line):
            continue
        if (
            bracestacks.get("{}", 0) < 1
            and bracestacks.get("()", 0) < 1
            and bracestacks.get("[]", 0) < 1
        ):
            break
    return num


def CommentStatementOrBlock(lines, start):
    def fn_process(line):
        return "//  " + line

    return ProcessStatementOrBlock(lines, start, fn_process)


def ParseVerifierTestCpp():
    "Returns dictionary mapping Run* test name to hlsl filename by parsing VerifierTest.cpp"
    tests = {}
    FoundTest = None

    def fn_null(line):
        return line

    def fn_process(line):
        searching = FoundTest is not None
        if searching:
            m = rxVerifierTestCheckFile.search(line)
            if m:
                tests[FoundTest] = m.group(1)
                searching = False
        return line

    with open(HlslVerifierTestCpp, "rt") as f:
        lines = f.readlines()
        start = 0
        while start < len(lines):
            m = rxVerifierTestMethod.search(lines[start])
            if m:
                FoundTest = m.group(1)
                start += ProcessStatementOrBlock(lines, start, fn_process)
                if FoundTest not in tests:
                    print("Could not parse file for test %s" % FoundTest)
                FoundTest = None
            else:
                start += ProcessStatementOrBlock(lines, start, fn_null)
    return tests


class SourceLocation(object):
    def __init__(self, line=None, **kwargs):
        if not kwargs:
            self.Invalid = "<invalid sloc>"
            return
        for key, value in kwargs.items():
            try:
                value = int(value)
            except:
                pass
            setattr(self, key, value)
        if line and not self.FromLine:
            self.FromLine = line
        self.FromCol = self.FromCol or self.FromLineCol
        self.ToCol = self.ToCol or self.ToLineCol

    def Offset(self, offset):
        "Offset From/To Lines by specified value"
        if self.Invalid:
            return
        if self.FromLine:
            self.FromLine = self.FromLine + offset
        if self.ToLine:
            self.ToLine = self.ToLine + offset

    def ToStringAtLine(self, line):
        "convert to string relative to specified line"
        if self.Invalid:
            sloc = self.Invalid
        else:
            if self.FromLine and line != self.FromLine:
                sloc = "line:%d:%d" % (self.FromLine, self.FromCol)
                line = self.FromLine
            else:
                sloc = "col:%d" % self.FromCol
            if self.ToCol:
                if self.ToLine and line != self.ToLine:
                    sloc += ", line:%d:%d" % (self.ToLine, self.ToCol)
                else:
                    sloc += ", col:%d" % self.ToCol
        return "<" + sloc + ">"


class AstNode(object):
    def __init__(self, name, sloc, prefix, text, indent=""):
        self.name, self.sloc, self.prefix, self.text, self.indent = (
            name,
            sloc,
            prefix,
            text,
            indent,
        )
        self.children = []

    def ToStringAtLine(self, line):
        "convert to string relative to specified line"
        if self.name == "<<<NULL>>>":
            return self.name
        return (
            "%s %s%s %s"
            % (self.name, self.prefix, self.sloc.ToStringAtLine(line), self.text)
        ).strip()


def WalkAstChildren(ast_root):
    "yield each child node in the ast tree in depth-first order"
    for node in ast_root.children:
        yield node
        for child in WalkAstChildren(node):
            yield child


def WriteAstSubtree(ast_root, line, indent=""):
    output = []
    output.append(indent + ast_root.ToStringAtLine(line))
    if not ast_root.sloc.Invalid and ast_root.sloc.FromLine:
        line = ast_root.sloc.FromLine
    root_indent_len = len(ast_root.indent)
    for child in WalkAstChildren(ast_root):
        output.append(
            indent + child.indent[root_indent_len:] + child.ToStringAtLine(line)
        )
        if not child.sloc.Invalid and child.sloc.FromLine:
            line = child.sloc.FromLine
    return output


def FindAstNodesByLine(ast_root, line):
    nodes = []
    if not ast_root.sloc.Invalid and ast_root.sloc.FromLine == line:
        return [ast_root]
    if (
        not ast_root.sloc.Invalid
        and ast_root.sloc.ToLine
        and ast_root.sloc.ToLine < line
    ):
        return []
    for child in ast_root.children:
        sub_nodes = FindAstNodesByLine(child, line)
        if sub_nodes:
            nodes += sub_nodes
    return nodes


def ParseAst(astlines):
    cur_line = 0  # current source line
    root_node = None
    ast_stack = (
        []
    )  # stack of nodes and column numbers so we can pop the right number of nodes up the stack
    i = 0  # ast line index

    def push(node, col):
        if ast_stack:
            cur_node, prior_col = ast_stack[-1]
            cur_node.children.append(node)
        ast_stack.append((node, col))

    def popto(col):
        cur_node, prior_col = ast_stack[-1]
        while ast_stack and col <= prior_col:
            ast_stack.pop()
            cur_node, prior_col = ast_stack[-1]
        assert ast_stack

    def parsenode(text, indent):
        m = rxAstNode.match(text)
        if m:
            name = m.group(1)
            text = text[m.end(1) :].strip()
        else:
            print("rxAstNode match failed on:\n  %s" % text)
            return AstNode("ast-parse-failed", SourceLocation(), "", "", indent)
        text = rxAstHexAddress.sub("", text).strip()
        m = rxAstSourceLocation.search(text)
        if m:
            prefix = text[: m.start()]
            sloc = SourceLocation(cur_line, **m.groupdict())
            text = text[m.end() :].strip()
        else:
            prefix = ""
            sloc = SourceLocation()
        return AstNode(name, sloc, prefix, text, indent)

    # Look for TranslationUnitDecl and start from there
    while i < len(astlines):
        text = astlines[i]
        if text.startswith("TranslationUnitDecl"):
            root_node = parsenode(text, "")
            push(root_node, 0)
            break
        i += 1
    i += 1

    # gather ast nodes
    while i < len(astlines):
        line = astlines[i]

        # get starting column and update stack
        m = rxAstIgnoredIndent.match(line)
        indent = ""
        col = 0
        if m:
            indent = m.group(0)
            col = m.end()
        if col == 0:
            break  # at this point we should be done parsing the translation unit!

        popto(col)

        # parse and add the node
        node = parsenode(line[col:], indent)
        if not node:
            print("error parsing line %d:\n%s" % (i + 1, line))
            assert False
        push(node, col)

        # update current source line
        sloc = node.sloc
        if not sloc.Invalid and sloc.FromLine:
            cur_line = sloc.FromLine

        i += 1

    return root_node


class File(object):
    def __init__(self, filename):
        self.filename = filename
        self.expected = (
            {}
        )  # {line_num: [('error' or 'warning', 'error or warning message'), ...], ...}
        self.unexpected = (
            {}
        )  # {line_num: [('error' or 'warning', 'error or warning message'), ...], ...}
        self.last_diag_col = None

    def AddExpected(self, line_num, ew, message):
        self.expected.setdefault(line_num, []).append((ew, message))

    def AddUnexpected(self, line_num, ew, message):
        self.unexpected.setdefault(line_num, []).append((ew, message))

    def MatchDiags(self, line, diags=[], prefix="expected", matchall=False):
        diags = diags[:]
        diag_col = None
        matches = []
        for m in rxDiag.finditer(line):
            if diag_col is None:
                diag_col = m.start()
                self.last_diag_col = diag_col
            if m.group(2) == prefix:
                pattern = m.groups()[2:4]
                for idx, (ew, message) in enumerate(diags):
                    if pattern == (ew, message):
                        matches.append(m)
                        break
                else:
                    if matchall:
                        matches.append(m)
                    continue
                del diags[idx]
        return sorted(matches, key=lambda m: m.start()), diags, diag_col

    def RemoveDiags(self, line, diags, prefix="expected", removeall=False):
        """Removes expected-* diags from line, returns result_line, remaining_diags, diag_col
        Where, result_line is the line without the matching diagnostics,
            remaining is the list of diags not found on the line,
            diag_col is the column of the first diagnostic found on the line.
        """
        matches, diags, diag_col = self.MatchDiags(line, diags, prefix, removeall)
        for m in reversed(matches):
            line = line[: m.start()] + line[m.end() :]
        return line, diags, diag_col

    def AddDiags(self, line, diags, diag_col=None, prefix="expected"):
        "Adds expected-* diags to line."
        if diags:
            if diag_col is None:
                if self.last_diag_col is not None and self.last_diag_col - 3 > len(
                    line
                ):
                    diag_col = self.last_diag_col
                else:
                    diag_col = max(
                        len(line) + 7, 63
                    )  # 4 spaces + '/* ' or at column 63, whichever is greater
                line = line + (" " * ((diag_col - 3) - len(line))) + "/* */"
            for ew, message in reversed(diags):
                line = (
                    line[:diag_col]
                    + ("%s-%s {{%s}} " % (prefix, ew, message))
                    + line[diag_col:]
                )
        return line.rstrip()

    def SortDiags(self, line):
        matches = list(rxDiag.finditer(line))
        if matches:
            for m in sorted(matches, key=lambda m: m.start(), reverse=True):
                line = line[: m.start()] + line[m.end() :]
                diag_col = m.start()
            for m in sorted(matches, key=lambda m: m.groups()[1:], reverse=True):
                line = (
                    line[:diag_col]
                    + ("%s-%s {{%s}} " % m.groups()[1:])
                    + line[diag_col:]
                )
        return line.rstrip()

    def OutputResult(self):
        temp_filename = os.path.expandvars(
            r"${TEMP}\%s" % os.path.split(self.filename)[1]
        )
        with open(self.filename, "rt") as fin:
            with open(temp_filename + ".result", "wt") as fout:
                line_num = 0
                for line in fin.readlines():
                    if line[-1] == "\n":
                        line = line[:-1]
                    line_num += 1
                    line, expected, diag_col = self.RemoveDiags(
                        line, self.expected.get(line_num, [])
                    )
                    for ew, message in expected:
                        print(
                            "Error: Line %d: Could not find: expected-%s {{%s}}!!"
                            % (line_num, ew, message)
                        )
                    line = self.AddDiags(
                        line, self.unexpected.get(line_num, []), diag_col
                    )
                    line = self.SortDiags(line)
                    fout.write(line + "\n")

    def TryFxc(self, result_filename=None):
        temp_filename = os.path.expandvars(
            r"${TEMP}\%s" % os.path.split(self.filename)[1]
        )
        if result_filename is None:
            result_filename = temp_filename + ".fxc"
        inlines = []
        with open(self.filename, "rt") as fin:
            for line in fin.readlines():
                if line[-1] == "\n":
                    line = line[:-1]
                inlines.append(line)
        verify_arguments = None
        for line in inlines:
            m = rxVerifyArguments.search(line)
            if m:
                verify_arguments = m.group(1)
                print("Found :FXC_VERIFY_ARGUMENTS: %s" % verify_arguments)
                break

        # result will hold the final result after adding fxc error messages
        # initialize it by removing all the expected diagnostics
        result = [(line, None, False) for line in inlines]
        for n, (line, diag_col, expected) in enumerate(result):
            line, diags, diag_col = self.RemoveDiags(
                line, [], prefix="fxc", removeall=True
            )
            matches, diags, diag_col2 = self.MatchDiags(
                line, [], prefix="expected", matchall=True
            )
            if matches:
                expected = True
            ##                if diag_col is None:
            ##                    diag_col = diag_col2
            ##                elif diag_col2 < diag_col:
            ##                    diag_col = diag_col2
            result[n] = (line, diag_col, expected)

        # commented holds the version that gets progressively commented as fxc reports errors
        commented = inlines[:]

        # diags_by_line is a dictionary of a set of errors and warnings keyed off line_num
        diags_by_line = {}
        while True:
            with open(temp_filename + ".fxc_temp", "wt") as fout:
                fout.write("\n".join(commented))
                if verify_arguments is None:
                    fout.write("\n[numthreads(1,1,1)] void _test_main() {  }\n")
            if verify_arguments is None:
                args = "/E _test_main /T cs_5_1".split()
            else:
                args = verify_arguments.split()
            fxcres = subprocess.run(
                [
                    "%s" % FxcPath,
                    temp_filename + ".fxc_temp",
                    *args,
                    "/nologo",
                    "/DVERIFY_FXC=1",
                    "/Fo",
                    temp_filename + ".fxo",
                    "/Fe",
                    temp_filename + ".err",
                ],
                capture_output=True,
                text=True,
            )
            with open(temp_filename + ".err", "rt") as f:
                errors = [m for m in map(rxFxcErr.match, f.readlines()) if m]
            errors = sorted(errors, key=lambda m: int(m.group(2)))
            first_error = None
            for m in errors:
                line_num = int(m.group(2))
                if not first_error and m.group(5) == "error":
                    first_error = line_num
                elif first_error and line_num > first_error:
                    break
                diags_by_line.setdefault(line_num, set()).add(
                    (m.group(5), m.group(6) + ": " + m.group(7))
                )
            if first_error and first_error <= len(commented):
                CommentStatementOrBlock(commented, first_error - 1)
            else:
                break

        # Add diagnostic messages from fxc to result:
        self.last_diag_col = None
        for i, (line, diag_col, expected) in enumerate(result):
            line_num = i + 1
            if diag_col:
                self.last_diag_col = diag_col
            diags = diags_by_line.get(line_num, set())
            if not diags:
                if expected:
                    diags.add(("pass", ""))
                else:
                    continue
            diags = sorted(list(diags))
            line = self.SortDiags(self.AddDiags(line, diags, diag_col, prefix="fxc"))
            result[i] = line, diag_col, expected

        with open(result_filename, "wt") as f:
            f.write("\n".join(map((lambda res: res[0]), result)))

    def TryAst(self, result_filename=None):
        temp_filename = os.path.expandvars(
            r"${TEMP}\%s" % os.path.split(self.filename)[1]
        )
        if result_filename is None:
            result_filename = temp_filename + ".ast"
        try:
            os.unlink(result_filename)
        except:
            pass
        result = subprocess.run(
            [
                "%s\\dxc.exe" % HlslBinDir,
                "-ast-dump",
                "-E",
                "main",
                "-T",
                "ps_5_0",
                self.filename,
            ],
            capture_output=True,
            text=True,
        )
        # dxc dumps ast even if there exists any syntax error. If there is any error, dxc returns some nonzero errorcode.
        if not result.stdout:
            with open("%s.log" % temp_filename, "wt") as f:
                f.write(result.stderr)
            print('ast-dump failed, see log:\n  "%s.log"' % (temp_filename))
            return
        try:
            ast_root = ParseAst(result.stdout.splitlines())
        except:
            with open("%s" % result_filename, "wt") as f:
                f.write(result.stdout)
            print('ParseAst failed on "%s"' % (result_filename))
            raise
        inlines = []
        with open(self.filename, "rt") as fin:
            for line in fin.readlines():
                if line[-1] == "\n":
                    line = line[:-1]
                inlines.append(line)
        outlines = []
        i = 0
        while i < len(inlines):
            line = inlines[i]
            outlines.append(line)
            m = rxVerifyAst.match(line)
            if m:
                indent = line[: m.start(1)] + "  "
                # at this point i is the ONE based source line number
                # (since it's one past the line we want to verify in zero based index)
                ast_nodes = FindAstNodesByLine(ast_root, i)
                if not ast_nodes:
                    outlines += [indent + "No matching AST found for line!"]
                else:
                    for ast in ast_nodes:
                        outlines += WriteAstSubtree(ast, i, indent)
                while i + 1 < len(inlines) and not rxEndVerifyAst.match(inlines[i + 1]):
                    i += 1
            i += 1

        with open(result_filename, "wt") as f:
            f.write("\n".join(outlines))


def ProcessVerifierOutput(lines):
    files = {}
    cur_filename = None
    cur_test = None
    state = "WaitingForFile"
    ew = ""
    expected = None
    for line in lines:
        if not line:
            continue
        if line[-1] == "\n":
            line = line[:-1]
        m = rxRUN.match(line)
        if m:
            cur_test = m.group(1)
        m = rxForProgram.match(line)
        if m:
            cur_filename = m.group(1)
            files[cur_filename] = File(cur_filename)
            state = "WaitingForCategory"
            continue
        if state == "WaitingForFile":
            m = rxEndGroup.match(line)
            if m and m.group(2) == "Failed":
                # This usually happens when compiler crashes
                print(
                    "Fatal Error: test %s failed without verifier results." % cur_test
                )
        if state == "WaitingForCategory" or state == "ReadingErrors":
            m = rxExpected.match(line)
            if m:
                ew = m.group(1)
                expected = m.group(2) == "expected but not seen"
                state = "ReadingErrors"
                continue
        if state == "ReadingErrors":
            m = rxDiagReport.match(line)
            if m:
                line_num = int(m.group(2))
                if expected:
                    files[cur_filename].AddExpected(line_num, ew, m.group(3))
                else:
                    files[cur_filename].AddUnexpected(line_num, ew, m.group(3))
                continue
    for f in files.values():
        f.OutputResult()
    return files


def maybe_compare(filename1, filename2):
    with open(filename1, "rt") as fbefore:
        with open(filename2, "rt") as fafter:
            before = fbefore.read()
            after = fafter.read()
    if before.strip() != after.strip():
        print(
            "Differences found.  Compare:\n  %s\nwith:\n  %s" % (filename1, filename2)
        )
        if DiffTool:
            subprocess.Popen(
                [DiffTool, filename1, filename2],
                creationflags=subprocess.DETACHED_PROCESS,
            )
        return True
    return False


def PrintUsage():
    print(__doc__)
    print("Available tests and corresponding files:")
    tests = sorted(VerifierTests.keys())
    width = len(max(tests, key=len))
    for name in tests:
        print(("    %%-%ds  %%s" % width) % (name, VerifierTests[name]))
    print("Tests incompatible with fxc mode:")
    for name in fxcExcludedTests:
        print("    %s" % name)


def RunVerifierTest(test, HlslDataDir=HlslDataDir):
    import codecs

    temp_filename = os.path.expandvars(r"${TEMP}\VerifierHelper_temp.txt")
    cmd = (
        'te %s\\ClangHLSLTests.dll /p:"HlslDataDir=%s" /name:VerifierTest::%s > %s'
        % (HlslBinDir, HlslDataDir, test, temp_filename)
    )
    print(cmd)
    os.system(cmd)  # TAEF test
    # TAEF outputs unicode, so read as binary and convert:
    with open(temp_filename, "rb") as f:
        return (
            codecs.decode(f.read(), "UTF-16")
            .replace("\x7f", "")
            .replace("\r\n", "\n")
            .splitlines()
        )


def main(*args):
    global VerifierTests
    try:
        VerifierTests = ParseVerifierTestCpp()
    except:
        print("Unable to parse tests from VerifierTest.cpp; using defaults")
    if len(args) < 1 or (
        args[0][0] in "-/" and args[0][1:].lower() in ("h", "?", "help")
    ):
        PrintUsage()
        return -1
    mode = args[0]
    if mode == "fxc":
        allFxcTests = sorted(
            filter(lambda key: key not in fxcExcludedTests, VerifierTests.keys())
        )
        if args[1] == "*":
            tests = allFxcTests
        else:
            if args[1] not in allFxcTests:
                PrintUsage()
                return -1
            tests = [args[1]]
        differences = False
        for test in tests:
            print("---- %s ----" % test)
            filename = os.path.join(HlslDataDir, VerifierTests[test])
            result_filename = os.path.expandvars(
                r"${TEMP}\%s.fxc" % os.path.split(filename)[1]
            )
            File(filename).TryFxc()
            differences = maybe_compare(filename, result_filename) or differences
        if not differences:
            print("No differences found!")
    elif mode == "clang":
        if args[1] != "*" and args[1] not in VerifierTests:
            PrintUsage()
            return -1
        files = ProcessVerifierOutput(RunVerifierTest(args[1]))
        differences = False
        if files:
            for f in files.values():
                if f.expected or f.unexpected:
                    result_filename = os.path.expandvars(
                        r"${TEMP}\%s.result" % os.path.split(f.filename)[1]
                    )
                    differences = (
                        maybe_compare(f.filename, result_filename) or differences
                    )
        if not differences:
            print("No differences found!")
    elif mode == "ast":
        allAstTests = sorted(VerifierTests.keys())
        if args[1] == "*":
            tests = allAstTests
        else:
            if args[1] not in allAstTests:
                PrintUsage()
                return -1
            tests = [args[1]]
        differences = False
        for test in tests:
            print("---- %s ----" % test)
            filename = os.path.join(HlslDataDir, VerifierTests[test])
            result_filename = os.path.expandvars(
                r"${TEMP}\%s.ast" % os.path.split(filename)[1]
            )
            File(filename).TryAst()
            differences = maybe_compare(filename, result_filename) or differences
        if not differences:
            print("No differences found!")
    elif mode == "all":
        allTests = sorted(VerifierTests.keys())
        if args[1] == "*":
            tests = allTests
        else:
            if args[1] not in allTests:
                PrintUsage()
                return -1
            tests = [args[1]]

        # Do clang verifier tests, updating source file paths for changed files:
        sourceFiles = dict(
            [
                (VerifierTests[test], os.path.join(HlslDataDir, VerifierTests[test]))
                for test in tests
            ]
        )
        files = ProcessVerifierOutput(RunVerifierTest(args[1]))
        if files:
            for f in files.values():
                if f.expected or f.unexpected:
                    name = os.path.split(f.filename)[1]
                    sourceFiles[name] = os.path.expandvars(r"${TEMP}\%s.result" % name)

        # update verify-ast blocks:
        for name, sourceFile in sourceFiles.items():
            result_filename = os.path.expandvars(r"${TEMP}\%s.ast" % name)
            File(sourceFile).TryAst(result_filename)
            sourceFiles[name] = result_filename

        # now do fxc verification and final comparison
        differences = False
        fxcExcludedFiles = [VerifierTests[test] for test in fxcExcludedTests]
        width = len(max(tests, key=len))
        for test in tests:
            name = VerifierTests[test]
            sourceFile = sourceFiles[name]
            print(("Test %%-%ds - %%s" % width) % (test, name))
            result_filename = os.path.expandvars(r"${TEMP}\%s.fxc" % name)
            if name not in fxcExcludedFiles:
                File(sourceFile).TryFxc(result_filename)
                sourceFiles[name] = result_filename
            differences = (
                maybe_compare(os.path.join(HlslDataDir, name), sourceFiles[name])
                or differences
            )
        if not differences:
            print("No differences found!")
    else:
        PrintUsage()
        return -1

    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
