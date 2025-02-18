// Copyright 2023 The Dawn & Tint Authors
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

package transform

import (
	"runtime"
	"sync"
)

// Map returns a new map by transforming each entry with the function fn
func Map[IN_KEY comparable, IN_VAL any, OUT_KEY comparable, OUT_VAL any](in map[IN_KEY]IN_VAL, fn func(k IN_KEY, v IN_VAL) (OUT_KEY, OUT_VAL, error)) (map[OUT_KEY]OUT_VAL, error) {
	out := make(map[OUT_KEY]OUT_VAL, len(in))
	errs := Errors{}
	for inKey, inValue := range in {
		outKey, outVal, err := fn(inKey, inValue)
		if err != nil {
			errs = append(errs, err)
		}
		out[outKey] = outVal
	}

	if len(errs) > 0 {
		return nil, errs
	}

	return out, nil
}

type keyVal[K comparable, V any] struct {
	key K
	val V
}

type keyValErr[K comparable, V any] struct {
	key K
	val V
	err error
}

// GoMap returns a new map by transforming each element with the function
// fn, called by multiple go-routines.
func GoMap[IN_KEY comparable, IN_VAL any, OUT_KEY comparable, OUT_VAL any](in map[IN_KEY]IN_VAL, fn func(k IN_KEY, v IN_VAL) (OUT_KEY, OUT_VAL, error)) (map[OUT_KEY]OUT_VAL, error) {
	// Create a channel of input key-value pairs
	tasks := make(chan keyVal[IN_KEY, IN_VAL], 256)
	go func() {
		for k, v := range in {
			tasks <- keyVal[IN_KEY, IN_VAL]{k, v}
		}
		close(tasks)
	}()

	// Kick a number of workers to process the elements
	results := make(chan keyValErr[OUT_KEY, OUT_VAL], 256)
	go func() {
		numWorkers := runtime.NumCPU()
		wg := sync.WaitGroup{}
		wg.Add(numWorkers)
		for worker := 0; worker < numWorkers; worker++ {
			go func() {
				defer wg.Done()
				for task := range tasks {
					outKey, outValue, err := fn(task.key, task.val)
					results <- keyValErr[OUT_KEY, OUT_VAL]{
						key: outKey,
						val: outValue,
						err: err,
					}
				}
			}()
		}
		wg.Wait()
		close(results)
	}()

	out := make(map[OUT_KEY]OUT_VAL, len(in))
	errs := Errors{}
	for res := range results {
		if res.err != nil {
			errs = append(errs, res.err)
		} else {
			out[res.key] = res.val
		}
	}

	if len(errs) > 0 {
		return nil, errs
	}

	return out, nil
}
