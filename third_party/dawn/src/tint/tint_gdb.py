# Copyright 2022 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Pretty printers for the Tint project.
# Add a line to your ~/.gdbinit to source this file, e.g.:
#
# source /path/to/dawn/src/tint/tint_gdb.py

import gdb
import gdb.printing
from itertools import chain

# When debugging this module, set _DEBUGGING = True so that re-sourcing this file in gdb replaces
# the existing printers.
_DEBUGGING = True

# Enable to display other data members along with child elements of compound data types (arrays, etc.).
# This is useful in debuggers like VS Code that doesn't display the `to_string()` result in the watch window.
# OTOH, it's less useful when using gdb/lldb's print command.
_DISPLAY_MEMBERS_AS_CHILDREN = False


# Tips for debugging using VS Code:
# - Set a breakpoint where you can view the types you want to debug/write pretty printers for.
# - Debug Console: source /path/to/dawn/src/tint/tint_gdb.py
# - To execute Python code, in the Debug Console:
#   -exec python foo = gdb.parse_and_eval('map.set_')
#   -exec python v = (foo['slots_']['impl_']['slice']['data'] + 8).dereference()['value']
#
# - Useful docs:
#   Python API: https://sourceware.org/gdb/onlinedocs/gdb/Python-API.html#Python-API
#   Especially:
#     Types: https://sourceware.org/gdb/onlinedocs/gdb/Types-In-Python.html#Types-In-Python
#     Values: https://sourceware.org/gdb/onlinedocs/gdb/Values-From-Inferior.html#Values-From-Inferior


pp_set = gdb.printing.RegexpCollectionPrettyPrinter("tint")


class Printer(object):
    '''Base class for Printers'''

    def __init__(self, val):
        self.val = val

    def template_type(self, index):
        '''Returns template type at index'''
        return self.val.type.template_argument(index)


class UtilsSlicePrinter(Printer):
    '''Printer for tint::Slice<T>'''

    def __init__(self, val):
        super(UtilsSlicePrinter, self).__init__(val)
        self.len = self.val['len']
        self.cap = self.val['cap']
        self.data = self.val['data']
        self.elem_type = self.data.type.target().unqualified()

    def length(self):
        return self.len

    def value_at(self, index):
        '''Returns array value at index'''
        return (self.data + index).dereference().cast(self.elem_type)

    def to_string(self):
        return 'length={} capacity={}'.format(self.len, self.cap)

    def members(self):
        if _DISPLAY_MEMBERS_AS_CHILDREN:
            return [
                ('length', self.len),
                ('capacity', self.cap),
            ]
        else:
            return []

    def children(self):
        for m in self.members():
            yield m
        for i in range(self.len):
            yield str(i), self.value_at(i)

    def display_hint(self):
        return 'array'


pp_set.add_printer('UtilsSlicePrinter', '^tint::Slice<.*>$', UtilsSlicePrinter)


class UtilsVectorPrinter(Printer):
    '''Printer for tint::Vector<T, N>'''

    def __init__(self, val):
        super(UtilsVectorPrinter, self).__init__(val)
        self.slice = self.val['impl_']['slice']
        self.using_heap = self.slice['cap'] > self.template_type(1)

    def slice_printer(self):
        return UtilsSlicePrinter(self.slice)

    def to_string(self):
        return 'heap={} {}'.format(self.using_heap, self.slice)

    def members(self):
        if _DISPLAY_MEMBERS_AS_CHILDREN:
            return [
                ('heap', self.using_heap),
            ]
        else:
            return []

    def children(self):
        return chain(self.members(), self.slice_printer().children())

    def display_hint(self):
        return 'array'


pp_set.add_printer('UtilsVector', '^tint::Vector<.*>$', UtilsVectorPrinter)


class UtilsVectorRefPrinter(Printer):
    '''Printer for tint::VectorRef<T>'''

    def __init__(self, val):
        super(UtilsVectorRefPrinter, self).__init__(val)
        self.slice = self.val['slice_']
        self.can_move = self.val['can_move_']

    def to_string(self):
        return 'can_move={} {}'.format(self.can_move, self.slice)

    def members(self):
        if _DISPLAY_MEMBERS_AS_CHILDREN:
            return [
                ('can_move', self.can_move),
            ]
        else:
            return []

    def children(self):
        return chain(self.members(), UtilsSlicePrinter(self.slice).children())

    def display_hint(self):
        return 'array'


pp_set.add_printer('UtilsVector', '^tint::VectorRef<.*>$',
                   UtilsVectorRefPrinter)


class UtilsHashmapBasePrinter(Printer):
    '''Base Printer for HashmapBase-derived types'''

    def __init__(self, val):
        super(UtilsHashmapBasePrinter, self).__init__(val)
        self.slice = UtilsVectorPrinter(self.val['slots_']).slice_printer()
        self.try_read_std_optional_func = self.try_read_std_optional

    def to_string(self):
        length = 0
        for slot in range(0, self.slice.length()):
            v = self.slice.value_at(slot)
            if v['hash'] != 0:
                length += 1
        return 'length={}'.format(length)

    def children(self):
        for slot in range(0, self.slice.length()):
            v = self.slice.value_at(slot)
            if v['hash'] != 0:
                entry = v['entry']

                # entry is a std::optional, let's try to extract its value for display
                kvp = self.try_read_std_optional_func(slot, entry)
                if kvp is None:
                    # If we failed, just output the slot and entry as is, which will use
                    # the default visualizer for each.
                    kvp = slot, entry

                yield str(kvp[0]), kvp[1]

    def display_hint(self):
        return 'array'

    def try_read_std_optional(self, slot, entry):
        return None


class UtilsHashsetPrinter(UtilsHashmapBasePrinter):
    '''Printer for Hashset<T, N, HASH, EQUAL>'''

    def try_read_std_optional(self, slot, entry):
        try:
            # libstdc++
            v = entry['_M_payload']['_M_payload']['_M_value']
            return slot, v
        except:
            return None


pp_set.add_printer('UtilsHashset', '^tint::Hashset<.*>$', UtilsHashsetPrinter)


class UtilsHashmapPrinter(UtilsHashmapBasePrinter):
    '''Printer for Hashmap<K, V, N, HASH, EQUAL>'''

    def try_read_std_optional(self, slot, entry):
        try:
            # libstdc++
            kvp = entry['_M_payload']['_M_payload']['_M_value']
            return str(kvp['key']), kvp['value']
        except:
            return None


pp_set.add_printer('UtilsHashmap', '^tint::Hashmap<.*>$', UtilsHashmapPrinter)


gdb.printing.register_pretty_printer(gdb, pp_set, replace=_DEBUGGING)
