#!/usr/bin/env python3
# Copyright 2025 Google LLC

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#from typing import *
from enum import IntEnum
from typing import Dict, List
from . IndexRange import *
from . StringList import *


class Context():
    """
    Contains global tables for strings, and lists-of-strings.
    It contains:

        - string_buffer: A list of null-terminated strings. The list is
          partitioned into contiguous segments dedicated to a specific
          kind of string, e.g.:
            - all instruction opcodes
            - all extension names
            - all aliases for a given enum
            - all enum names for a first operand type
            - all enum names for a second operand type, etc.
          Strings are sorted within each segment.

        - string_total_len: The sum of lengths of strings in string_buffer.

        - strings: Maps a string to an IndexRange indicating where the string
          can be found in the (future) concatenation of all strings in the
          string_buffer.

        - range_buffer: A dictionary mapping a string kind to a list IndexRange
              objects R. Each R is one of:

            - An index range referencing one string as it appears in the
              (future) concatenation of all strings in the string_buffer.
              In this case R represents a single string.

            - An index range referencing earlier elements in range_buffer[kind]
              itself.  In this case R represents a list of strings.

        - ranges:  A dictionary mapping a string kind and lists-of-strings
            to its encoding in the range_buffer array.

            It is a two-level mapping of Python type:

                Dict[str,dict[StringList,IndexRange]]

            where

                ranges[kind][list of strings] = an IndexRange

            The 'kind' string encodes a purpose including:
                - opcodes: the list of instruction opcode strings.
                - the list of aliases for an opcode, or an enum
                - an operand type such as 'SPV_OPERAND_TYPE_DIMENSIONALITY':
                  the list of operand type names, e.g. '2D', '3D', 'Cube',
                  'Rect', etc. in the case of SPV_OPERAND_TYPE_DIMENSIONALITY.
            By convention, the 'kind' string should be a singular noun for
            the type of object named by each member of the list.

            The IndexRange leaf value encodes a list of strings as in the
            second case described for 'range_buffer'.

    """
    def __init__(self) -> None:
        self.string_total_len: int = 0  # Sum of  lengths of all strings in string_buffer
        self.string_buffer: List[str] = []
        self.strings: Dict[str, IndexRange] = {}
        self.ir_to_string: Dict[IndexRange, str] = {} # Inverse of self.strings

        self.range_buffer: Dict[str,List[IndexRange]] = {}
        # We need StringList here because it's hashable, and so it
        # can be used as the key for a dict.
        self.ranges: Dict[str,Dict[StringList,IndexRange]] = {}

    def GetString(self, ir: IndexRange) -> str:
        if ir in self.ir_to_string:
            return self.ir_to_string[ir]
        raise Exception("unregistered index range {}".format(str(ir)))

    def AddString(self, s: str) -> IndexRange:
        """
        Adds or finds a string in the string_buffer.
        Returns its IndexRange.
        """
        if s in self.strings:
            return self.strings[s]
        # Allocate space, including for the terminating null.
        s_space: int = len(s) + 1
        ir = IndexRange(self.string_total_len, s_space)
        self.strings[s] = ir
        self.ir_to_string[ir] = s
        self.string_total_len += s_space
        self.string_buffer.append(s)
        return ir

    def AddStringList(self, kind: str, words: List[str]) -> IndexRange:
        """
        Ensures a list of strings is recorded in range_buffer[kind], and
        returns its location in the range_buffer[kind].
        As a side effect, also ensures each string in the list is in
        the string_buffer.
        """
        l = StringList(words)

        entry: Dict[StringList, IndexRange] = self.ranges.get(kind, {})
        if kind not in self.ranges:
            self.ranges[kind] = entry
            self.range_buffer[kind] = []

        if l in entry:
            return entry[l]
        new_ranges = [self.AddString(s) for s in l]
        ir = IndexRange(len(self.range_buffer[kind]), len(new_ranges))
        self.range_buffer[kind].extend(new_ranges)
        entry[l] = ir
        return ir

    def dump(self) -> None:
        print("string_total_len: {}".format(self.string_total_len))

        sbi = 0
        print("string_buffer:")
        for sb in self.string_buffer:
            print("  {}: '{}'".format(sbi, sb))
            sbi += len(sb) + 1
        print("")

        s = []
        for k,v in self.strings.items():
            s.append("'{}': {}".format(k,str(v)))
        print("strings:\n  {}\n".format('\n  '.join(s)))

        for rbk, rbv in self.range_buffer.items():
            print("range_buffer[{}]:".format(rbk))
            i: int = 0
            for r in rbv:
                print("  {} {}: {}".format(rbk, i, str(r)))
                i += 1
            print("")

        for rk, rv in self.ranges.items():
            for key,val in rv.items():
                print("ranges[{}][{}]: {}".format(str(rk),str(key), str(val)))
        print("")


