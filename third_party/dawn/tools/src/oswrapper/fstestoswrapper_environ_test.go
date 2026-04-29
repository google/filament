// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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
package oswrapper_test

import (
	"os"
	"runtime"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
)

// Tests for the environ-related components of FSTestOSWrapper.

// NOTE: There are two types of tests in this file, those suffixed with _MatchesReal and those that
// are not. Those that are suffixed are meant to be testing the behaviour of the FSTestOSWrapper
// against the RealOSWrapper to confirm that it is a drop in replacement. Those that are not
// suffixed are traditional unittests that test the implementation functions in isolation against
// defined expectations.

func TestCreateFSTestOSWrapperWithRealEnv_MatchesReal(t *testing.T) {
	testWrapper := oswrapper.CreateFSTestOSWrapperWithRealEnv()

	for key, testVal := range testWrapper.EnvMap() {
		if key == "PWD" || key == "HOME" {
			// PWD and HOME are handled below
			continue
		}

		realVal := os.Getenv(key)
		require.Equal(t, realVal, testVal, "environment variable '%s' should match real env", key)
	}

	// PWD and HOME may not be set in the real environment, but FSTestEnvironProvider sets them
	// based on the appropriate os calls.
	realWd, realWdErr := os.Getwd()
	if realWdErr == nil {
		testWd, testWdErr := testWrapper.Getwd()
		require.NoError(t, testWdErr, "Getwd() should not fail if real Getwd() succeeds")
		require.Equal(t, realWd, testWd, "PWD should match real working directory")
	}

	realHome, realHomeErr := os.UserHomeDir()
	if realHomeErr == nil {
		testHome, testHomeErr := testWrapper.UserHomeDir()
		require.NoError(t, testHomeErr, "UserHomeDir() should not fail if real UserHomeDir() succeeds")
		require.Equal(t, realHome, testHome, "HOME should match real user home directory")
	}
}

func TestFSTestEnvironProvider_Environ(t *testing.T) {
	t.Run("Empty environment", func(t *testing.T) {
		wrapper := oswrapper.CreateFSTestOSWrapper()
		env := wrapper.Environ()
		require.Empty(t, env)
	})

	t.Run("With variables", func(t *testing.T) {
		wrapper := oswrapper.CreateFSTestOSWrapper()
		wrapper.Setenv("FOO", "bar")
		wrapper.Setenv("BAZ", "qux")
		env := wrapper.Environ()
		require.ElementsMatch(t, []string{"FOO=bar", "BAZ=qux"}, env)
	})
}

// TestFSTestEnvironProvider_Environ_MatchesReal is not implemented, since the test version inserts
// PWD and HOME into the map, which might not match the behaviour of all environments, so there would
// need to be some special case logic, inspecting if the real system provides these values and
// massaging the map. And this just be getting us the same coverage as
// TestCreateFSTestOSWrapperWithRealEnv_MatchesReal

func TestFSTestEnvironProvider_Getenv(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	wrapper.Setenv("EXISTING_VAR", "value")

	t.Run("Get existing variable", func(t *testing.T) {
		val := wrapper.Getenv("EXISTING_VAR")
		require.Equal(t, "value", val)
	})

	t.Run("Get non-existent variable", func(t *testing.T) {
		val := wrapper.Getenv("NON_EXISTENT_VAR")
		require.Empty(t, val)
	})
}

func TestFSTestEnvironProvider_Getenv_MatchesReal(t *testing.T) {
	realWrapper := oswrapper.GetRealOSWrapper()
	testWrapper := oswrapper.CreateFSTestOSWrapperWithRealEnv()

	t.Run("existing variable", func(t *testing.T) {
		// Test a variable that is likely to exist
		var existingVar string
		if runtime.GOOS == "windows" {
			existingVar = "SystemRoot"
		} else {
			existingVar = "PATH"
		}

		realVal, realExists := os.LookupEnv(existingVar)
		if !realExists {
			t.Skipf("Skipping test as environment variable '%s' is not set", existingVar)
		}

		testVal := testWrapper.Getenv(existingVar)
		require.Equal(t, realVal, testVal, "Value for existing var '%s' should match", existingVar)
		require.NotEmpty(t, realVal, "Expected existing var '%s' to be non-empty", existingVar)
	})

	t.Run("non-existent variable", func(t *testing.T) {
		// Test a variable that is unlikely to exist
		nonExistentVar := "THIS_VAR_SHOULD_NOT_EXIST_IN_TEST_ENV"
		if _, realExists := os.LookupEnv(nonExistentVar); realExists {
			t.Skipf("Skipping test as environment variable '%s' is unexpectedly set", nonExistentVar)
		}

		realVal := realWrapper.Getenv(nonExistentVar)
		testVal := testWrapper.Getenv(nonExistentVar)
		require.Equal(t, realVal, testVal, "Value for non-existent var should match")
		require.Empty(t, realVal, "Expected non-existent var to be empty")
	})
}

func TestFSTestEnvironProvider_Getwd(t *testing.T) {
	t.Run("PWD is set", func(t *testing.T) {
		wrapper := oswrapper.CreateFSTestOSWrapper()
		wrapper.Setenv("PWD", "/my/test/dir")
		wd, err := wrapper.Getwd()
		require.NoError(t, err)
		require.Equal(t, "/my/test/dir", wd)
	})

	t.Run("PWD is not set", func(t *testing.T) {
		wrapper := oswrapper.CreateFSTestOSWrapper()
		_, err := wrapper.Getwd()
		require.Error(t, err)
		require.ErrorIs(t, err, oswrapper.ErrPwdNotSet)
	})
}

func TestFSTestEnvironProvider_Getwd_MatchesReal(t *testing.T) {
	realWrapper := oswrapper.GetRealOSWrapper()
	testWrapper := oswrapper.CreateFSTestOSWrapperWithRealEnv()

	realWd, realErr := realWrapper.Getwd()
	testWd, testErr := testWrapper.Getwd()

	// os.Getwd() can fail in some environments
	if realErr != nil {
		require.Error(t, testErr)
	} else {
		require.NoError(t, testErr)
		require.Equal(t, realWd, testWd)
	}
}

func TestFSTestEnvironProvider_UserHomeDir(t *testing.T) {
	t.Run("HOME is set", func(t *testing.T) {
		wrapper := oswrapper.CreateFSTestOSWrapper()
		wrapper.Setenv("HOME", "/home/user")
		home, err := wrapper.UserHomeDir()
		require.NoError(t, err)
		require.Equal(t, "/home/user", home)
	})

	t.Run("HOME is not set", func(t *testing.T) {
		wrapper := oswrapper.CreateFSTestOSWrapper()
		_, err := wrapper.UserHomeDir()
		require.Error(t, err)
		require.ErrorIs(t, err, oswrapper.ErrHomeNotSet)
	})
}

func TestFSTestEnvironProvider_UserHomeDir_MatchesReal(t *testing.T) {
	realWrapper := oswrapper.GetRealOSWrapper()
	testWrapper := oswrapper.CreateFSTestOSWrapperWithRealEnv()

	realHome, realErr := realWrapper.UserHomeDir()
	testHome, testErr := testWrapper.UserHomeDir()

	// os.UserHomeDir can fail in some environments
	if realErr != nil {
		require.Error(t, testErr)
	} else {
		require.NoError(t, testErr)
		require.Equal(t, realHome, testHome)
	}
}
