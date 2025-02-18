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

package query

import (
	"errors"
	"fmt"
	"io"
	"sort"
)

// Tree holds a tree structure of Query to generic Data type.
// Each separate suite, file, test of the query produces a separate tree node.
// All cases of the query produce a single leaf tree node.
type Tree[Data any] struct {
	TreeNode[Data]
}

// TreeNode is a single node in the Tree
type TreeNode[Data any] struct {
	// The full query of the node
	Query Query
	// The data associated with this node. nil is used to represent no-data.
	Data *Data
	// Children of the node. Keyed by query.Target and name.
	Children TreeNodeChildren[Data]
}

// TreeNodeChildKey is the key used by TreeNode for the Children map
type TreeNodeChildKey struct {
	// The child name. This is the string between `:` and `,` delimiters.
	// Note: that all test cases are held by a single TreeNode.
	Name string
	// The target type of the child. Examples:
	//  Query           |  Target of 'child'
	// -----------------+--------------------
	// parent:child     | Files
	// parent:x,child   | Files
	// parent:x:child   | Test
	// parent:x:y,child | Test
	// parent:x:y:child | Cases
	//
	// It's possible to have a directory and '.spec.ts' share the same name,
	// hence why we include the Target as part of the child key.
	Target Target
}

// TreeNodeChildren is a map of TreeNodeChildKey to TreeNode pointer.
// Data is the data type held by a TreeNode.
type TreeNodeChildren[Data any] map[TreeNodeChildKey]*TreeNode[Data]

// sortedChildKeys returns all the sorted children keys.
func (n *TreeNode[Data]) sortedChildKeys() []TreeNodeChildKey {
	keys := make([]TreeNodeChildKey, 0, len(n.Children))
	for key := range n.Children {
		keys = append(keys, key)
	}
	sort.Slice(keys, func(i, j int) bool {
		a, b := keys[i], keys[j]
		switch {
		case a.Name < b.Name:
			return true
		case a.Name > b.Name:
			return false
		case a.Target < b.Target:
			return true
		case a.Target > b.Target:
			return false
		}
		return false
	})
	return keys
}

// traverse performs a depth-first-search of the tree calling f for each visited
// node, starting with n, then visiting each of children in sorted order
// (pre-order traversal).
func (n *TreeNode[Data]) traverse(f func(n *TreeNode[Data]) error) error {
	if err := f(n); err != nil {
		return err
	}
	for _, key := range n.sortedChildKeys() {
		if err := n.Children[key].traverse(f); err != nil {
			return err
		}
	}
	return nil
}

// print writes a textual representation of this node and its children to w.
// prefix is used as the line prefix for each node, which is appended with
// whitespace for each child node.
func (n *TreeNode[Data]) print(w io.Writer, prefix string) {
	fmt.Fprintf(w, "%v{\n", prefix)
	fmt.Fprintf(w, "%v  query: '%v'\n", prefix, n.Query)
	fmt.Fprintf(w, "%v  data:  '%v'\n", prefix, n.Data)
	for _, key := range n.sortedChildKeys() {
		n.Children[key].print(w, prefix+"  ")
	}
	fmt.Fprintf(w, "%v}\n", prefix)
}

// Format implements the io.Formatter interface.
// See https://pkg.go.dev/fmt#Formatter
func (n *TreeNode[Data]) Format(f fmt.State, verb rune) {
	n.print(f, "")
}

// getOrCreateChild returns the child with the given key if it exists,
// otherwise the child node is created and added to n and is returned.
func (n *TreeNode[Data]) getOrCreateChild(key TreeNodeChildKey) *TreeNode[Data] {
	if n.Children == nil {
		child := &TreeNode[Data]{Query: n.Query.Append(key.Target, key.Name)}
		n.Children = TreeNodeChildren[Data]{key: child}
		return child
	}
	if child, ok := n.Children[key]; ok {
		return child
	}
	child := &TreeNode[Data]{Query: n.Query.Append(key.Target, key.Name)}
	n.Children[key] = child
	return child
}

