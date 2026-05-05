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

package treemap

import (
	"context"
	"encoding/json"
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/stretchr/testify/require"
	"go.chromium.org/luci/auth"
)

func TestDurations(t *testing.T) {
	d := durations{sum: 10 * time.Second, count: 2}

	// Test add
	d2 := d.add(5 * time.Second)
	require.Equal(t, 15*time.Second, d2.sum)
	require.Equal(t, 3, d2.count)

	// Test average
	require.Equal(t, 5*time.Second, d.average())
	require.Equal(t, 5*time.Second, d2.average())

	d3 := durations{sum: 0, count: 0}
	require.Equal(t, time.Duration(0), d3.average())
}

func TestParentOf(t *testing.T) {
	require.Equal(t, "webgpu:api,operation,command_buffer,copyBufferToBuffer",
		parentOf("webgpu:api,operation,command_buffer,copyBufferToBuffer:copy_one_buffer_to_another"))
	require.Equal(t, "webgpu:api", parentOf("webgpu:api,operation"))
	require.Equal(t, "webgpu", parentOf("webgpu:test"))
	require.Equal(t, "", parentOf("webgpu"))
}

func TestParentOfOrRoot(t *testing.T) {
	require.Equal(t, "root", parentOfOrRoot("webgpu"))
	require.Equal(t, "webgpu", parentOfOrRoot("webgpu:test"))
	require.Equal(t, "webgpu:a", parentOfOrRoot("webgpu:a,b"))
}

func TestLoadTimingData_Success(t *testing.T) {
	ctx := context.Background()
	wrapper := oswrapper.CreateFSTestOSWrapper()

	// Setup mock result file content
	resultsContent := `webgpu:test,case="1" Pass 100ms false
webgpu:test,case="2" Pass 200ms false
core
`
	err := wrapper.WriteFile("results.txt", []byte(resultsContent), 0644)
	require.NoError(t, err)

	cfg := common.Config{
		OsWrapper: wrapper,
	}

	src := common.ResultSource{
		File: "results.txt",
	}

	authOpts := auth.Options{}

	data, err := loadTimingData(ctx, src, cfg, authOpts)
	require.NoError(t, err)

	type treemapJSON struct {
		Desc  string          `json:"desc"`
		Limit int             `json:"limit"`
		Data  [][]interface{} `json:"data"`
	}

	var tmData treemapJSON
	err = json.Unmarshal([]byte(data), &tmData)
	require.NoError(t, err)

	require.Contains(t, tmData.Desc, "Treemap visualization of the CTS timings")
	require.Equal(t, 1000, tmData.Limit)

	findRow := func(key string) []interface{} {
		for _, row := range tmData.Data {
			if str, ok := row[0].(string); ok && str == key {
				return row
			}
		}
		return nil
	}

	// Verify Header
	require.Equal(t, "Query", tmData.Data[0][0])
	require.Equal(t, "Parent", tmData.Data[0][1])

	// Verify Root
	rootRow := findRow("root")
	require.NotNil(t, rootRow)
	require.Nil(t, rootRow[1])
	require.EqualValues(t, 0, rootRow[2])
	require.EqualValues(t, 0, rootRow[3])

	// Verify webgpu
	// ["webgpu", "root", 150, 150]
	// 100+200 / 2 = 150ms
	webgpuRow := findRow("webgpu")
	require.NotNil(t, webgpuRow)
	require.Equal(t, "root", webgpuRow[1])
	require.EqualValues(t, 150, webgpuRow[2])
	require.EqualValues(t, 150, webgpuRow[3])

	// Verify webgpu:test
	// ["webgpu:test", "webgpu", 150, 150]
	testRow := findRow("webgpu:test")
	require.NotNil(t, testRow)
	require.Equal(t, "webgpu", testRow[1])
	require.EqualValues(t, 150, testRow[2])
	require.EqualValues(t, 150, testRow[3])
}

func TestLoadTimingData_ErrorGettingResults(t *testing.T) {
	ctx := context.Background()
	wrapper := oswrapper.CreateFSTestOSWrapper()
	// No file written.

	cfg := common.Config{
		OsWrapper: wrapper,
	}

	src := common.ResultSource{
		File: "missing_results.txt",
	}

	authOpts := auth.Options{}

	_, err := loadTimingData(ctx, src, cfg, authOpts)
	require.ErrorContains(t, err, "file does not exist")
}
