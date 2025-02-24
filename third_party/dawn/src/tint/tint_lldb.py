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
#
# If using lldb from command line, add a line to your ~/.lldbinit to import the printers:
#
#    command script import /path/to/dawn/src/tint/tint_lldb.py
#
#
# If using VS Code on MacOS with the Microsoft C/C++ extension, add the following to
# your launch.json (make sure you specify an absolute path to tint_lldb.py):
#
#    "name": "Launch",
#    "type": "cppdbg",
#    "request": "launch",
#    ...
#    "setupCommands": [
#        {
#            "description": "Load tint pretty printers",
#            "ignoreFailures": false,
#            "text": "command script import /path/to/dawn/src/tint/tint_lldb.py,
#        }
#    ]
#
# If using VS Code with the CodeLLDB extension (https://github.com/vadimcn/vscode-lldb),
# add the following to your launch.json:
#
#    "name": "Launch",
#    "type": "lldb",
#    "request": "launch",
#    ...
#    "initCommands": [
#        "command script import /path/to/dawn/src/tint/tint_lldb.py"
#    ]

# Based on pretty printers for:
# Rust: https://github.com/vadimcn/vscode-lldb/blob/master/formatters/rust.py
# Dlang: https://github.com/Pure-D/dlang-debug/blob/master/lldb_dlang.py
#
#
# Tips for debugging using VS Code:
#
# - Set a breakpoint where you can view the types you want to debug/write pretty printers for.
# - Debug Console: -exec command script import /path/to/dawn/src/tint/tint_lldb.py
# - You can re-run the above command to reload the printers after modifying the python script.

# - Useful docs:
#   Formattesr: https://lldb.llvm.org/use/variable.html
#   Python API: https://lldb.llvm.org/python_api.html
#   Especially:
#     SBType: https://lldb.llvm.org/python_api/lldb.SBType.html
#     SBValue: https://lldb.llvm.org/python_api/lldb.SBValue.html

from __future__ import print_function, division
import sys
import logging
import re
import lldb
import types

if sys.version_info[0] == 2:
    # python2-based LLDB accepts utf8-encoded ascii strings only.
    def to_lldb_str(s): return s.encode(
        'utf8', 'backslashreplace') if isinstance(s, unicode) else s
    range = xrange
else:
    to_lldb_str = str

string_encoding = "escape"  # remove | unicode | escape

log = logging.getLogger(__name__)

module = sys.modules[__name__]
tint_category = None


def __lldb_init_module(debugger, dict):
    global tint_category

    tint_category = debugger.CreateCategory('tint')
    tint_category.SetEnabled(True)

    attach_synthetic_to_type(UtilsSlicePrinter, r'^tint::Slice<.+>$', True)

    attach_synthetic_to_type(UtilsVectorPrinter, r'^tint::Vector<.+>$', True)

    attach_synthetic_to_type(UtilsVectorRefPrinter, r'^tint::VectorRef<.+>$',
                             True)

    attach_synthetic_to_type(UtilsHashsetPrinter, r'^tint::Hashset<.+>$', True)

    attach_synthetic_to_type(UtilsHashmapPrinter, r'^tint::Hashmap<.+>$', True)


def attach_synthetic_to_type(synth_class, type_name, is_regex=False):
    global module, tint_category
    synth = lldb.SBTypeSynthetic.CreateWithClassName(
        __name__ + '.' + synth_class.__name__)
    synth.SetOptions(lldb.eTypeOptionCascade)
    ret = tint_category.AddTypeSynthetic(
        lldb.SBTypeNameSpecifier(type_name, is_regex), synth)
    log.debug('attaching synthetic %s to "%s", is_regex=%s -> %s',
              synth_class.__name__, type_name, is_regex, ret)

    def summary_fn(valobj, dict): return get_synth_summary(
        synth_class, valobj, dict)
    # LLDB accesses summary fn's by name, so we need to create a unique one.
    summary_fn.__name__ = '_get_synth_summary_' + synth_class.__name__
    setattr(module, summary_fn.__name__, summary_fn)
    attach_summary_to_type(summary_fn, type_name, is_regex)


