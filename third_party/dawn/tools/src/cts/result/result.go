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

// Package result holds types that describe CTS test results.
package result

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/cts/query"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
)

// Result holds the result of a CTS test
type Result struct {
	Query    query.Query
	Tags     Tags
	Status   Status
	Duration time.Duration
	// If true, this result may be exonerated if there are other
	// results with the same query and tags that have MayExonerate: false
	MayExonerate bool
}

// Format writes the Result to the fmt.State
// The Result is printed as a single line, in the form:
//
//	<query> <tags> <status>
//
// This matches the order in which results are sorted.
func (r Result) Format(f fmt.State, verb rune) {
	if len(r.Tags) > 0 {
		fmt.Fprintf(f, "%v %v %v %v %v", r.Query, TagsToString(r.Tags), r.Status, r.Duration, r.MayExonerate)
	} else {
		fmt.Fprintf(f, "%v %v %v %v", r.Query, r.Status, r.Duration, r.MayExonerate)
	}
}

// String returns the result as a string
func (r Result) String() string {
	sb := strings.Builder{}
	fmt.Fprint(&sb, r)
	return sb.String()
}

// Compare compares the relative order of r and o, returning:
//
//	-1 if r should come before o
//	 1 if r should come after o
//	 0 if r and o are identical
//
// Note: Result.Duration is not considered in comparison.
func (r Result) Compare(o Result) int {
	a, b := r, o
	switch a.Query.Compare(b.Query) {
	case -1:
		return -1
	case 1:
		return 1
	}
	ta := strings.Join(a.Tags.List(), TagDelimiter)
	tb := strings.Join(b.Tags.List(), TagDelimiter)
	switch {
	case ta < tb:
		return -1
	case ta > tb:
		return 1
	case a.Status < b.Status:
		return -1
	case a.Status > b.Status:
		return 1
	}
	return 0
}

// Parse parses the result from a string of the form:
//
//	<query> <tags> <status>
//
// <tags> may be omitted if there were no tags.
//
// Tests are separated into sections where the section name
// appears at the end of the list as just a line with
//
//	<section-name>
func Parse(in string) (ExecutionMode, Result, error) {
	line := in
	token := func() string {
		for i, c := range line {
			if c != ' ' {
				line = line[i:]
				break
			}
		}
		for i, c := range line {
			if c == ' ' {
				tok := line[:i]
				line = line[i:]
				return tok
			}
		}
		tok := line
		line = ""
		return tok
	}

	a := token()
	b := token()
	c := token()
	d := token()
	e := token()

	if a != "" && b == "" && token() == "" {
		return ExecutionMode(a), Result{}, nil
	}
	if a == "" || b == "" || c == "" || d == "" || token() != "" {
		return "", Result{}, fmt.Errorf("unable to parse result '%v'", in)
	}

	query := query.Parse(a)

	if e == "" {
		status := Status(b)
		duration, err := time.ParseDuration(c)
		if err != nil {
			return "", Result{}, fmt.Errorf("unable to parse result '%v': %w", in, err)
		}
		mayExonerate, err := strconv.ParseBool(d)
		if err != nil {
			return "", Result{}, fmt.Errorf("unable to parse result '%v': %w", in, err)
		}
		return "", Result{query, nil, status, duration, mayExonerate}, nil
	} else {
		tags := StringToTags(b)
		status := Status(c)
		duration, err := time.ParseDuration(d)
		if err != nil {
			return "", Result{}, fmt.Errorf("unable to parse result '%v': %w", in, err)
		}
		mayExonerate, err := strconv.ParseBool(e)
		if err != nil {
			return "", Result{}, fmt.Errorf("unable to parse result '%v': %w", in, err)
		}
		return "", Result{query, tags, status, duration, mayExonerate}, nil
	}
}

// List is a list of results
type List []Result

// The mode the tests were run in, "core" or" "compat"
type ExecutionMode string

// Lists of test results by execution mode.
type ResultsByExecutionMode map[ExecutionMode]List

