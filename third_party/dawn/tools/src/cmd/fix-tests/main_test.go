// Copyright 2025 The Dawn & Tint Authors
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

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

func TestTestName(t *testing.T) {
	group := TestsuiteGroup{Name: "Group/Name"}
	suite := Testsuite{Name: "SuiteName/0"}
	require.Equal(t, "Name.SuiteName", testName(group, suite))

	group = TestsuiteGroup{Name: "Simple"}
	suite = Testsuite{Name: "Test"}
	require.Equal(t, "Simple.Test", testName(group, suite))
}

func TestSize(t *testing.T) {
	mr := MatchRange{start: 5, end: 10}
	require.Equal(t, 5, Size(mr))

	mr = MatchRange{start: 0, end: 0}
	require.Equal(t, 0, Size(mr))
}

func TestLongestSubstringMatch(t *testing.T) {
	tests := []struct {
		name      string
		searchStr string
		docString string
		want      MatchRange
	}{
		{
			name:      "Exact match",
			searchStr: "abc",
			docString: "abc",
			want:      MatchRange{start: 0, end: 3},
		},
		{
			name:      "Substring match",
			searchStr: "bcd",
			docString: "abcdef",
			want:      MatchRange{start: 0, end: 3},
		},
		{
			name:      "Partial match start",
			searchStr: "xyzabc",
			docString: "abcdef",
			want:      MatchRange{start: 3, end: 6}, // "abc" matches
		},
		{
			name:      "Partial match end",
			searchStr: "defxyz",
			docString: "abcdef",
			want:      MatchRange{start: 0, end: 3}, // "def" matches
		},
		{
			name:      "No match",
			searchStr: "xyz",
			docString: "abcdef",
			want:      MatchRange{start: 0, end: 0},
		},
		{
			name:      "Multiple matches, picks longest",
			searchStr: "apple pie",
			docString: "I like apple tart and apple pie",
			want:      MatchRange{start: 0, end: 9},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := longestSubstringMatch(tt.searchStr, tt.docString)
			require.Equal(t, tt.want, got)
		})
	}
}

func TestProcessFailure_ExpectEq_Success(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := filepath.Join(wd, "test.cc")

	fileContent := `
TEST(MySuite, MyTest) {
  const char* actual = "actual_value";
  EXPECT_EQ("expected_value", actual);
}
`
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(filePath, []byte(fileContent), 0666))

	// Simulate an EXPECT_EQ failure message.
	failureMsgEq := `test.cc:3: Failure
Expected equality of these values:
  "expected_value"
  actual
    Which is: "actual_value"`

	err := processFailure("MySuite.MyTest", wd, failureMsgEq, wrapper)
	require.NoError(t, err)

	contentBytes, err := wrapper.ReadFile(filePath)
	require.NoError(t, err)
	content := string(contentBytes)
	require.Contains(t, content, `EXPECT_EQ("actual_value", actual);`)
}

func TestProcessFailure_HasSubstr_Success(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := filepath.Join(wd, "test.cc")

	fileContent := `
TEST(MySuite, SubstrTest) {
    const char* output = "The quick brown fox jumps over the lazy dog";
    EXPECT_THAT(output, HasSubstr("quick red fox"));
}
`
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(filePath, []byte(fileContent), 0666))

	// Simulate a HasSubstr failure message.
	failureMsgSubstr := `test.cc:3: Failure
Value of: output
Expected: has substring "quick red fox"
  Actual: "The quick brown fox jumps over the lazy dog"`

	err := processFailure("MySuite.SubstrTest", wd, failureMsgSubstr, wrapper)
	require.NoError(t, err)

	contentBytes, err := wrapper.ReadFile(filePath)
	require.NoError(t, err)
	content := string(contentBytes)
	require.Contains(t, content, `HasSubstr("quick brown fox")`)
}

func TestProcessFailure_RelativePath_Success(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := "test.cc"
	fullPath := filepath.Join(wd, filePath)

	fileContent := `TEST(Suite, Test) { EXPECT_EQ("a", "b"); }`
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(fullPath, []byte(fileContent), 0666))

	failureMsg := `test.cc:1: Failure
Expected equality of these values:
  "a"
  "b"`

	err := processFailure("Suite.Test", wd, failureMsg, wrapper)
	require.NoError(t, err)

	contentBytes, _ := wrapper.ReadFile(fullPath)
	require.Contains(t, string(contentBytes), `EXPECT_EQ("b", "b")`)
}

