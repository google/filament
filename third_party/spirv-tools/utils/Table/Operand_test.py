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

import unittest
from . Operand import *

class TestOperand(unittest.TestCase):
    def test_enumerant(self) -> None:
        x = Operand({'enumerant': 'abc'});
        self.assertEqual(x.enumerant, 'abc');

    def test_value_decimal(self) -> None:
        x = Operand({'value': 123});
        self.assertEqual(x.value, 123);

    def test_value_str_hex(self) -> None:
        x = Operand({'value': "0x0101"});
        self.assertEqual(x.value, 257);

    def test_value_str_dec(self) -> None:
        x = Operand({'value': "0101"});
        self.assertEqual(x.value, 101);

    def test_value_str_invalid_dec(self) -> None:
        x = Operand({'value': "01ab"});
        self.assertRaises(Exception, lambda y: x.value, 0);

    def test_value_str_invalid_hex(self) -> None:
        x = Operand({'value': "0x010j"});
        self.assertRaises(Exception, lambda y: x.value, 0);

    def test_capabilities_absent(self) -> None:
        x = Operand({});
        self.assertEqual(x.capabilities, []);

    def test_capabilities_present(self) -> None:
        x = Operand({'capabilities': ['abc', 'def']});
        self.assertEqual(x.capabilities, ['abc', 'def']);

    def test_extensions_absent(self) -> None:
        x = Operand({});
        self.assertEqual(x.extensions, []);

    def test_extensions_present(self) -> None:
        x = Operand({'extensions': ['abc', 'def']});
        self.assertEqual(x.extensions, ['abc', 'def']);

    def test_aliases_absent(self) -> None:
        x = Operand({});
        self.assertEqual(x.aliases, []);

    def test_aliases_present(self) -> None:
        x = Operand({'aliases': ['abc', 'def']});
        self.assertEqual(x.aliases, ['abc', 'def']);

    def test_parameters_absent(self) -> None:
        x = Operand({});
        self.assertEqual(x.parameters, []);

    def test_parameters_present(self) -> None:
        x = Operand({'parameters': ['abc', 'def']});
        self.assertEqual(x.parameters, ['abc', 'def']);

    def test_version_absent(self) -> None:
        x = Operand({});
        self.assertEqual(x.version, None);

    def test_version_present(self) -> None:
        x = Operand({'version': '1.0'});
        self.assertEqual(x.version, '1.0');

    def test_lastVersion_absent(self) -> None:
        x = Operand({});
        self.assertEqual(x.lastVersion, None);

    def test_lastVersion_present(self) -> None:
        x = Operand({'lastVersion': '1.3'});
        self.assertEqual(x.lastVersion, '1.3');

    def test_all_propertites(self) -> None:
        x = Operand({'enumerant': 'Foobar',
                         'value': 12,
                         'capabilities': ["yes"],
                         'extensions': ["SPV_FOOBAR_baz_bat"],
                         'version': "1.0",
                         'lastVersion': "1.3",
                         });
        self.assertEqual(x.enumerant, 'Foobar');
        self.assertEqual(x.value, 12);
        self.assertEqual(x.capabilities, ["yes"]);
        self.assertEqual(x.extensions, ["SPV_FOOBAR_baz_bat"]);
        self.assertEqual(x.version, '1.0');
        self.assertEqual(x.lastVersion, '1.3');