// TransformTags returns the list of results with the tags transformed using f.
// TransformTags assumes that f will return the same output for the same input.
func (l List) TransformTags(f func(Tags) Tags) List {
	cache := map[string]Tags{}
	out := List{}
	for _, r := range l {
		key := TagsToString(r.Tags)
		tags, cached := cache[key]
		if !cached {
			tags = f(r.Tags.Clone())
			cache[key] = tags
		}
		out = append(out, Result{
			Query:        r.Query,
			Tags:         tags,
			Status:       r.Status,
			Duration:     r.Duration,
			MayExonerate: r.MayExonerate,
		})
	}
	return out
}

// ReplaceDuplicates returns a new list with duplicate test results replaced.
// When a duplicate is found, the function f is called with the duplicate
// results. The returned status will be used as the replaced result.
// Merged results will use the average (mean) duration of the duplicates.
func (l List) ReplaceDuplicates(f func(Statuses) Status) List {
	type key struct {
		query query.Query
		tags  string
	}
	// Collect all duplicates
	keyToIndices := map[key][]int{} // key to index
	for i, r := range l {
		k := key{r.Query, TagsToString(r.Tags)}
		keyToIndices[k] = append(keyToIndices[k], i)
	}
	// Filter out exonerated results
	for key, indices := range keyToIndices {
		keptIndices := []int{}
		for _, i := range indices {
			// Copy all indices which are not exonerated into keptIndices.
			if !l[i].MayExonerate {
				keptIndices = append(keptIndices, i)
			}
		}

		// Change indices to only the kept ones. If keptIndices is empty,
		// then all results were marked with may_exonerate, and we keep all
		// of them.
		if len(keptIndices) > 0 {
			keyToIndices[key] = keptIndices
		}
	}
	// Resolve duplicates
	type StatusAndDuration struct {
		Status   Status
		Duration time.Duration
	}
	merged := map[key]StatusAndDuration{}
	for key, indices := range keyToIndices {
		statuses := NewStatuses()
		duration := time.Duration(0)
		for _, i := range indices {
			r := l[i]
			statuses.Add(r.Status)
			duration += r.Duration
		}
		status := func() Status {
			if len(statuses) > 1 {
				return f(statuses)
			}
			return statuses.One()
		}()
		duration = duration / time.Duration(len(indices))
		merged[key] = StatusAndDuration{
			Status:   status,
			Duration: duration,
		}
	}
	// Rebuild list
	out := make(List, 0, len(keyToIndices))
	for _, r := range l {
		k := key{r.Query, TagsToString(r.Tags)}
		if sd, ok := merged[k]; ok {
			out = append(out, Result{
				Query:        r.Query,
				Tags:         r.Tags,
				Status:       sd.Status,
				Duration:     sd.Duration,
				MayExonerate: l[keyToIndices[k][0]].MayExonerate,
			})
			delete(merged, k) // Remove from map to prevent duplicates
		}
	}
	return out
}

// Sort sorts the list
func (l List) Sort() {
	sort.Slice(l, func(i, j int) bool { return l[i].Compare(l[j]) < 0 })
}

// Filter returns the results that match the given predicate
func (l List) Filter(f func(Result) bool) List {
	out := make(List, 0, len(l))
	for _, r := range l {
		if f(r) {
			out = append(out, r)
		}
	}
	return out
}

// FilterByStatus returns the results that the given status
func (l List) FilterByStatus(status Status) List {
	return l.Filter(func(r Result) bool {
		return r.Status == status
	})
}

// FilterByTags returns the results that have all the given tags
func (l List) FilterByTags(tags Tags) List {
	return l.Filter(func(r Result) bool {
		return r.Tags.ContainsAll(tags)
	})
}

// FilterByQuery returns the results that match the given query
func (l List) FilterByQuery(q query.Query) List {
	return l.Filter(func(r Result) bool {
		return q.Contains(r.Query)
	})
}

// Statuses is a set of Status
type Statuses = container.Set[Status]

