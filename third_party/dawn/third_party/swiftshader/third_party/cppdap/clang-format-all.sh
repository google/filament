# Copyright 2020 The Marl Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SRC_DIR=${ROOT_DIR}/src
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

# Double clang-format, as it seems that one pass isn't always enough
find ${SRC_DIR} -iname "*.h" -o -iname "*.cpp" | xargs ${CLANG_FORMAT} -i -style=file
find ${SRC_DIR} -iname "*.h" -o -iname "*.cpp" | xargs ${CLANG_FORMAT} -i -style=file
