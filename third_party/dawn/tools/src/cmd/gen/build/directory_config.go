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

// TargetConfig holds configuration options for a single target of a directory
type TargetConfig struct {
	// Override for the output name of this target
	OutputName string
	// Conditionals for this target
	Condition string
	// Additional dependencies to add to this target
	AdditionalDependencies struct {
		// List of internal dependency patterns
		Internal []string
		// List of external dependencies
		External []ExternalDependencyName
	}
}

// DirectoryConfig holds configuration options for a directory
type DirectoryConfig struct {
	// Condition for all targets in the directory
	Condition string
	// Configuration for the 'lib' target
	Lib *TargetConfig
	// Configuration for the 'proto' target
	Proto *TargetConfig
	// Configuration for the 'test' target
	Test *TargetConfig
	// Configuration for the 'test_cmd' target
	TestCmd *TargetConfig `json:"test_cmd"`
	// Configuration for the 'bench' target
	Bench *TargetConfig
	// Configuration for the 'bench_cmd' target
	BenchCmd *TargetConfig `json:"bench_cmd"`
	// Configuration for the 'fuzz' target
	Fuzz *TargetConfig
	// Configuration for the 'fuzz_cmd' target
	FuzzCmd *TargetConfig `json:"fuzz_cmd"`
	// Configuration for the 'cmd' target
	Cmd *TargetConfig
}
