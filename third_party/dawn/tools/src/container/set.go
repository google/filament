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

package container

import (
	"fmt"
	"sort"
)

// Set is a generic unordered set, which wrap's go's builtin 'map'.
// T is the set key, which must match the 'key' constraint.
type Set[T key] map[T]struct{}

// Returns a new set with the give items
func NewSet[T key](items ...T) Set[T] {
	out := make(Set[T])
	for _, item := range items {
		out.Add(item)
	}
	return out
}

// Clone returns a new Set populated with s
func (s Set[T]) Clone() Set[T] {
	out := make(Set[T], len(s))
	for item := range s {
		out.Add(item)
	}
	return out
}

// Add adds an item to the set.
func (s Set[T]) Add(item T) {
	s[item] = struct{}{}
}

// AddAll adds all the items of o to the set.
func (s Set[T]) AddAll(o Set[T]) {
	for item := range o {
		s.Add(item)
	}
}

// Remove removes an item from the set
func (s Set[T]) Remove(item T) {
	delete(s, item)
}

// RemoveAll removes all the items of o from the set.
func (s Set[T]) RemoveAll(o Set[T]) {
	for item := range o {
		s.Remove(item)
	}
}

// Contains returns true if the set contains the given item
func (s Set[T]) Contains(item T) bool {
	_, found := s[item]
	return found
}

// ContainsAll returns true if the set contains all the items in o
func (s Set[T]) ContainsAll(o Set[T]) bool {
	for item := range o {
		if !s.Contains(item) {
			return false
		}
	}
	return true
}

// ContainsAny returns true if the set contains any of the items in o
func (s Set[T]) ContainsAny(o Set[T]) bool {
	for item := range o {
		if s.Contains(item) {
			return true
		}
	}
	return false
}

// Intersection returns true if the set contains all the items in o
func (s Set[T]) Intersection(o Set[T]) Set[T] {
	out := NewSet[T]()
	for item := range o {
		if s.Contains(item) {
			out.Add(item)
		}
	}
	return out
}

// List returns the sorted entries of the set as a slice
func (s Set[T]) List() []T {
	out := make([]T, 0, len(s))
	for v := range s {
		out = append(out, v)
	}
	sort.Slice(out, func(i, j int) bool { return out[i] < out[j] })
	return out
}

// One returns a random item from the set, or an empty item if the set is empty.
func (s Set[T]) One() T {
	for item := range s {
		return item
	}
	var zero T
	return zero
}

// Format writes the Target to the fmt.State
func (s Set[T]) Format(f fmt.State, verb rune) {
	fmt.Fprint(f, "[")
	for i, item := range s.List() {
		if i > 0 {
			fmt.Fprint(f, ", ")
		}
		fmt.Fprint(f, item)
	}
	fmt.Fprint(f, "]")
}
