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

package expectations

import (
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/result"
)

// Tags holds the tag information parsed in the comments between the
// 'BEGIN TAG HEADER' and 'END TAG HEADER' markers.
// Tags are grouped in tag-sets.
type Tags struct {
	// Map of tag-set name to tags
	Sets []TagSet
	// Map of tag name to tag-set and priority
	ByName map[string]TagSetAndPriority
}

// TagSet is a named collection of tags, parsed from the 'TAG HEADER'
type TagSet struct {
	Name string      // Name of the tag-set
	Tags result.Tags // Tags belonging to the tag-set
}

// TagSetAndPriority is used by the Tags.ByName map to identify which tag-set
// a tag belongs to.
type TagSetAndPriority struct {
	// The tag-set that the tag belongs to.
	Set string
	// The declared order of tag in the set.
	// An expectation may only list a single tag from any set. This priority
	// is used to decide which tag(s) should be dropped when multiple tags are
	// found in the same set.
	Priority int
}

// Clone returns a deep-copy of the Tags
func (t Tags) Clone() Tags {
	out := Tags{}
	if t.ByName != nil {
		out.ByName = make(map[string]TagSetAndPriority, len(t.ByName))
		for n, t := range t.ByName {
			out.ByName[n] = t
		}
	}
	if t.Sets != nil {
		out.Sets = make([]TagSet, len(t.Sets))
		copy(out.Sets, t.Sets)
	}
	return out
}

// RemoveLowerPriorityTags returns a copy of the provided tags with only the
// highest priority tag of each tag set.
func (t Tags) RemoveLowerPriorityTags(tags result.Tags) result.Tags {
	return t.removeXPriorityTagsImpl(tags, func(p1, p2 int) bool { return p1 < p2 })
}

// RemoveHigherPriorityTags returns a copy of the provided tags with only the
// lowest priority tag of each tag set.
func (t Tags) RemoveHigherPriorityTags(tagsToReduce result.Tags) result.Tags {
	return t.removeXPriorityTagsImpl(tagsToReduce, func(p1, p2 int) bool { return p1 > p2 })
}

func (t Tags) removeXPriorityTagsImpl(tagsToReduce result.Tags, priorityComparison func(int, int) bool) result.Tags {
	type TagPriority struct {
		tag      string
		priority int
	}
	highestPriorty := container.NewMap[string, *TagPriority]()
	for tag := range tagsToReduce {
		info := t.ByName[tag]
		tp := highestPriorty[info.Set]
		if tp == nil {
			tp = &TagPriority{tag: tag, priority: info.Priority}
			highestPriorty[info.Set] = tp
		} else if priorityComparison(tp.priority, info.Priority) {
			tp.tag, tp.priority = tag, info.Priority
		}
	}
	out := container.Set[string]{}
	for _, tp := range highestPriorty {
		out.Add(tp.tag)
	}
	return out
}
