// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef Version_h
#define Version_h

#define MAJOR_VERSION 5
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define BUILD_REVISION 1

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#define REVISION_STRING MACRO_STRINGIFY(BUILD_REVISION)
#define VERSION_STRING             \
	MACRO_STRINGIFY(MAJOR_VERSION) \
	"." MACRO_STRINGIFY(MINOR_VERSION) "." MACRO_STRINGIFY(PATCH_VERSION)

#endif  // Version_h
