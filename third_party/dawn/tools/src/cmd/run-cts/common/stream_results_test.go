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

package common

import (
	"bytes"
	"sync"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cov"
	"github.com/stretchr/testify/require"
)

func TestCloseChanOnWaitGroupDone(t *testing.T) {
	wg := sync.WaitGroup{}
	c := make(chan bool, 1)

	wg.Add(1)
	CloseChanOnWaitGroupDone(&wg, c)
	wg.Done()

	_, ok := <-c
	require.False(t, ok)
}

func TestPercentage(t *testing.T) {
	require.Equal(t, "50.0%", percentage(1, 2))
	require.Equal(t, "100.0%", percentage(2, 2))
	require.Equal(t, "0.0%", percentage(0, 2))
	require.Equal(t, "33.3%", percentage(1, 3))
	require.Equal(t, "66.7%", percentage(2, 3))
	require.Equal(t, "-", percentage(1, 0))
}

func TestAlignLeft(t *testing.T) {
	require.Equal(t, "hello     ", alignLeft("hello", 10))
	require.Equal(t, "hello", alignLeft("hello", 5))
	require.Equal(t, "hello", alignLeft("hello", 3))
}

func TestAlignRight(t *testing.T) {
	require.Equal(t, "     hello", alignRight("hello", 10))
	require.Equal(t, "hello", alignRight("hello", 5))
	require.Equal(t, "hello", alignRight("hello", 3))
}

func TestDrawProgressBar(t *testing.T) {
	// Not a lot to test here, just that it doesn't crash.
	// The output is heavily dependent on terminal support.
	buf := &bytes.Buffer{}
	numByExpectedStatus := map[expectedStatus]int{
		{status: Pass, expected: true}:  10,
		{status: Fail, expected: false}: 2,
		{status: Skip, expected: true}:  5,
	}
	drawProgressBar(buf, 0, 17, true, numByExpectedStatus)
	drawProgressBar(buf, 0, 17, false, numByExpectedStatus)
}

func TestSplitCTSQuery(t *testing.T) {
	require.Equal(t,
		cov.Path{
			"webgpu:",
			"shader,",
			"execution,",
			"expression,",
			"call,",
			"builtin,",
			"acos:",
			"f32:",
			`inputSource="storage_r";vectorize=4`,
		},
		splitCTSQuery(`webgpu:shader,execution,expression,call,builtin,acos:f32:inputSource="storage_r";vectorize=4`),
	)
}
