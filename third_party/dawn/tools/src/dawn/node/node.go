// Copyright 2024 The Dawn & Tint Authors
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

// Package node holds common code for running dawn/node
package node

import (
	"fmt"
	"strings"
)

// Flags for running dawn/node
type Flags []string

func (f *Flags) String() string {
	return strings.Join(*f, "")
}

// Set sets a dawn flag
func (f *Flags) Set(value string) error {
	// Multiple flags must be passed in individually:
	// -flag=a=b -dawn_node_flag=c=d
	*f = append(*f, value)
	return nil
}

// Consolidates all the delimiter separated flags with a given prefix into a single flag.
// Example:
// Given the flags: ["foo=a", "bar", "foo=b,c"]
// GlobListFlags("foo=", ",") will transform the flags to: ["bar", "foo=a,b,c"]
func (f *Flags) GlobListFlags(prefix string, delimiter string) {
	list := []string{}
	i := 0
	for _, flag := range *f {
		if strings.HasPrefix(flag, prefix) {
			// Trim the prefix.
			value := flag[len(prefix):]
			// Extract the deliminated values.
			list = append(list, strings.Split(value, delimiter)...)
		} else {
			(*f)[i] = flag
			i++
		}
	}
	(*f) = (*f)[:i]
	if len(list) > 0 {
		// Append back the consolidated flags.
		f.Set(prefix + strings.Join(list, delimiter))
	}
}

// Options that can be passed to Flags.SetOptions
type Options struct {
	BinDir            string
	Backend           string
	Adapter           string
	Validate          bool
	AllowUnsafeAPIs   bool
	DumpShaders       bool
	DumpShadersPretty bool
	UseFXC            bool
	UseIR             bool
}

func (f *Flags) SetOptions(opts Options) error {
	// For Windows, set the DLL directory to bin so that Dawn loads dxcompiler.dll from there.
	f.Set("dlldir=" + opts.BinDir)

	// Forward the backend and adapter to use, if specified.
	if opts.Backend != "" && opts.Backend != "default" {
		fmt.Println("Forcing backend to", opts.Backend)
		f.Set("backend=" + opts.Backend)
	}
	if opts.Adapter != "" {
		f.Set("adapter=" + opts.Adapter)
	}
	if opts.Validate {
		f.Set("validate=1")
	}

	if opts.AllowUnsafeAPIs {
		f.Set("enable-dawn-features=allow_unsafe_apis")
	}
	if opts.DumpShaders {
		f.Set("enable-dawn-features=dump_shaders")
	}
	if opts.DumpShadersPretty {
		f.Set("enable-dawn-features=dump_shaders,disable_symbol_renaming")
	}
	if opts.UseFXC {
		f.Set("disable-dawn-features=use_dxc")
	}
	f.GlobListFlags("enable-dawn-features=", ",")

	return nil
}
