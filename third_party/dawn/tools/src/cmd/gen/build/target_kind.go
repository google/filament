// Copyright 2023 The Dawn & Tint Authors.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package build

import (
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// TargetKind is an enumerator of target kinds
type TargetKind string

const (
	// A library target, used for production code.
	targetLib TargetKind = "lib"
	// A library target generated from a proto file, used for production code.
	targetProto TargetKind = "proto"
	// A library target, used for test binaries.
	targetTest TargetKind = "test"
	// A library target, used for benchmark binaries.
	targetBench TargetKind = "bench"
	// A library target, used for fuzzer binaries.
	targetFuzz TargetKind = "fuzz"
	// An executable target.
	targetCmd TargetKind = "cmd"
	// A test executable target.
	targetTestCmd TargetKind = "test_cmd"
	// A benchmark executable target.
	targetBenchCmd TargetKind = "bench_cmd"
	// A fuzzer executable target.
	targetFuzzCmd TargetKind = "fuzz_cmd"
	// An invalid target.
	targetInvalid TargetKind = "<invalid>"
)

// IsLib returns true if the TargetKind is 'lib'
func (k TargetKind) IsLib() bool { return k == targetLib }

// IsProto returns true if the TargetKind is 'proto'
func (k TargetKind) IsProto() bool { return k == targetProto }

// IsTest returns true if the TargetKind is 'test'
func (k TargetKind) IsTest() bool { return k == targetTest }

// IsBench returns true if the TargetKind is 'bench'
func (k TargetKind) IsBench() bool { return k == targetBench }

// IsFuzz returns true if the TargetKind is 'fuzz'
func (k TargetKind) IsFuzz() bool { return k == targetFuzz }

// IsCmd returns true if the TargetKind is 'cmd'
func (k TargetKind) IsCmd() bool { return k == targetCmd }

// IsTestCmd returns true if the TargetKind is 'test_cmd'
func (k TargetKind) IsTestCmd() bool { return k == targetTestCmd }

// IsBenchCmd returns true if the TargetKind is 'bench_cmd'
func (k TargetKind) IsBenchCmd() bool { return k == targetBenchCmd }

// IsFuzzCmd returns true if the TargetKind is 'fuzz_cmd'
func (k TargetKind) IsFuzzCmd() bool { return k == targetFuzzCmd }

// IsTestOrTestCmd returns true if the TargetKind is 'test' or 'test_cmd'
func (k TargetKind) IsTestOrTestCmd() bool { return k.IsTest() || k.IsTestCmd() }

// IsBenchOrBenchCmd returns true if the TargetKind is 'bench' or 'bench_cmd'
func (k TargetKind) IsBenchOrBenchCmd() bool { return k.IsBench() || k.IsBenchCmd() }

// AllTargetKinds is a list of all the target kinds
var AllTargetKinds = []TargetKind{
	targetLib,
	targetProto,
	targetTest,
	targetBench,
	targetFuzz,
	targetCmd,
	targetTestCmd,
	targetBenchCmd,
	targetFuzzCmd,
}

// targetKindFromFilename returns the target kind my pattern matching the filename
func targetKindFromFilename(filename string) TargetKind {
	noExt, ext := fileutils.SplitExt(filename)

	if ext == "proto" {
		return targetProto
	}

	if ext != "cc" && ext != "mm" && ext != "h" {
		return targetInvalid
	}

	switch {
	case filename == "main_test.cc":
		return targetTestCmd
	case filename == "main_bench.cc":
		return targetBenchCmd
	case filename == "main_fuzz.cc":
		return targetFuzzCmd
	case strings.HasSuffix(noExt, "_test"):
		return targetTest
	case noExt == "bench" || strings.HasSuffix(noExt, "_bench"):
		return targetBench
	case noExt == "fuzz" || strings.HasSuffix(noExt, "_fuzz"):
		return targetFuzz
	case noExt == "main" || strings.HasSuffix(noExt, "_main"):
		return targetCmd
	default:
		return targetLib
	}
}

// isValidDependency returns true iff its valid for a target of kind 'from' to
// depend on a target with kind 'to'.
func isValidDependency(from, to TargetKind) bool {
	switch from {
	case targetLib, targetCmd:
		return to == targetLib || to == targetProto
	case targetTest, targetTestCmd:
		return to == targetLib || to == targetProto || to == targetTest
	case targetBench, targetBenchCmd:
		return to == targetLib || to == targetProto || to == targetBench
	case targetFuzz, targetFuzzCmd:
		return to == targetLib || to == targetProto || to == targetFuzz
	default:
		return false
	}
}
