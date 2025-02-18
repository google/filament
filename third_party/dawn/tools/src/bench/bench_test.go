// Copyright 2022 The Dawn & Tint Authors
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

package bench_test

import (
	"reflect"
	"testing"
	"time"

	"dawn.googlesource.com/dawn/tools/src/bench"
)

func TestParseJson(t *testing.T) {
	json := `
	{
		"context": {
		  "date": "2022-01-24T10:28:13+00:00",
		  "host_name": "hostname",
		  "executable": "./myexe",
		  "num_cpus": 16,
		  "mhz_per_cpu": 2400,
		  "cpu_scaling_enabled": false,
		  "caches": [
			{
			  "type": "Data",
			  "level": 1,
			  "size": 32768,
			  "num_sharing": 2
			},
			{
			  "type": "Instruction",
			  "level": 1,
			  "size": 32768,
			  "num_sharing": 2
			},
			{
			  "type": "Unified",
			  "level": 2,
			  "size": 262144,
			  "num_sharing": 2
			},
			{
			  "type": "Unified",
			  "level": 3,
			  "size": 16777216,
			  "num_sharing": 16
			}
		  ],
		  "load_avg": [2.60938,2.59863,2.55566],
		  "library_build_type": "release"
		},
		"benchmarks": [
		  {
			"name": "MyBenchmark",
			"family_index": 0,
			"per_family_instance_index": 0,
			"run_name": "MyBenchmark",
			"run_type": "iteration",
			"repetitions": 2,
			"repetition_index": 0,
			"threads": 1,
			"iterations": 402,
			"real_time": 1.6392272438353568e+06,
			"cpu_time": 1.6387412935323382e+06,
			"time_unit": "ns"
		  },
		  {
			"name": "MyBenchmark",
			"family_index": 0,
			"per_family_instance_index": 0,
			"run_name": "MyBenchmark",
			"run_type": "iteration",
			"repetitions": 2,
			"repetition_index": 1,
			"threads": 1,
			"iterations": 402,
			"real_time": 1.7143936117703272e+06,
			"cpu_time": 1.7124004975124374e+06,
			"time_unit": "ns"
		  },
		  {
			"name": "MyBenchmark_mean",
			"family_index": 0,
			"per_family_instance_index": 0,
			"run_name": "MyBenchmark",
			"run_type": "aggregate",
			"repetitions": 2,
			"threads": 1,
			"aggregate_name": "mean",
			"iterations": 2,
			"real_time": 1.6768104278028419e+06,
			"cpu_time": 1.6755708955223879e+06,
			"time_unit": "ns"
		  },
		  {
			"name": "MyBenchmark_median",
			"family_index": 0,
			"per_family_instance_index": 0,
			"run_name": "MyBenchmark",
			"run_type": "aggregate",
			"repetitions": 2,
			"threads": 1,
			"aggregate_name": "median",
			"iterations": 2,
			"real_time": 1.6768104278028419e+06,
			"cpu_time": 1.6755708955223879e+06,
			"time_unit": "ns"
		  },
		  {
			"name": "MyBenchmark_stddev",
			"family_index": 0,
			"per_family_instance_index": 0,
			"run_name": "MyBenchmark",
			"run_type": "aggregate",
			"repetitions": 2,
			"threads": 1,
			"aggregate_name": "stddev",
			"iterations": 2,
			"real_time": 5.3150648483981553e+04,
			"cpu_time": 5.2084922631119407e+04,
			"time_unit": "ns"
		  }
		]
	  }
`
	got, err := bench.Parse(json)
	if err != nil {
		t.Errorf("bench.Parse() returned %v", err)
		return
	}

	expectedDate, err := time.Parse(time.RFC3339, "2022-01-24T10:28:13+00:00")
	if err != nil {
		t.Errorf("time.Parse() returned %v", err)
		return
	}

	expect := bench.Run{
		Benchmarks: []bench.Benchmark{
			{Name: "MyBenchmark", Duration: time.Nanosecond * 1638741, AggregateType: ""},
			{Name: "MyBenchmark", Duration: time.Nanosecond * 1712400, AggregateType: ""},
			{Name: "MyBenchmark", Duration: time.Nanosecond * 1675570, AggregateType: "mean"},
			{Name: "MyBenchmark", Duration: time.Nanosecond * 1675570, AggregateType: "median"},
			{Name: "MyBenchmark", Duration: time.Nanosecond * 52084, AggregateType: "stddev"},
		},
		Context: &bench.Context{
			Date:       expectedDate,
			HostName:   "hostname",
			Executable: "./myexe",
			NumCPUs:    16,
			MhzPerCPU:  2400, CPUScalingEnabled: false,
			Caches: []bench.ContextCache{
				{Type: "Data", Level: 1, Size: 32768, NumSharing: 2},
				{Type: "Instruction", Level: 1, Size: 32768, NumSharing: 2},
				{Type: "Unified", Level: 2, Size: 262144, NumSharing: 2},
				{Type: "Unified", Level: 3, Size: 16777216, NumSharing: 16},
			},
			LoadAvg: []float32{2.60938, 2.59863, 2.55566}, LibraryBuildType: "release"},
	}

	expectEqual(t, "bench.Parse().Benchmarks", got.Benchmarks, expect.Benchmarks)
	expectEqual(t, "bench.Parse().Context", got.Context, expect.Context)
}

