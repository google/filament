// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

package llvm_test

import (
	"flag"
	"os"
	"testing"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/llvm"
)

var testLLVMDownloads = flag.Bool("test-llvm-downloads", false, "include download tests of llvm")

func TestMain(m *testing.M) {
	flag.Parse()
	os.Exit(m.Run())
}

func TestLLVMDownloads(t *testing.T) {
	if !*testLLVMDownloads {
		t.Skip("LLVM download tests disabled. Enable with --test-llvm-downloads")
	}
	for _, version := range []llvm.Version{
		{Major: 10, Minor: 0, Point: 0},
	} {
		t.Logf("Downloading %v...", version)
		for _, os := range []string{"linux", "darwin", "windows"} {
			data, err := version.DownloadForOS(os)
			switch {
			case err != nil:
				t.Errorf("Download of LLVM %v failed with: %v", version, err)
			case len(data) == 0:
				t.Errorf("Download of LLVM %v resulted in no data", version)
			default:
				t.Logf("done\n")
			}
		}
	}
}