def attach_summary_to_type(summary_fn, type_name, is_regex=False):
    global module, tint_category
    summary = lldb.SBTypeSummary.CreateWithFunctionName(
        __name__ + '.' + summary_fn.__name__)
    summary.SetOptions(lldb.eTypeOptionCascade)
    ret = tint_category.AddTypeSummary(
        lldb.SBTypeNameSpecifier(type_name, is_regex), summary)
    log.debug('attaching summary %s to "%s", is_regex=%s -> %s',
              summary_fn.__name__, type_name, is_regex, ret)


def get_synth_summary(synth_class, valobj, dict):
    ''''
    get_summary' is annoyingly not a part of the standard LLDB synth provider API.
    This trick allows us to share data extraction logic between synth providers and their sibling summary providers.
    '''
    synth = synth_class(valobj.GetNonSyntheticValue(), dict)
    synth.update()
    summary = synth.get_summary()
    return to_lldb_str(summary)


def member(valobj, *chain):
    '''Performs chained GetChildMemberWithName lookups'''
    for name in chain:
        valobj = valobj.GetChildMemberWithName(name)
    return valobj


class Printer(object):
    '''Base class for Printers'''

    def __init__(self, valobj, dict={}):
        self.valobj = valobj
        self.initialize()

    def initialize(self):
        return None

    def update(self):
        return False

    def num_children(self):
        return 0

    def has_children(self):
        return False

    def get_child_at_index(self, index):
        return None

    def get_child_index(self, name):
        return None

    def get_summary(self):
        return None

    def member(self, *chain):
        '''Performs chained GetChildMemberWithName lookups'''
        return member(self.valobj, *chain)

    def template_params(self):
        '''Returns list of template params values (as strings)'''
        type_name = self.valobj.GetTypeName()
        params = []
        level = 0
        start = 0
        for i, c in enumerate(type_name):
            if c == '<':
                level += 1
                if level == 1:
                    start = i + 1
            elif c == '>':
                level -= 1
                if level == 0:
                    params.append(type_name[start:i].strip())
            elif c == ',' and level == 1:
                params.append(type_name[start:i].strip())
                start = i + 1
        return params

    def template_param_at(self, index):
        '''Returns template param value at index (as string)'''
        return self.template_params()[index]


class UtilsSlicePrinter(Printer):
    '''Printer for tint::Slice<T>'''

    def initialize(self):
        self.len = self.valobj.GetChildMemberWithName('len')
        self.cap = self.valobj.GetChildMemberWithName('cap')
        self.data = self.valobj.GetChildMemberWithName('data')
        self.elem_type = self.data.GetType().GetPointeeType()
        self.elem_size = self.elem_type.GetByteSize()

    def get_summary(self):
        return 'length={} capacity={}'.format(self.len.GetValueAsUnsigned(), self.cap.GetValueAsUnsigned())

    def num_children(self):
        # NOTE: VS Code on MacOS hangs if we try to expand something too large, so put an artificial limit
        # until we can figure out how to know if this is a valid instance.
        return min(self.len.GetValueAsUnsigned(), 256)

    def has_children(self):
        return True

    def get_child_at_index(self, index):
        try:
            if not 0 <= index < self.num_children():
                return None
            # TODO: return self.value_at(index)
            offset = index * self.elem_size
            return self.data.CreateChildAtOffset('[%s]' % index, offset, self.elem_type)
        except Exception as e:
            log.error('%s', e)
            raise

    def value_at(self, index):
        '''Returns array value at index'''
        offset = index * self.elem_size
        return self.data.CreateChildAtOffset('[%s]' % index, offset, self.elem_type)


