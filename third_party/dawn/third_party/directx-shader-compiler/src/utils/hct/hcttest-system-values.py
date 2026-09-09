# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
"""hcttest-system-values.py - Test all system values with each signature point through fxc.
Builds csv tables for the results for each shader model from 4.0 through 5.1.
"""
import re

SignaturePoints = [
    # sig point, stage, tessfactors already present
    ("VSIn", "vs", False),
    ("VSOut", "vs", False),
    ("PCIn", "hs", False),
    ("HSIn", "hs", False),
    ("HSCPIn", "hs", False),
    ("HSCPOut", "hs", False),
    ("PCOut", "hs", True),
    ("DSIn", "ds", True),
    ("DSCPIn", "ds", False),
    ("DSOut", "ds", False),
    ("GSVIn", "gs", False),
    ("GSIn", "gs", False),
    ("GSOut", "gs", False),
    ("PSIn", "ps", False),
    ("PSOut", "ps", False),
    ("CSIn", "cs", False),
]

SysValues = """
  VertexID
  InstanceID
  Position
  RenderTargetArrayIndex
  ViewportArrayIndex
  ClipDistance
  CullDistance
  OutputControlPointID
  DomainLocation
  PrimitiveID
  GSInstanceID
  SampleIndex
  IsFrontFace
  Coverage
  InnerCoverage
  Target
  Depth
  DepthLessEqual
  DepthGreaterEqual
  StencilRef
  DispatchThreadID
  GroupID
  GroupIndex
  GroupThreadID
  TessFactor
  InsideTessFactor
""".split()


def run(cmd, output_filename=None):
    import os

    print(cmd)
    if output_filename:
        if os.path.isfile(output_filename):
            os.unlink(output_filename)
        with file(output_filename, "wt") as f:
            f.write("%s\n\n" % cmd)
        ret = os.system(cmd + ' >> "%s" 2>&1' % output_filename)
        output = ""
        if os.path.isfile(output_filename):
            with open(output_filename, "rt") as f:
                output = f.read()
        return ret, output
    else:
        ret = os.system(cmd)
        return ret, None


sig_examples = """
// Patch Constant signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// Arb                      0   x           0     NONE   float   x
// SV_TessFactor            0   x           1 QUADEDGE   float   x
// SV_TessFactor            1   x           2 QUADEDGE   float   x
// SV_TessFactor            2   x           3 QUADEDGE   float   x
// SV_TessFactor            3   x           4 QUADEDGE   float   x
// SV_InsideTessFactor      0   x           5  QUADINT   float   x
// SV_InsideTessFactor      1   x           6  QUADINT   float   x
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// Arb                      0   x           0     NONE   float
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// Arb                      0   x           0     NONE   float   x
"""

rxSigBegin = re.compile(r"^// (Input|Output|Patch Constant) signature:\s*$")
rxSigElementsBegin = re.compile(
    r"^// -------------------- ----- ------ -------- -------- ------- ------\s*$"
)

# // SV_Target                1   xyzw        1   TARGET   float   xyzw
rxSigElement = re.compile(
    r"^// ([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s*([^\s]+)?\s*$"
)
# Name, Index, Mask, Register, SysValue, Format, Used = m.groups()


# For now, ParseSigs just looks for a matching semantic name and reports whether it uses
# a separate register (unpacked) and whether it's treated as arbitrary.
def ParseSigs(text, semantic):
    "return separate_reg, as_arb for matching semantic"
    it = iter(text.splitlines())
    sigtype = None
    try:
        while True:
            line = it.next()
            m = rxSigBegin.match(line)
            if m:
                sigtype = m.group(1)
                continue
            m = rxSigElementsBegin.match(line)
            if m:
                while True:
                    line = it.next()
                    m = rxSigElement.match(line)
                    if m:
                        Name, Index, Mask, Register, SysValue, Format, Used = m.groups()
                        if Name.lower() == semantic.lower():
                            try:
                                regnum = int(Register)
                                reg = False
                            except:
                                reg = True
                            arb = SysValue == "NONE"
                            return reg, arb
    except StopIteration:
        pass
    return None


# Internal error or validation error:
rxInternalError = re.compile(r"^(internal error:.*|error X8000:.*)$")

# error X4502: invalid ps input semantic 'Foo'
# error X4502: SV_Coverage input not supported on ps_4_0
# error X4502: SV_SampleIndex isn't supported on ps_4_0
# error X3514: SV_GSInstanceID is an invalid input semantic for geometry shader primitives, it must be its own parameter.
# error X3514: 'GSMain': input parameter 'tfactor' must have a geometry specifier
# also errors for unsupported shader models
# error X3660: cs_4_0 does not support interlocked operations
rxSemanticErrors = [
    re.compile(r".*?\.hlsl.*?: error X4502: invalid .*? semantic '(\w+)'"),
    re.compile(r".*?\.hlsl.*?: error X4502: (\w+) .*? supported on \w+"),
    re.compile(
        r".*?\.hlsl.*?: error X3514: (\w+) is an invalid input semantic for geometry shader primitives, it must be its own parameter\."
    ),
    re.compile(
        r".*?\.hlsl.*?: error X3514: 'GSMain': input parameter '\w+' must have a geometry specifier"
    ),
]


def map_gen(fn, *sequences):
    "generator style map"
    iters = map(iter, sequences)
    while True:
        yield fn(*map(next, iters))


def firstTrue(iterable):
    "returns first non-False element, or None."
    for it in iterable:
        if it:
            return it


