// Copyright 2016 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Returns true if the specified Draco version is supported by this decoder.
function isVersionSupported(versionString) {
  if (typeof versionString !== 'string')
    return false;
  const version = versionString.split('.');
  if (version.length < 2 || version.length > 3)
    return false;  // Unexpected version string.
  if (version[0] == 1 && version[1] >= 0 && version[1] <= 3)
    return true;
  if (version[0] != 0 || version[1] > 10)
    return false;
  return true;
}

Module['isVersionSupported'] = isVersionSupported;