// QueryData is a pair of a Query and a generic Data type.
// Used by NewTree for constructing a tree with entries.
type QueryData[Data any] struct {
	Query Query
	Data  Data
}

// NewTree returns a new Tree populated with the given entries.
// If entries returns duplicate queries, then ErrDuplicateData will be returned.
func NewTree[Data any](entries ...QueryData[Data]) (Tree[Data], error) {
	out := Tree[Data]{}
	for _, qd := range entries {
		if err := out.Add(qd.Query, qd.Data); err != nil {
			return Tree[Data]{}, err
		}
	}
	return out, nil
}

// Add adds a new data to the tree.
// Returns ErrDuplicateData if the tree already contains a data for the given node at query
func (t *Tree[Data]) Add(q Query, d Data) error {
	node := &t.TreeNode
	q.Walk(func(q Query, t Target, n string) error {
		node = node.getOrCreateChild(TreeNodeChildKey{n, t})
		return nil
	})
	if node.Data != nil {
		return ErrDuplicateData{node.Query}
	}
	node.Data = &d
	return nil
}

// GetOrCreate returns existing, or adds a new data to the tree.
func (t *Tree[Data]) GetOrCreate(q Query, create func() Data) *Data {
	node := &t.TreeNode
	q.Walk(func(q Query, t Target, n string) error {
		node = node.getOrCreateChild(TreeNodeChildKey{n, t})
		return nil
	})
	if node.Data == nil {
		data := create()
		node.Data = &data
	}
	return node.Data
}

// errStop is an error used to stop traversal of Query.Walk
var errStop = errors.New("stop")

// Get returns the closest existing tree node for the given query
func (t *Tree[Data]) Get(q Query) *TreeNode[Data] {
	node := &t.TreeNode
	q.Walk(func(q Query, t Target, n string) error {
		if n := node.Children[TreeNodeChildKey{n, t}]; n != nil {
			node = n
			return nil
		}
		return errStop
	})
	return node
}

// glob calls f for every node under the given query.
func (t *Tree[Data]) glob(fq Query, f func(f *TreeNode[Data]) error) error {
	node := &t.TreeNode
	return fq.Walk(func(q Query, t Target, n string) error {
		if n == "*" {
			// Wildcard reached.
			// Glob the parent, but restrict to the wildcard target type.
			for _, key := range node.sortedChildKeys() {
				child := node.Children[key]
				if child.Query.Target() == t {
					if err := child.traverse(f); err != nil {
						return err
					}
				}
			}
			return nil
		}
		switch t {
		case Suite, Files, Tests:
			child, ok := node.Children[TreeNodeChildKey{n, t}]
			if !ok {
				return ErrNoDataForQuery{q}
			}
			node = child
		case Cases:
			for _, key := range node.sortedChildKeys() {
				child := node.Children[key]
				if child.Query.Contains(fq) {
					if err := f(child); err != nil {
						return err
					}
				}
			}
			return nil
		}
		if q == fq {
			return node.traverse(f)
		}
		return nil
	})
}

// Glob returns a list of QueryData's for every node that is under the given
// query, which holds data.
// Glob handles wildcards as well as non-wildcard queries:
//   - A non-wildcard query will match the node itself, along with every node
//     under the query. For example: 'a:b' will match every File and Test
//     node under 'a:b', including 'a:b' itself.
//   - A wildcard Query will include every node under the parent node with the
//     matching Query target. For example: 'a:b:*' will match every Test
//     node (excluding File nodes) under 'a:b', 'a:b' will not be included.
func (t *Tree[Data]) Glob(q Query) ([]QueryData[Data], error) {
	out := []QueryData[Data]{}
	err := t.glob(q, func(n *TreeNode[Data]) error {
		if n.Data != nil {
			out = append(out, QueryData[Data]{n.Query, *n.Data})
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	return out, nil
}