func TestCompare(t *testing.T) {
	a := []bench.Benchmark{
		{Name: "MyBenchmark1", Duration: time.Nanosecond * 1714393},
		{Name: "MyBenchmark0", Duration: time.Nanosecond * 1639227},
		{Name: "MyBenchmark3", Duration: time.Nanosecond * 1676810},
		{Name: "MyBenchmark4", Duration: time.Nanosecond * 53150},
		{Name: "MyBenchmark2", Duration: time.Nanosecond * 1676810},
	}
	b := []bench.Benchmark{
		{Name: "MyBenchmark1", Duration: time.Nanosecond * 56747654},
		{Name: "MyBenchmark0", Duration: time.Nanosecond * 236246},
		{Name: "MyBenchmark2", Duration: time.Nanosecond * 675865},
		{Name: "MyBenchmark4", Duration: time.Nanosecond * 2352336},
		{Name: "MyBenchmark3", Duration: time.Nanosecond * 87657868},
	}

	minDiff := time.Millisecond * 2
	minRelDiff := 35.0

	cmp := bench.Compare(a, b, minDiff, minRelDiff)

	expectEqual(t, "bench.Compare().Format", cmp.Format(bench.DiffFormat{}), "")
	expectEqual(t, "bench.Compare().Format", "\n"+cmp.Format(bench.DiffFormat{TimeA: true}), `
+-----------+
| A         |
+-----------+
| 1.67681ms |
| 53.15µs   |
+-----------+
`)
	expectEqual(t, "bench.Compare().Format", "\n"+cmp.Format(bench.DiffFormat{TimeA: true, TimeB: true}), `
+-----------+-------------+
| A         | B           |
+-----------+-------------+
| 1.67681ms | 87.657868ms |
| 53.15µs   | 2.352336ms  |
+-----------+-------------+
`)
	expectEqual(t, "bench.Compare().Format", "\n"+cmp.Format(bench.DiffFormat{
		TestName:        true,
		Delta:           true,
		PercentChangeAB: true,
		TimeA:           true,
		TimeB:           true,
	}), `
+--------------+-------------+-----------+-----------+-------------+
| Test name    | Δ (A → B)   | % (A → B) | A         | B           |
+--------------+-------------+-----------+-----------+-------------+
| MyBenchmark3 | 85.981058ms | +5127.7%  | 1.67681ms | 87.657868ms |
| MyBenchmark4 | 2.299186ms  | +4325.8%  | 53.15µs   | 2.352336ms  |
+--------------+-------------+-----------+-----------+-------------+
`)
	expectEqual(t, "bench.Compare().Format", "\n"+cmp.Format(bench.DiffFormat{
		TestName:           true,
		Delta:              true,
		PercentChangeAB:    true,
		PercentChangeBA:    true,
		MultiplierChangeAB: true,
		MultiplierChangeBA: true,
		TimeA:              true,
		TimeB:              true,
	}), `
+--------------+-------------+-----------+-----------+-----------+-----------+-----------+-------------+
| Test name    | Δ (A → B)   | % (A → B) | % (B → A) | × (A → B) | × (B → A) | A         | B           |
+--------------+-------------+-----------+-----------+-----------+-----------+-----------+-------------+
| MyBenchmark3 | 85.981058ms | +5127.7%  | -98.1%    | +52.2766  | +0.0191   | 1.67681ms | 87.657868ms |
| MyBenchmark4 | 2.299186ms  | +4325.8%  | -97.7%    | +44.2584  | +0.0226   | 53.15µs   | 2.352336ms  |
+--------------+-------------+-----------+-----------+-----------+-----------+-----------+-------------+
`)
}

func expectEqual(t *testing.T, desc string, got, expect interface{}) {
	if !reflect.DeepEqual(got, expect) {
		t.Errorf("%v was not expected:\nGot:\n%+v\nExpected:\n%+v", desc, got, expect)
	}
}