class UtilsVectorPrinter(Printer):
    '''Printer for tint::Vector<T, N>'''

    def initialize(self):
        self.slice = self.member('impl_', 'slice')
        self.slice_printer = UtilsSlicePrinter(self.slice)
        self.fixed_size = int(self.template_param_at(1))
        self.cap = self.slice_printer.member('cap')

    def get_summary(self):
        using_heap = self.cap.GetValueAsUnsigned() > self.fixed_size
        return 'heap={} {}'.format(using_heap, self.slice_printer.get_summary())

    def num_children(self):
        return self.slice_printer.num_children()

    def has_children(self):
        return self.slice_printer.has_children()

    def get_child_at_index(self, index):
        return self.slice_printer.get_child_at_index(index)

    def make_slice_printer(self):
        return UtilsSlicePrinter(self.slice)


class UtilsVectorRefPrinter(Printer):
    '''Printer for tint::VectorRef<T>'''

    def initialize(self):
        self.slice = self.member('slice_')
        self.slice_printer = UtilsSlicePrinter(self.slice)
        self.can_move = self.member('can_move_')

    def get_summary(self):
        return 'can_move={} {}'.format(self.can_move.GetValue(), self.slice_printer.get_summary())

    def num_children(self):
        return self.slice_printer.num_children()

    def has_children(self):
        return self.slice_printer.has_children()

    def get_child_at_index(self, index):
        return self.slice_printer.get_child_at_index(index)


class UtilsHashmapBasePrinter(Printer):
    '''Base Printer for HashmapBase-derived types'''

    def initialize(self):
        self.slice = UtilsVectorPrinter(
            self.member('slots_')).make_slice_printer()

        self.try_read_std_optional_func = self.try_read_std_optional

    def update(self):
        self.valid_slots = []
        for slot in range(0, self.slice.num_children()):
            v = self.slice.value_at(slot)
            if member(v, 'hash').GetValueAsUnsigned() != 0:
                self.valid_slots.append(slot)
        return False

    def get_summary(self):
        return 'length={}'.format(self.num_children())

    def num_children(self):
        return len(self.valid_slots)

    def has_children(self):
        return True

    def get_child_at_index(self, index):
        slot = self.valid_slots[index]
        v = self.slice.value_at(slot)
        entry = member(v, 'entry')

        # entry is a std::optional, let's try to extract its value for display
        kvp = self.try_read_std_optional_func(slot, entry)
        if kvp is None:
            # If we failed, just output the slot and entry as is, which will use
            # the default printer for std::optional.
            kvp = slot, entry

        return kvp[1].CreateChildAtOffset('[{}]'.format(kvp[0]), 0, kvp[1].GetType())

    def try_read_std_optional(self, slot, entry):
        return None


class UtilsHashsetPrinter(UtilsHashmapBasePrinter):
    '''Printer for Hashset<T, N, HASH, EQUAL>'''

    def try_read_std_optional(self, slot, entry):
        try:
            # libc++
            v = entry.EvaluateExpression('__val_')
            if v.name is not None:
                return slot, v

            # libstdc++
            v = entry.EvaluateExpression('_M_payload._M_payload._M_value')
            if v.name is not None:
                return slot, v
            return None
        except:
            return None


class UtilsHashmapPrinter(UtilsHashsetPrinter):
    '''Printer for Hashmap<K, V, N, HASH, EQUAL>'''

    def try_read_std_optional(self, slot, entry):
        try:
            # libc++
            val = entry.EvaluateExpression('__val_')
            k = val.EvaluateExpression('key')
            v = val.EvaluateExpression('value')
            if k.name is not None and v.name is not None:
                return k.GetValue(), v

            # libstdc++
            val = entry.EvaluateExpression('_M_payload._M_payload._M_value')
            k = val.EvaluateExpression('key')
            v = val.EvaluateExpression('value')
            if k.name is not None and v.name is not None:
                return k.GetValue(), v
            return None
        except:
            return None
