#!/usr/bin/env python3
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
#
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

import os
import json
from generators.base_generator import BaseGenerator

class SpirvToolCommitIdOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        json_file = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'known_good.json'))

        commit_id = 'COMMIT NOT FOUND'
        with open(json_file) as json_stream:
            json_data = json.load(json_stream)
            commit_id = [x for x in json_data['repos'] if x['name'] == 'SPIRV-Tools'][0]['commit']

        try:
            str_as_int = int(commit_id, 16)
        except ValueError:
            raise ValueError(f'commit ID for SPIRV_TOOLS_COMMIT_ID ({commit_id}) must be a SHA1 hash.')
        if len(commit_id) != 40:
            raise ValueError(f'commit ID for SPIRV_TOOLS_COMMIT_ID ({commit_id}) must be a SHA1 hash.')

        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
            * Copyright (c) 2015-2025 Google Inc.
            *
            * Licensed under the Apache License, Version 2.0 (the "License");
            * you may not use this file except in compliance with the License.
            * You may obtain a copy of the License at
            *
            *     http://www.apache.org/licenses/LICENSE-2.0
            *
            * Unless required by applicable law or agreed to in writing, software
            * distributed under the License is distributed on an "AS IS" BASIS,
            * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
            * See the License for the specific language governing permissions and
            * limitations under the License.
            ****************************************************************************/

            #pragma once

            ''')

        out.append(f'#define SPIRV_TOOLS_COMMIT_ID "{commit_id}"')
        self.write("".join(out))
