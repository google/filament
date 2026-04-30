#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 The Khronos Group Inc.
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and/or associated documentation files (the "Materials"),
# to deal in the Materials without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Materials, and to permit persons to whom the
# Materials are furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Materials.
#
# MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS KHRONOS
# STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS SPECIFICATIONS AND
# HEADER INFORMATION ARE LOCATED AT https:#www.khronos.org/registry/
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS
# IN THE MATERIALS.
"""
Enforce conventions of a SPIR-V JSON grammar file
"""

import json
import sys
from collections import OrderedDict


class Checker:

    def __init__(self, file_name, capabilities):
        self._file_name = file_name
        self._capabilities = capabilities

    def print_err(self, msg):
        print(self._file_name + ": " + msg)

    def check_capabilities(self, entry, kind=None):
        """
        Check the capabilities of an instruction or enum are declared.
        """
        if not "capabilities" in entry:
            return False
        error = False
        for cap in entry["capabilities"]:
            if not cap in self._capabilities:
                error = True
                if kind is not None:
                    self.print_err(
                        "For operand kind {}, enumerant {}, non existing capability {}"
                        .format(kind["kind"], entry["enumerant"], cap))
                else:
                    self.print_err(
                        "Instruction {}, non existing capability {}".format(
                            entry["opname"], cap))
        return error

    def check_instructions(self, insn_list):
        """
        Check conventions for instructions:
            - Must be ordered
            - No duplicated instructions
        """
        seen_insn = {}
        prev_ops = -1
        error = False
        for insn in insn_list:
            opcode = insn["opcode"]
            error = self.check_capabilities(insn) or error
            if opcode in seen_insn:
                self.print_err(
                    "Duplicated opcode: instruction {} and {}".format(
                        insn["opname"], seen_insn[opcode]["opname"]))
                error = True
                continue
            if insn["opname"] in seen_insn:
                self.print_err(
                    "Duplicated instruction name {}: opcode {} and {}".format(
                        opcode, seen_insn[opcode]["opcode"]))
                error = True
                continue
            seen_insn[opcode] = insn
            seen_insn[insn["opname"]] = insn
            if (prev_ops >= opcode):
                self.print_err("Out of order instruction {}".format(
                    insn["opname"]))
                error = True
            prev_ops = opcode
        return error

    # TODO: check for duplicated names
    def check_operand_values(self, kind, enumerants):
        """
        Check conventions for enumerants:
            - Enums must be ordered
        """
        seen_values = {}
        error = False
        prev_ops = -1
        for enum in enumerants:
            v = enum["value"]
            v = v if isinstance(v, int) else int(v, base=16)
            error = self.check_capabilities(enum, kind) or error
            if not v in seen_values:
                seen_values[v] = enum
            else:
                self.print_err(
                    "For operand kind {}, duplicated enumerant {}".format(
                        kind["kind"], enum["enumerant"]))
                error = True
            if (prev_ops >= v):
                self.print_err(
                    "For operand kind {}, out of order enumerant {}".format(
                        kind["kind"], enum["enumerant"]))
                error = True
            prev_ops = v
        return error

    def check_operand_kind(self, kind_list):
        """
        For each enumerant kind, ensure all enum values are valid
        """
        error = False
        for kind in kind_list:
            if "enumerants" in kind:
                error = self.check_operand_values(kind,
                                                  kind["enumerants"]) or error
        return error

    def check(self, grammar):
        error = False

        if "operand_kinds" in grammar:
            error = self.check_operand_kind(grammar["operand_kinds"]) or error
        if "instructions" in grammar:
            error = self.check_instructions(grammar["instructions"]) or error
        return error


def main():
    import argparse
    parser = argparse.ArgumentParser(
        description=
        'Check JSON grammars are well formed. The capabilities are aggregated from all files before performing checks.'
    )

    parser.add_argument('grammars',
                        metavar='<grammar>',
                        nargs='*',
                        help='Path to the grammar file.')
    args = parser.parse_args()

    error = False
    capabilities = {}

    def load_json_and_register_capabilities(grammar_file):
        with open(grammar_file) as json_fd:
            grammar_json = json.loads(json_fd.read(),
                                      object_pairs_hook=OrderedDict)
        if "operand_kinds" in grammar_json:
            for kind in grammar_json["operand_kinds"]:
                if kind["kind"] == "Capability":
                    for enum in kind["enumerants"]:
                        capabilities[enum["enumerant"]] = kind
        return grammar_json

    grammar_list = list([(grammar_file,
                          load_json_and_register_capabilities(grammar_file))
                         for grammar_file in args.grammars])

    for grammar_file, grammar_json in grammar_list:
        checker = Checker(grammar_file, capabilities)
        error = checker.check(grammar_json) or error

    return error


if __name__ == '__main__':
    sys.exit(main())
