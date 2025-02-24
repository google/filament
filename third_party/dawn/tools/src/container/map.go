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
	"sort"
	"sync"
)

// Map is a generic unordered map, which wrap's go's builtin 'map'.
// K is the map key, which must match the 'key' constraint.
// V is the map value, which can be any type.
type Map[K key, V any] map[K]V

// Returns a new empty map
func NewMap[K key, V any]() Map[K, V] {
	return make(Map[K, V])
}

// Add adds an item to the map.
func (m Map[K, V]) Add(k K, v V) {
	m[k] = v
}

// Remove removes an item from the map
func (m Map[K, V]) Remove(item K) {
	delete(m, item)
}

// Contains returns true if the map contains the given item
func (m Map[K, V]) Contains(item K) bool {
	_, found := m[item]
	return found
}

// Keys returns the sorted keys of the map as a slice
func (m Map[K, V]) Keys() []K {
	out := make([]K, 0, len(m))
	for v := range m {
		out = append(out, v)
	}
	sort.Slice(out, func(i, j int) bool { return out[i] < out[j] })
	return out
}

// Values returns the values of the map sorted by key
func (m Map[K, V]) Values() []V {
	out := make([]V, 0, len(m))
	for _, k := range m.Keys() {
		out = append(out, m[k])
	}
	return out
}

// GetOrCreate returns the value of the map entry with the given key, creating
// the map entry with create() if the entry did not exist.
func (m Map[K, V]) GetOrCreate(key K, create func() V) V {
	value, ok := m[key]
	if !ok {
		value = create()
		m[key] = value
	}
	return value
}

// GetOrCreateLocked is similar to GetOrCreate, but performs lookup with a
// read-lock on the provided mutex, and a write lock on create() and map
// insertion.
func (m Map[K, V]) GetOrCreateLocked(mutex *sync.RWMutex, key K, create func() V) V {
	mutex.RLock()
	value, ok := m[key]
	mutex.RUnlock()
	if !ok {
		mutex.Lock()
		defer mutex.Unlock()
		value, ok = m[key]
		if !ok {
			value = create()
			m[key] = value
		}
	}
	return value
}
