// Copyright 2017 The Draco Authors.
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

// Prepares callbacks that can be used to inform the caller that the module has
// been fully loaded.
var isRuntimeInitialized = false;
var isModuleParsed = false;

// These two callbacks can be called in arbitrary order. We call the final
// function |onModuleLoaded| after both of these callbacks have been called.
Module['onRuntimeInitialized'] = function() {
  isRuntimeInitialized = true;
  if (isModuleParsed) {
    if (typeof Module['onModuleLoaded'] === 'function') {
      Module['onModuleLoaded'](Module);
    }
  }
};

Module['onModuleParsed'] = function() {
  isModuleParsed = true;
  if (isRuntimeInitialized) {
    if (typeof Module['onModuleLoaded'] === 'function') {
      Module['onModuleLoaded'](Module);
    }
  }
};
