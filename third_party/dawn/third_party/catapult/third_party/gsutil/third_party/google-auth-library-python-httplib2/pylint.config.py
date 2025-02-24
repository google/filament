# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""This module is used to config gcp-devrel-py-tools run-pylint."""

import copy

library_additions = {
    'MESSAGES CONTROL': {
        'disable': [
            'I',
            'import-error',
            'no-member',
            'protected-access',
            'redefined-variable-type',
            'similarities',
            'useless-object-inheritance',
            'no-else-return',
            'wrong-import-order',
        ],
    },
}

library_replacements = {
    'MASTER': {
        'ignore': ['CVS', '.git', '.cache', '.tox', '.nox'],
        'load-plugins': 'pylint.extensions.check_docs',
    },
    'REPORTS': {
        'reports': 'no',
    },
    'BASIC': {
        'method-rgx': '[a-z_][a-z0-9_]{2,40}$',
        'function-rgx': '[a-z_][a-z0-9_]{2,40}$',
    },
    'TYPECHECK': {
        'ignored-modules': ['google.protobuf'],
    },
    'DESIGN': {
        'min-public-methods': '0',
        'max-args': '10',
        'max-attributes': '15',
    },
}

test_additions = copy.deepcopy(library_additions)
test_additions['MESSAGES CONTROL']['disable'].extend([
    'missing-docstring',
    'no-self-use',
    'redefined-outer-name',
    'unused-argument',
    'no-name-in-module',
])
test_replacements = copy.deepcopy(library_replacements)
test_replacements.setdefault('BASIC', {})
test_replacements['BASIC'].update({
    'good-names': ['i', 'j', 'k', 'ex', 'Run', '_', 'fh', 'pytestmark'],
    'method-rgx': '[a-z_][a-z0-9_]{2,80}$',
    'function-rgx': '[a-z_][a-z0-9_]{2,80}$',
})

ignored_files = ()