func TestProcessFailure_UnknownTest(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := filepath.Join(wd, "test.cc")
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(filePath, []byte(`TEST(A, B) {}`), 0666))

	failureMsg := `test.cc:1: Failure
Expected equality of these values:
  "a"
  "b"`

	err := processFailure("Unknown.Test", wd, failureMsg, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "Test not found")
}

func TestProcessFailure_FileNotFound(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"

	failureMsg := `test.cc:1: Failure
Expected equality of these values:
  "a"
  "b"`

	err := processFailure("Suite.Test", wd, failureMsg, wrapper)
	require.ErrorContains(t, err, "file does not exist")
}

func TestProcessFailure_UnknownFailureType(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := filepath.Join(wd, "test.cc")
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(filePath, []byte(`TEST(A,B){}`), 0666))

	err := processFailure("A.B", wd, "Some random failure", wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "cannot fix this type of failure")
}

func TestProcessFailure_ExpectEq_NoMatch(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := filepath.Join(wd, "test.cc")
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(filePath, []byte(`TEST(A,B){ EXPECT_EQ(x, y); }`), 0666))

	failureMsg := `test.cc:1: Failure
Expected equality of these values:
  "foo"
  "bar"`

	err := processFailure("A.B", wd, failureMsg, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "could not fix 'EXPECT_EQ' pattern")
}

func TestProcessFailure_HasSubstr_NoFix(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wd := "/src"
	filePath := filepath.Join(wd, "test.cc")
	require.NoError(t, wrapper.MkdirAll(wd, 0755))
	require.NoError(t, wrapper.WriteFile(filePath, []byte(`TEST(A,B){ EXPECT_THAT(x, HasSubstr("foo")); }`), 0666))

	failureMsg := `test.cc:1: Failure
Value of: x
Expected: has substring "foo"
  Actual: "bar"`

	err := processFailure("A.B", wd, failureMsg, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "could find fix for 'HasSubstr' pattern")
}

type mockOSWrapper struct {
	oswrapper.OSWrapper
	writeFileErr error
}

func (m *mockOSWrapper) WriteFile(name string, data []byte, perm os.FileMode) error {
	if m.writeFileErr != nil {
		return m.writeFileErr
	}
	return m.OSWrapper.WriteFile(name, data, perm)
}

func TestParseSourceFile_Success(t *testing.T) {
	content := `
Some Preamble
TEST(Group, Test1) {
  EXPECT_EQ(1, 1);
}
TEST_F(Group, Test2) {
  ASSERT_TRUE(true);
}
TEST_P(Group, Test3) {
  // Comment
}
`
	wrapper := oswrapper.CreateFSTestOSWrapper()
	path := "/test.cc"
	require.NoError(t, wrapper.WriteFile(path, []byte(content), 0666))

	source, err := parseSourceFile(path, wrapper)
	require.NoError(t, err)

	require.Len(t, source.parts, 4)
	require.Contains(t, source.tests, "Group.Test1")
	require.Contains(t, source.tests, "Group.Test2")
	require.Contains(t, source.tests, "Group.Test3")
	require.Equal(t, "TEST(Group, Test1) {\n  EXPECT_EQ(1, 1);\n}\n",
		source.parts[source.tests["Group.Test1"]])
}

func TestParseSourceFile_ReadError(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	_, err := parseSourceFile("/nonexistent.cc", wrapper)
	require.ErrorContains(t, err, "file does not exist")
}

func TestWriteSourceFile_Success(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	path := "/test.cc"

	source := sourceFile{
		parts: []string{"Part1", "Part2"},
		tests: map[string]int{},
	}

	err := writeSourceFile(path, source, wrapper)
	require.NoError(t, err)

	contentBytes, err := wrapper.ReadFile(path)
	require.NoError(t, err)
	require.Equal(t, "Part1Part2", string(contentBytes))
}

func TestWriteSourceFile_Error(t *testing.T) {
	wrapper := &mockOSWrapper{
		OSWrapper:    oswrapper.CreateFSTestOSWrapper(),
		writeFileErr: fmt.Errorf("mock write error"),
	}

	source := sourceFile{
		parts: []string{"Part1"},
	}
	err := writeSourceFile("/test.cc", source, wrapper)
	require.Error(t, err)
	require.Equal(t, "mock write error", err.Error())
}