def ParseSVError(text, semantic):
    "return true if error is about illegal use of matching semantic"
    for line in text.splitlines():
        m = firstTrue(map_gen(lambda rx: rx.match(line), rxSemanticErrors))
        if m:
            if len(m.groups()) < 1 or m.group(1).lower() == semantic.lower():
                return True
        else:
            m = rxInternalError.match(line)
            if m:
                print("#### Internal error detected!")
                print(m.group(1))
                return "InternalError"
    return False


# TODO: Fill in the correct error pattern
# error X4576: Non system-generated input signature parameter (Arb) cannot appear after a system generated value.
rxMustBeLastError = re.compile(
    r".*?\.hlsl.*?: error X4576: Non system-generated input signature parameter \(\w+\) cannot appear after a system generated value."
)


def ParseSGVError(text, semantic):
    "return true if error is about matching semantic having to be declared last"
    for line in text.splitlines():
        m = rxMustBeLastError.match(line)
        if m:
            return True
        else:
            m = rxInternalError.match(line)
            if m:
                print("#### Internal error detected!")
                print(m.group(1))
                return "InternalError"
    return False


hlsl_filename = os.abspath(
    os.path.join(
        os.environ["HLSL_SRC_DIR"], r"tools\clang\test\HLSL", "system-values.hlsl"
    )
)


def main():
    do("5_1")
    do("5_0")
    do("4_1")
    do("4_0")


def do(sm):
    import os, sys

    # set up table:
    table = [[None] * len(SignaturePoints) for sv in SysValues]
    null_filename = "output\\test_sv_null.txt"

    for col, (sigpoint, stage, tfpresent) in enumerate(SignaturePoints):
        entry = stage.upper() + "Main"
        target = stage + "_%s" % sm

        # test arb support:
        ret, output = run(
            "fxc %s /E %s /T %s /D%s_Defs=Def_Arb(float,Arb1,Arb1)"
            % (hlsl_filename, entry, target, sigpoint),
            null_filename,
        )
        arb_supported = ret == 0

        # iterate all system values
        sysvalues = tfpresent and SysValues[:-2] or SysValues
        for row, sv in enumerate(sysvalues):
            output_filename = "output\\test_sv_output_%s_%s_%s.txt" % (sm, sv, sigpoint)
            separate_reg, as_arb, def_last, result = False, False, False, "NotInSig"

            ret, output = run(
                "fxc %s /E %s /T %s /D%s_Defs=Def_%s"
                % (hlsl_filename, entry, target, sigpoint, sv),
                output_filename,
            )
            if ret:
                # Failed, look for expected error message:
                found = ParseSVError(output, "SV_" + sv)
                if found == "InternalError":
                    table[row][col] = "InternalError"
                    print(
                        '#### Internal error from ParseSVError - see "%s"'
                        % output_filename
                    )
                elif not found:
                    table[row][col] = "ParseSVError"
                    print('#### Error from ParseSVError - see "%s"' % output_filename)
                else:
                    table[row][col] = "NA"
                    if os.path.isfile(output_filename):
                        os.unlink(output_filename)
                continue

            parse_result = ParseSigs(output, "SV_" + sv)
            if parse_result:
                separate_reg, as_arb = parse_result
                if as_arb:
                    if separate_reg:
                        table[row][col] = "Error: both as_arb and separate_reg set!"
                        print(
                            '#### Error from ParseSigs, both as_arb and separate_reg set - see "%s"'
                            % output_filename
                        )
                        continue
                    result = "Arb"
                else:
                    if separate_reg:
                        result = "NotPacked"
                    else:
                        result = "SV"
                if os.path.isfile(output_filename):
                    os.unlink(output_filename)
            else:
                print('## Not in signature?  See "%s"' % output_filename)

            if arb_supported and not as_arb and not separate_reg:
                output_filename = "output\\test_sv_output_last_%s_%s_%s.txt" % (
                    sm,
                    sv,
                    sigpoint,
                )
                # must system value be declared last?  test by adding arb last if arb support
                ret, output = run(
                    'fxc %s /E %s /T %s /D%s_Defs="Def_%s Def_Arb(float,Arb1,Arb1)"'
                    % (hlsl_filename, entry, target, sigpoint, sv),
                    output_filename,
                )
                if ret:
                    found = ParseSGVError(output, "SV_" + sv)
                    if found == "InternalError":
                        result += " | InternalError found with ParseSGVError"
                        print(
                            '#### Internal error from ParseSGVError - see "%s"'
                            % output_filename
                        )
                    elif not found:
                        result += " | ParseSGVError"
                        print(
                            '#### Error from ParseSGVError - see "%s"' % output_filename
                        )
                    elif result == "SV":
                        result = "SGV"
                        if os.path.isfile(output_filename):
                            os.unlink(output_filename)
                    else:
                        result += " | Error: last required detected, but not SV?"
                        print(
                            '#### Error: last required detected, but not SV? - see "%s"'
                            % output_filename
                        )
                else:
                    if os.path.isfile(output_filename):
                        os.unlink(output_filename)

            table[row][col] = result

        for row in range(row + 1, len(SysValues)):
            table[row][col] = "TessFactor"

    def WriteTable(writefn, table):
        writefn(
            "Semantic,"
            + ",".join([sigpoint for sigpoint, stage, tfpresent in SignaturePoints])
            + "\n"
        )
        for n, row in enumerate(table):
            writefn((SysValues[n]) + ",".join(row) + "\n")

    WriteTable(sys.stdout.write, table)
    with open("fxc_sig_packing_table_%s.csv" % sm, "wt") as f:
        WriteTable(f.write, table)


if __name__ == "__main__":
    main()
