// Copyright 2025 The Dawn & Tint Authors.
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
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cnf"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestLoadExternals(t *testing.T) {
	tests := []struct {
		name            string
		jsonContent     string
		skipWritingFile bool
		want            *Project
		wantErr         bool
		wantErrMsg      string
	}{
		{ /////////////////////////////////////////////////////////////////////////
			name:            "Non-existent file",
			skipWritingFile: true,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals:         container.NewMap[ExternalDependencyName, ExternalDependency](),
			},
			wantErr:    true,
			wantErrMsg: "open /foo.json: file does not exist",
		},
		{ /////////////////////////////////////////////////////////////////////////
			name:        "Invalid JSON",
			jsonContent: "{\"a\": \"b\"}",
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals:         container.NewMap[ExternalDependencyName, ExternalDependency](),
			},
			wantErr: true,
			wantErrMsg: ("failed to parse '/foo.json': json: cannot unmarshal string into Go value of " +
				"type struct { IncludePatterns []string; Condition string }"),
		},
		// Invalid regex not tested due to not being able to find any patterns that
		// actually cause regexp.Compile() to fail.
		{ /////////////////////////////////////////////////////////////////////////
			name: "Invalid condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "*"
    ],
    "Condition": "||"
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
			},
			wantErr: true,
			wantErrMsg: ("/foo.json: could not parse condition: Parse " +
				"error:\n\n||\n^^\n\nexpected 'ident', got '||"),
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Single external no pattern no condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": []
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name:      "name",
						Condition: nil,
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Single external single pattern no condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "gtest/**"
    ]
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name:      "name",
						Condition: nil,
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Single external multiple patterns no condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "gtest/**",
      "gmock/**"
    ]
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name:      "name",
						Condition: nil,
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Single external single pattern with condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "gtest/**"
    ],
    "Condition": "tint_build_spv_reader || tint_build_spv_writer"
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name: "name",
						Condition: cnf.Expr{
							{
								cnf.Unary{
									Negate: false,
									Var:    "tint_build_spv_reader",
								},
								cnf.Unary{
									Negate: false,
									Var:    "tint_build_spv_writer",
								},
							},
						},
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Multiple externals no pattern no condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": []
  },
  "other_name": {
    "IncludePatterns": []
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name:      "name",
						Condition: nil,
					},
					"other_name": ExternalDependency{
						Name:      "other_name",
						Condition: nil,
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Multiple externals single patterns no condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "gtest/**"
    ]
  },
  "other_name": {
    "IncludePatterns": [
      "gmock/**"
    ]
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name:      "name",
						Condition: nil,
					},
					"other_name": ExternalDependency{
						Name:      "other_name",
						Condition: nil,
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Multiple externals multiple patterns no condition",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "gtest/**",
      "foo/**"
    ]
  },
  "other_name": {
    "IncludePatterns": [
      "gmock/**",
      "bar/**"
    ]
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name:      "name",
						Condition: nil,
					},
					"other_name": ExternalDependency{
						Name:      "other_name",
						Condition: nil,
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Multiple externals single patterns with conditions",
			jsonContent: `{
  "name": {
    "IncludePatterns": [
      "gtest/**"
    ],
    "Condition": "tint_build_spv_reader || tint_build_spv_writer"
  },
  "other_name": {
    "IncludePatterns": [
      "gmock/**"
    ],
    "Condition": "!tint_build_spv_reader || !tint_build_spv_writer"
  }
}`,
			want: &Project{
				externalsJsonPath: "/foo.json",
				externals: map[ExternalDependencyName]ExternalDependency{
					"name": ExternalDependency{
						Name: "name",
						Condition: cnf.Expr{
							{
								cnf.Unary{
									Negate: false,
									Var:    "tint_build_spv_reader",
								},
								cnf.Unary{
									Negate: false,
									Var:    "tint_build_spv_writer",
								},
							},
						},
					},
					"other_name": ExternalDependency{
						Name: "other_name",
						Condition: cnf.Expr{
							{
								cnf.Unary{
									Negate: true,
									Var:    "tint_build_spv_reader",
								},
								cnf.Unary{
									Negate: true,
									Var:    "tint_build_spv_writer",
								},
							},
						},
					},
				},
			},
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			jsonFilePath := "/foo.json"
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if !testCase.skipWritingFile {
				err := wrapper.WriteFile(jsonFilePath, []byte(testCase.jsonContent), 0o700)
				require.NoErrorf(t, err, "Error writing file: %v", err)
			}

			p := Project{
				externalsJsonPath: jsonFilePath,
				externals:         container.NewMap[ExternalDependencyName, ExternalDependency](),
			}
			err := loadExternals(&p, wrapper)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Error loading externals: %v", err)
			}
			// We can't compare the entire structs to each other since
			// includePatternMatch will be an anonymous function.
			require.Equal(t, testCase.want.externalsJsonPath, p.externalsJsonPath)
			require.Equal(t, len(testCase.want.externals), len(p.externals))
			for k := range p.externals {
				require.Contains(t, testCase.want.externals, k)
				require.Equal(t, testCase.want.externals[k].Name, p.externals[k].Name)
				require.Equal(t, testCase.want.externals[k].Condition, p.externals[k].Condition)
			}
		})
	}
}

func TestPopulateSourceFiles(t *testing.T) {
	tests := []struct {
		name               string
		skipFileCreation   bool
		wantDirectories    []string
		wantTargets        [][]string
		wantFiles          [][]string
		wantGeneratedFiles [][]string
		wantErr            bool
		wantErrMsg         string
	}{
		{ /////////////////////////////////////////////////////////////////////////
			name:             "Non-existent root",
			skipFileCreation: true,
			wantErr:          true,
			wantErrMsg:       "open /root: file does not exist",
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Success",
			wantDirectories: []string{
				"a",
				"b",
			},
			wantTargets: [][]string{
				{
					"a",
					string(targetLib),
				},
				{
					"a",
					string(targetProto),
				},
				{
					"a",
					string(targetTestCmd),
				},
				{
					"b",
					string(targetLib),
				},
				{
					"b",
					string(targetProto),
				},
				{
					"b",
					string(targetTestCmd),
				},
			},
			wantFiles: [][]string{
				{
					"a",
					string(targetLib),
					"file.cc",
				},
				{
					"a",
					string(targetLib),
					"file.h",
				},
				{
					"a",
					string(targetLib),
					"file.mm",
				},
				{
					"a",
					string(targetProto),
					"file.proto",
				},
				{
					"a",
					string(targetTestCmd),
					"main_test.cc",
				},
				{
					"b",
					string(targetLib),
					"file.cc",
				},
				{
					"b",
					string(targetLib),
					"file.h",
				},
				{
					"b",
					string(targetLib),
					"file.mm",
				},
				{
					"b",
					string(targetProto),
					"file.proto",
				},
				{
					"b",
					string(targetTestCmd),
					"main_test.cc",
				},
			},
			wantGeneratedFiles: [][]string{
				{
					"a",
					string(targetProto),
					"file.pb.cc",
				},
				{
					"a",
					string(targetProto),
					"file.pb.h",
				},
				{
					"b",
					string(targetProto),
					"file.pb.cc",
				},
				{
					"b",
					string(targetProto),
					"file.pb.h",
				},
			},
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			wrapper := oswrapper.CreateMemMapOSWrapper()
			if !testCase.skipFileCreation {
				parentDirs := []string{"root", "not_root"}
				childDirs := []string{"a", "b", "fuzzers"}
				fileNames := []string{
					"file.cc", "file.h", "file.inl", "file.mm", "file.proto", "file.txt", "main_test.cc"}
				for _, pd := range parentDirs {
					for _, cd := range childDirs {
						wrapper.MkdirAll(fmt.Sprintf("/%s/%s", pd, cd), 0o700)
						for _, fn := range fileNames {
							wrapper.Create(fmt.Sprintf("/%s/%s/%s", pd, cd, fn))
						}
					}
				}
			}

			p := Project{
				Root:        "/root",
				Files:       container.NewMap[string, *File](),
				Directories: container.NewMap[string, *Directory](),
				Targets:     container.NewMap[TargetName, *Target](),
			}

			// Project structs become way too verbose to reasonably list out
			// explicitly in the test cases, so generate them here.
			want := Project{
				Root:        "/root",
				Files:       container.NewMap[string, *File](),
				Directories: container.NewMap[string, *Directory](),
				Targets:     container.NewMap[TargetName, *Target](),
			}
			for _, d := range testCase.wantDirectories {
				want.AddDirectory(d)
			}
			for _, t := range testCase.wantTargets {
				directory := want.Directories[t[0]]
				kind := TargetKind(t[1])
				want.AddTarget(directory, kind)
			}
			for _, f := range testCase.wantFiles {
				directoryName := f[0]
				kind := TargetKind(f[1])
				targetName := directoryName
				if !kind.IsLib() {
					targetName = fmt.Sprintf("%s:%s", directoryName, kind)
				}
				filepath := fmt.Sprintf("%s/%s", directoryName, f[2])
				target := want.Targets[TargetName(targetName)]
				target.AddSourceFile(want.AddFile(filepath))
			}
			for _, f := range testCase.wantGeneratedFiles {
				directoryName := f[0]
				kind := TargetKind(f[1])
				targetName := directoryName
				if !kind.IsLib() {
					targetName = fmt.Sprintf("%s:%s", directoryName, kind)
				}
				filepath := fmt.Sprintf("%s/%s", directoryName, f[2])
				target := want.Targets[TargetName(targetName)]
				target.AddGeneratedFile(want.AddGeneratedFile(filepath))
			}

			err := populateSourceFiles(&p, wrapper)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Error populating source files: %v", err)
			}
			require.Equal(t, want, p)
		})
	}
}

func TestCheckInclude(t *testing.T) {
	tests := []struct {
		name             string
		file             *File
		include          Include
		includeCondition Condition
		wantErr          bool
		wantErrMsg       string
	}{
		{ /////////////////////////////////////////////////////////////////////////
			name: "Success no conditions",
			file: &File{
				Name:      "file.cc",
				Condition: Condition{},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{},
				},
			},
			include:          Include{},
			includeCondition: Condition{},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Success all conditions met",
			file: &File{
				Name: "file.cc",
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "foo",
						},
					},
				},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{
						{
							cnf.Unary{
								Negate: true,
								Var:    "bar",
							},
						},
					},
				},
			},
			include: Include{
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "baz",
						},
					},
				},
			},
			includeCondition: Condition{
				{
					cnf.Unary{
						Negate: false,
						Var:    "foo",
					},
				},
				{
					cnf.Unary{
						Negate: true,
						Var:    "bar",
					},
				},
				{
					cnf.Unary{
						Negate: false,
						Var:    "baz",
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Success all conditions met underspecified",
			file: &File{
				Name: "file.cc",
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "foo",
						},
					},
				},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{
						{
							cnf.Unary{
								Negate: true,
								Var:    "bar",
							},
						},
					},
				},
			},
			include: Include{
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "baz",
						},
					},
				},
			},
			includeCondition: Condition{
				{
					cnf.Unary{
						Negate: false,
						Var:    "foo",
					},
				},
			},
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "includeCondition not met no source conditions",
			file: &File{
				Name:      "file.cc",
				Condition: Condition{},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{},
				},
			},
			include: Include{},
			includeCondition: Condition{
				{
					cnf.Unary{
						Negate: false,
						Var:    "foo",
					},
				},
			},
			wantErr: true,
			wantErrMsg: `/root/file.cc:0 #include "" requires guard: #if FOO

/root/file.cc build conditions: <none>
 build conditions: foo`,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "File condition not met",
			file: &File{
				Name: "file.cc",
				Condition: Condition{
					{
						cnf.Unary{
							Negate: true,
							Var:    "foo",
						},
					},
				},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{
						{
							cnf.Unary{
								Negate: true,
								Var:    "bar",
							},
						},
					},
				},
			},
			include: Include{
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "baz",
						},
					},
				},
			},
			includeCondition: Condition{
				{
					cnf.Unary{
						Negate: false,
						Var:    "foo",
					},
				},
				{
					cnf.Unary{
						Negate: true,
						Var:    "bar",
					},
				},
				{
					cnf.Unary{
						Negate: false,
						Var:    "baz",
					},
				},
			},
			wantErr: true,
			wantErrMsg: `/root/file.cc:0 #include "" requires guard: #if FOO

/root/file.cc build conditions: baz && (!foo) && (!bar)
 build conditions: foo && (!bar) && baz`,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Target condition not met",
			file: &File{
				Name: "file.cc",
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "foo",
						},
					},
				},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{
						{
							cnf.Unary{
								Negate: false,
								Var:    "bar",
							},
						},
					},
				},
			},
			include: Include{
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "baz",
						},
					},
				},
			},
			includeCondition: Condition{
				{
					cnf.Unary{
						Negate: false,
						Var:    "foo",
					},
				},
				{
					cnf.Unary{
						Negate: true,
						Var:    "bar",
					},
				},
				{
					cnf.Unary{
						Negate: false,
						Var:    "baz",
					},
				},
			},
			wantErr: true,
			wantErrMsg: `/root/file.cc:0 #include "" requires guard: #if (!BAR)

/root/file.cc build conditions: baz && foo && bar
 build conditions: foo && (!bar) && baz`,
		},
		{ /////////////////////////////////////////////////////////////////////////
			name: "Include condition not met",
			file: &File{
				Name: "file.cc",
				Condition: Condition{
					{
						cnf.Unary{
							Negate: false,
							Var:    "foo",
						},
					},
				},
				Directory: &Directory{
					Path: "/root",
				},
				Target: &Target{
					Condition: Condition{
						{
							cnf.Unary{
								Negate: true,
								Var:    "bar",
							},
						},
					},
				},
			},
			include: Include{
				Condition: Condition{
					{
						cnf.Unary{
							Negate: true,
							Var:    "baz",
						},
					},
				},
			},
			includeCondition: Condition{
				{
					cnf.Unary{
						Negate: false,
						Var:    "foo",
					},
				},
				{
					cnf.Unary{
						Negate: true,
						Var:    "bar",
					},
				},
				{
					cnf.Unary{
						Negate: false,
						Var:    "baz",
					},
				},
			},
			wantErr: true,
			wantErrMsg: `/root/file.cc:0 #include "" requires guard: #if BAZ

/root/file.cc build conditions: (!baz) && foo && (!bar)
 build conditions: foo && (!bar) && baz`,
		},
	}

	for _, testCase := range tests {
		t.Run(testCase.name, func(t *testing.T) {
			err := checkInclude(testCase.file, testCase.include, testCase.includeCondition)
			if testCase.wantErr {
				require.ErrorContains(t, err, testCase.wantErrMsg)
			} else {
				require.NoErrorf(t, err, "Got error checking include: %v", err)
			}
		})
	}
}

func TestCheckForCycles_EmptyGraph(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	project := &Project{
		Targets: container.NewMap[TargetName, *Target](),
	}

	err := checkForCycles(project, wrapper)
	require.NoErrorf(t, err, "Error checking for cycles: %v", err)
}

func TestCheckForCycles_NoCycle(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	project := &Project{}
	project.Targets = map[TargetName]*Target{
		"target_1": &Target{
			Name: "target_1",
			Dependencies: &Dependencies{
				internal: container.NewSet(TargetName("target_2")),
				project:  project,
			},
		},
		"target_2": &Target{
			Name: "target_2",
			Dependencies: &Dependencies{
				project: project,
			},
		},
	}

	err := checkForCycles(project, wrapper)
	require.NoErrorf(t, err, "Error checking for cycles: %v", err)
}

func TestCheckForCycles_Cycle(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	project := &Project{}
	project.Targets = map[TargetName]*Target{
		"target_1": &Target{
			Name: "target_1",
			Dependencies: &Dependencies{
				internal: container.NewSet(TargetName("target_2")),
				project:  project,
			},
		},
		"target_2": &Target{
			Name: "target_2",
			Dependencies: &Dependencies{
				internal: container.NewSet(TargetName("target_1")),
				project:  project,
			},
		},
	}

	err := checkForCycles(project, wrapper)
	require.ErrorContains(t, err, `cyclic target dependency found:
   target_1
   target_2
   target_1
   target_1
`)
}

func TestEmitDotFile_Empty(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	project := &Project{
		Targets: container.NewMap[TargetName, *Target](),
		Root:    "/root",
	}

	err := emitDotFile(project, targetLib, wrapper)
	require.NoErrorf(t, err, "Error emitting dot file: %v", err)

	bytes, err := wrapper.ReadFile(fmt.Sprintf("/root/%v.dot", targetLib))
	require.NoErrorf(t, err, "Error reading dot file: %v", err)
	expectedContents := `strict digraph {
  node [ shape = "box" ]
  node [ fontname = "Courier" ]
  node [ fontsize = "14" ]
  node [ style = "filled,rounded" ]
  node [ fillcolor = "yellow" ]
  edge [ fontname = "Courier" ]
  edge [ fontsize = "12" ]
}
`
	require.Equal(t, expectedContents, string(bytes[:]))
}

func TestEmitDotFile_OnlyMatchingKindIncluded(t *testing.T) {
	wrapper := oswrapper.CreateMemMapOSWrapper()
	project := &Project{
		Root: "/root",
	}
	project.Targets = map[TargetName]*Target{
		"target_1": &Target{
			Name: "target_1",
			Kind: targetLib,
			Dependencies: &Dependencies{
				internal: container.NewSet(TargetName("target_2"), TargetName("target_3")),
				project:  project,
			},
		},
		"target_2": &Target{
			Name: "target_2",
			Kind: targetLib,
			Dependencies: &Dependencies{
				project: project,
			},
		},
		"target_3": &Target{
			Name: "target_3",
			Kind: targetProto,
			Dependencies: &Dependencies{
				project: project,
			},
		},
	}

	err := emitDotFile(project, targetLib, wrapper)
	require.NoErrorf(t, err, "Error emitting dot file: %v", err)

	bytes, err := wrapper.ReadFile(fmt.Sprintf("/root/%v.dot", targetLib))
	require.NoErrorf(t, err, "Error reading dot file: %v", err)
	expectedContents := `strict digraph {
  node [ shape = "box" ]
  node [ fontname = "Courier" ]
  node [ fontsize = "14" ]
  node [ style = "filled,rounded" ]
  node [ fillcolor = "yellow" ]
  edge [ fontname = "Courier" ]
  edge [ fontsize = "12" ]
  n0 [label="target_1"]
  n1 [label="target_2"]
  n0 -> n1 [label=""]
  n0 -> n0 [label=""]
}
`
	require.Equal(t, expectedContents, string(bytes[:]))
}