// NewStatuses returns a new status set with the provided statuses
func NewStatuses(s ...Status) Statuses { return container.NewSet(s...) }

// Statuses returns a set of all the statuses in the list
func (l List) Statuses() Statuses {
	set := NewStatuses()
	for _, r := range l {
		set.Add(r.Status)
	}
	return set
}

// Load loads the result list from the file with the given path
func Load(path string) (ResultsByExecutionMode, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	results, err := Read(file)
	if err != nil {
		return nil, fmt.Errorf("while reading '%v': %w", path, err)
	}
	return results, nil
}

// Identical to Load, but using the provided fsReader instead of directly using
// os.
// TODO(crbug.com/344014313): Merge this with Load once all uses have switched
// to the wrapper version.
func LoadWithWrapper(
	path string,
	fsReader oswrapper.FilesystemReader) (ResultsByExecutionMode, error) {

	file, err := fsReader.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	results, err := Read(file)
	if err != nil {
		return nil, fmt.Errorf("while reading '%v': %w", path, err)
	}
	return results, nil
}

// Save saves the result list to the file with the given path
func Save(path string, results ResultsByExecutionMode) error {
	dir := filepath.Dir(path)
	if err := os.MkdirAll(dir, 0777); err != nil {
		return err
	}
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()
	return Write(file, results)
}

// Identical to Save, but using the provided fsWriter instead of directly using
// os.
// TODO(crbug.com/344014313): Merge this with Save once all uses have switched
// to the wrapper version.
func SaveWithWrapper(
	path string,
	results ResultsByExecutionMode,
	fsWriter oswrapper.FilesystemWriter) error {

	dir := filepath.Dir(path)
	if err := fsWriter.MkdirAll(dir, 0777); err != nil {
		return err
	}
	file, err := fsWriter.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()
	return Write(file, results)
}

// Read reads a result list from the given reader
func Read(r io.Reader) (ResultsByExecutionMode, error) {
	scanner := bufio.NewScanner(r)
	results := ResultsByExecutionMode{}
	l := List{}
	for scanner.Scan() {
		section, r, err := Parse(scanner.Text())
		if err != nil {
			return nil, err
		}
		if section != "" {
			results[section] = l
			l = List{}
		} else {
			l = append(l, r)
		}
	}
	return results, nil
}

// Write writes a result list to the given writer
func Write(w io.Writer, r ResultsByExecutionMode) error {
	for name, l := range r {
		for _, r := range l {
			if _, err := fmt.Fprintln(w, r); err != nil {
				return err
			}
		}
		if _, err := fmt.Fprintln(w, name); err != nil {
			return err
		}
	}
	return nil
}

// Merge merges and sorts multiple results lists.
// Duplicates are removed using the Deduplicate() function.
func Merge(lists ...List) List {
	n := 0
	for _, l := range lists {
		n += len(l)
	}
	merged := make(List, 0, n)
	for _, l := range lists {
		merged = append(merged, l...)
	}
	out := merged.ReplaceDuplicates(Deduplicate)
	out.Sort()
	return out
}

// Deduplicate is the standard algorithm used to de-duplicating mixed results.
// This function is expected to be handed to List.ReplaceDuplicates().
func Deduplicate(s Statuses) Status {
	// If all results have the same status, then use that
	if len(s) == 1 {
		return s.One()
	}

	// Mixed statuses. Replace with something appropriate.
	switch {
	// Crash + * = Crash
	case s.Contains(Crash):
		return Crash
	// Abort + * = Abort
	case s.Contains(Abort):
		return Abort
	// Unknown + * = Unknown
	case s.Contains(Unknown):
		return Unknown
	// RetryOnFailure + ~(Crash | Abort | Unknown) = RetryOnFailure
	case s.Contains(RetryOnFailure):
		return RetryOnFailure
	// Pass + ~(Crash | Abort | Unknown | RetryOnFailure | Slow) = RetryOnFailure
	case s.Contains(Pass):
		return RetryOnFailure
	}
	return Unknown
}
