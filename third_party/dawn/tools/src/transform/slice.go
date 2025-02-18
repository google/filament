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

// Filter returns a new slice of T with all the items that match the given predicate
func Filter[T any](items []T, pred func(T) bool) []T {
	out := make([]T, 0, len(items))
	for _, item := range items {
		if pred(item) {
			out = append(out, item)
		}
	}
	return out
}

// Flatten takes a slice of slices, and returns a linearized slice
func Flatten[T any, S ~[]T](lists []S) S {
	flat := S{}
	for _, list := range lists {
		flat = append(flat, list...)
	}
	return flat
}

// Slice returns a new slice by transforming each element with the function fn
func Slice[IN any, OUT any](in []IN, fn func(in IN) (OUT, error)) ([]OUT, error) {
	out := make([]OUT, len(in))
	for i, el := range in {
		o, err := fn(el)
		if err != nil {
			return nil, err
		}
		out[i] = o
	}

	return out, nil
}

// SliceNoErr returns a new slice by transforming each element with the function fn
func SliceNoErr[IN any, OUT any](in []IN, fn func(in IN) OUT) []OUT {
	out := make([]OUT, len(in))
	for i, el := range in {
		out[i] = fn(el)
	}
	return out
}

// GoSlice returns a new slice by transforming each element with the function
// fn, called by multiple go-routines.
func GoSlice[IN any, OUT any](in []IN, fn func(in IN) (OUT, error)) ([]OUT, error) {
	// Create a channel of indices
	indices := make(chan int, 256)
	go func() {
		for i := range in {
			indices <- i
		}
		close(indices)
	}()

	out := make([]OUT, len(in))
	errs := make(Errors, len(in))

	// Kick a number of workers to process the elements
	numWorkers := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numWorkers)
	for worker := 0; worker < numWorkers; worker++ {
		go func() {
			defer wg.Done()
			for idx := range indices {
				out[idx], errs[idx] = fn(in[idx])
			}
		}()
	}
	wg.Wait()

	errs = Filter(errs, func(e error) bool { return e != nil })
	if len(errs) > 0 {
		return nil, errs
	}

	return out, nil
}

// GoSliceNoErr returns a new slice by transforming each element with the function
// fn, called by multiple go-routines.
func GoSliceNoErr[IN any, OUT any](in []IN, fn func(in IN) OUT) []OUT {
	// Create a channel of indices
	indices := make(chan int, 256)
	go func() {
		for i := range in {
			indices <- i
		}
		close(indices)
	}()

	out := make([]OUT, len(in))

	// Kick a number of workers to process the elements
	numWorkers := runtime.NumCPU()
	wg := sync.WaitGroup{}
	wg.Add(numWorkers)
	for worker := 0; worker < numWorkers; worker++ {
		go func() {
			defer wg.Done()
			for idx := range indices {
				out[idx] = fn(in[idx])
			}
		}()
	}
	wg.Wait()

	return out
}

// SliceToChan returns a new chan populated with all the items in slice.
// The chan is closed after being populated.
func SliceToChan[T any](slice []T) <-chan T {
	c := make(chan T, 256)
	go func() {
		for _, el := range slice {
			c <- el
		}
		close(c)
	}()
	return c
}
