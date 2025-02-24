// Copyright 2021 The Dawn & Tint Authors
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

// Package gen holds types and helpers for generating templated code from the
// intrinsic.def file.
//
// Used by tools/src/cmd/gen/main.go
package gen

import (
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/lut"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

// IntrinsicTable holds data specific to the intrinsic_table.inl.tmpl template
type IntrinsicTable struct {
	// The semantic info
	Sem *sem.Sem

	// TMatchers are all the sem.TemplateType, sem.Type and sem.TypeMatchers.
	// These are all implemented by classes deriving from tint::TypeMatcher
	TMatchers     []sem.Named
	TMatcherIndex map[sem.Named]int // [object -> index] in TMatcher

	// NMatchers are all the sem.TemplateNumber and sem.EnumMatchers.
	// These are all implemented by classes deriving from tint::NumberMatcher
	NMatchers     []sem.Named
	NMatcherIndex map[sem.Named]int // [object -> index] in NMatchers

	MatcherIndices            []int       // kMatcherIndices table content
	Templates                 []Template  // kTemplates table content
	Parameters                []Parameter // kParameters table content
	Overloads                 []Overload  // kOverloads table content
	Builtins                  []Intrinsic // kBuiltins table content
	UnaryOperators            []Intrinsic // kUnaryOperators table content
	BinaryOperators           []Intrinsic // kBinaryOperators table content
	ConstructorsAndConverters []Intrinsic // kInitializersAndConverters table content
	ConstEvalFunctions        []string    // kConstEvalFunctions table content
}

// Template is used to create the C++ TemplateInfo structure
type Template struct {
	// Name of the template type (e.g. 'T')
	Name string

	// Kind of the template
	Kind sem.TemplateKind

	// Index into IntrinsicTable.MatcherIndices, beginning the list of matchers required to match
	// the parameter type.
	// The matcher indices index into IntrinsicTable::TMatchers and IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	MatcherIndicesOffset int
}

// Parameter is used to create the C++ ParameterInfo structure
type Parameter struct {
	// The parameter usage (parameter name)
	Usage string

	// Index into IntrinsicTable.MatcherIndices, beginning the list of matchers required to match
	// the parameter type.
	// The matcher indices index into IntrinsicTable::TMatchers and IntrinsicTable::NMatchers.
	// These indices are consumed by the matchers themselves.
	MatcherIndicesOffset int
}

// Overload is used to create the C++ OverloadInfo structure
type Overload struct {
	// Total number of parameters for the overload
	NumParameters int
	// Total number of explicit templates for the overload
	NumExplicitTemplates int
	// Total number of explicit and implicit templates for the overload
	NumTemplates int
	// Index to the first template in IntrinsicTable.Templates
	// This is a list of template starting with the explicit templates, then the implicit templates.
	TemplatesOffset int
	// Index to the first parameter in IntrinsicTable.Parameters
	ParametersOffset int
	// Index into IntrinsicTable.matcherIndices, beginning the list of matchers
	// required to match the return type.
	// The matcher indices index into IntrinsicTable::TMatchers.
	// These indices are consumed by the matchers themselves.
	ReturnMatcherIndicesOffset int
	// Index into IntrinsicTable.ConstEvalFunctions.
	ConstEvalFunctionOffset int
	// StageUses describes the stages an overload can be used in
	CanBeUsedInStage sem.StageUses
	// True if the overload is marked as @must_use
	MustUse bool
	// True if the overload is marked as @member_function
	MemberFunction bool
	// True if the overload is marked as deprecated
	IsDeprecated bool
	// The kind of overload
	Kind string
}

// Intrinsic is used to create the C++ IntrinsicInfo structure
type Intrinsic struct {
	Name                 string
	OverloadDescriptions []string
	NumOverloads         int
	OverloadsOffset      *int
}

// Helper for building the IntrinsicTable
type IntrinsicTableBuilder struct {
	// The output of the builder
	IntrinsicTable

	// Lookup tables.
	// These are packed (compressed) once all the entries have been added.
	lut struct {
		matcherIndices           lut.LUT[int]
		templates                lut.LUT[Template]
		constEvalFunctionIndices lut.LUT[string]
		parameters               lut.LUT[Parameter]
		overloads                lut.LUT[Overload]
	}
}

type parameterBuilder struct {
	usage                string
	matcherIndicesOffset *int
}

type templateBuilder struct {
	// The index of the template in kTypeMatchers / kNumberMatchers
	matcherIndex int
	// The matcher indices for this template type / number
	constraintIndicesOffset *int
}

// Helper for building a single overload
type overloadBuilder struct {
	*IntrinsicTableBuilder
	// The overload being built
	overload *sem.Overload
	// Map of TemplateParam to templatesBuilder
	templateBuilders map[sem.TemplateParam]*templateBuilder
	// Templates used by the overload
	// This is a list of explicit template types followed by the implicit template types.
	templates []Template
	// Index to the first template in IntrinsicTable.Templates
	// This is a list of template starting with the explicit templates, then the implicit templates.
	templateOffset *int
	// Builders for all parameters
	parameterBuilders []parameterBuilder
	// Index to the first parameter in IntrinsicTable.Parameters
	parametersOffset *int
	// Index into IntrinsicTable.ConstEvalFunctions
	constEvalFunctionOffset *int
	// Index into IntrinsicTable.matcherIndices, beginning the list of
	// matchers required to match the return type.
	// The matcher indices index into IntrinsicTable::TMatchers.
	// These indices are consumed by the matchers themselves.
	returnMatcherIndicesOffset *int
}

// layoutMatchers assigns each of the TMatchers and NMatchers a unique index.
func (b *IntrinsicTableBuilder) layoutMatchers(s *sem.Sem) {
	// First MaxTemplates of TMatchers and NMatchers are template types
	b.TMatchers = make([]sem.Named, s.MaxTemplates)
	b.NMatchers = make([]sem.Named, s.MaxTemplates)

	for _, m := range s.Types {
		b.TMatcherIndex[m] = len(b.TMatchers)
		b.TMatchers = append(b.TMatchers, m)
	}
	for _, m := range s.TypeMatchers {
		b.TMatcherIndex[m] = len(b.TMatchers)
		b.TMatchers = append(b.TMatchers, m)
	}
	for _, m := range s.EnumMatchers {
		b.NMatcherIndex[m] = len(b.NMatchers)
		b.NMatchers = append(b.NMatchers, m)
	}
}

func (b *IntrinsicTableBuilder) newOverloadBuilder(o *sem.Overload) *overloadBuilder {
	return &overloadBuilder{
		IntrinsicTableBuilder: b,
		overload:              o,
		templateBuilders:      map[sem.TemplateParam]*templateBuilder{},
	}
}

// processStage0 begins processing of the overload.
// Preconditions:
// - Must be called before any LUTs are compacted.
// Populates:
// - b.templateBuilders
// - b.parameterBuilders
// - b.returnMatcherIndicesOffset
// - b.constEvalFunctionOffset
func (b *overloadBuilder) processStage0() error {
	// Calculate the template matcher indices
	for _, t := range b.overload.AllTemplates() {
		b.templateBuilders[t] = &templateBuilder{matcherIndex: len(b.templateBuilders)}
	}

	for _, t := range b.overload.AllTemplates() {
		switch t := t.(type) {
		case *sem.TemplateTypeParam:
			if t.Type != nil {
				indices, err := b.collectMatcherIndices(*t.Type)
				if err != nil {
					return err
				}
				b.templateBuilders[t].constraintIndicesOffset = b.lut.matcherIndices.Add(indices)
			}
		case *sem.TemplateEnumParam:
			if t.Matcher != nil {
				index, err := b.matcherIndex(t.Matcher)
				if err != nil {
					return err
				}
				b.templateBuilders[t].constraintIndicesOffset = b.lut.matcherIndices.Add([]int{index})
			}
		}
	}

	if b.overload.ReturnType != nil {
		indices, err := b.collectMatcherIndices(*b.overload.ReturnType)
		if err != nil {
			return err
		}
		b.returnMatcherIndicesOffset = b.lut.matcherIndices.Add(indices)
	}

	b.parameterBuilders = make([]parameterBuilder, len(b.overload.Parameters))
	for i, p := range b.overload.Parameters {
		matcherIndices, err := b.collectMatcherIndices(p.Type)
		if err != nil {
			return err
		}

		b.parameterBuilders[i] = parameterBuilder{
			usage:                p.Name,
			matcherIndicesOffset: b.lut.matcherIndices.Add(matcherIndices),
		}
	}

	if b.overload.ConstEvalFunction != "" {
		b.constEvalFunctionOffset = b.lut.constEvalFunctionIndices.Add([]string{b.overload.ConstEvalFunction})
	}

	return nil
}

// processStage1 builds the Parameters  used by the overload
// Must only be called after the following LUTs have been compacted:
// - b.lut.matcherIndices
// Populates:
// - b.templates
// - b.templateOffset
// - b.parametersOffset
func (b *overloadBuilder) processStage1() error {
	b.templates = []Template{}
	for _, t := range b.overload.AllTemplates() {
		b.templates = append(b.templates, Template{
			Name:                 t.GetName(),
			Kind:                 t.TemplateKind(),
			MatcherIndicesOffset: loadOrMinusOne(b.templateBuilders[t].constraintIndicesOffset),
		})
	}
	b.templateOffset = b.lut.templates.Add(b.templates)

	parameters := make([]Parameter, len(b.parameterBuilders))
	for i, pb := range b.parameterBuilders {
		parameters[i] = Parameter{
			Usage:                pb.usage,
			MatcherIndicesOffset: loadOrMinusOne(pb.matcherIndicesOffset),
		}
	}
	b.parametersOffset = b.lut.parameters.Add(parameters)
	return nil
}

func (b *overloadBuilder) build() (Overload, error) {
	return Overload{
		NumParameters:              len(b.parameterBuilders),
		NumExplicitTemplates:       len(b.overload.ExplicitTemplates),
		NumTemplates:               len(b.overload.ExplicitTemplates) + len(b.overload.ImplicitTemplates),
		TemplatesOffset:            loadOrMinusOne(b.templateOffset),
		ParametersOffset:           loadOrMinusOne(b.parametersOffset),
		ConstEvalFunctionOffset:    loadOrMinusOne(b.constEvalFunctionOffset),
		ReturnMatcherIndicesOffset: loadOrMinusOne(b.returnMatcherIndicesOffset),
		CanBeUsedInStage:           b.overload.CanBeUsedInStage,
		MustUse:                    b.overload.MustUse,
		MemberFunction:             b.overload.MemberFunction,
		IsDeprecated:               b.overload.IsDeprecated,
		Kind:                       string(b.overload.Decl.Kind),
	}, nil
}

// matcherIndex returns the matcher indices into IntrinsicTable.TMatcher and
// IntrinsicTable.NMatcher, respectively for the given named entity.
func (b *overloadBuilder) matcherIndex(n sem.Named) (int, error) {
	switch n := n.(type) {
	case *sem.Type, *sem.TypeMatcher:
		if i, ok := b.TMatcherIndex[n]; ok {
			return i, nil
		}
		return -1, fmt.Errorf("TMatcherIndex missing entry for %v %T", n.GetName(), n)
	case *sem.EnumMatcher:
		if i, ok := b.NMatcherIndex[n]; ok {
			return i, nil
		}
		return -1, fmt.Errorf("NMatcherIndex missing entry for %v %T", n.GetName(), n)
	case sem.TemplateParam:
		if b, ok := b.templateBuilders[n]; ok {
			return b.matcherIndex, nil
		}
		return -1, fmt.Errorf("templatesBuilders missing entry for %v %T", n.GetName(), n)
	default:
		return -1, fmt.Errorf("overload.matcherIndices() does not handle %v %T", n, n)
	}
}

// collectMatcherIndices returns the full list of matcher indices required to
// match the fully-qualified-name. For names that have do not have templated
// arguments, collectMatcherIndices() will return a single TMatcher index.
// For names that do have templated arguments, collectMatcherIndices() returns
// a list of type matcher indices, starting with the target of the fully
// qualified name, then followed by each of the template arguments from left to
// right. Note that template arguments may themselves have template arguments,
// and so collectMatcherIndices() may call itself.
// The order of returned matcher indices is always the order of the fully
// qualified name as read from left to right.
// For example, calling collectMatcherIndices() for the fully qualified name:
//
//	A<B<C, D>, E<F, G<H>, I>
//
// Would return the matcher indices:
//
//	A, B, C, D, E, F, G, H, I
func (b *overloadBuilder) collectMatcherIndices(fqn sem.FullyQualifiedName) ([]int, error) {
	base, err := b.matcherIndex(fqn.Target)
	if err != nil {
		return nil, err
	}
	indices := []int{base}
	for _, arg := range fqn.TemplateArguments {
		subIndices, err := b.collectMatcherIndices(arg.(sem.FullyQualifiedName))
		if err != nil {
			return nil, err
		}
		indices = append(indices, subIndices...)
	}
	return indices, nil
}

// BuildIntrinsicTable builds the IntrinsicTable from the semantic info
func BuildIntrinsicTable(s *sem.Sem) (*IntrinsicTable, error) {
	b := IntrinsicTableBuilder{
		IntrinsicTable: IntrinsicTable{
			Sem:           s,
			TMatcherIndex: map[sem.Named]int{},
			NMatcherIndex: map[sem.Named]int{},
		},
	}
	b.layoutMatchers(s)

	intrinsicGroups := []struct {
		in  []*sem.Intrinsic
		out *[]Intrinsic
	}{
		{s.Builtins, &b.Builtins},
		{s.UnaryOperators, &b.UnaryOperators},
		{s.BinaryOperators, &b.BinaryOperators},
		{s.ConstructorsAndConverters, &b.ConstructorsAndConverters},
	}

	// Create an overload builder for every overload
	overloadToBuilder := map[*sem.Overload]*overloadBuilder{}
	overloadBuilders := []*overloadBuilder{}
	for _, intrinsics := range intrinsicGroups {
		for _, f := range intrinsics.in {
			for _, o := range f.Overloads {
				builder := b.newOverloadBuilder(o)
				overloadToBuilder[o] = builder
				overloadBuilders = append(overloadBuilders, builder)
			}
		}
	}

	// Perform the 'stage-0' processing of the overloads
	b.lut.matcherIndices = lut.New[int]()
	b.lut.constEvalFunctionIndices = lut.New[string]()
	for _, b := range overloadBuilders {
		if err := b.processStage0(); err != nil {
			return nil, fmt.Errorf("while processing stage 0 of '%v'\n%w", b.overload, err)
		}
	}

	// Clear the compacted LUTs to prevent use-after-compaction
	b.MatcherIndices = b.lut.matcherIndices.Compact()
	b.ConstEvalFunctions = b.lut.constEvalFunctionIndices.Compact()
	b.lut.matcherIndices = nil
	b.lut.constEvalFunctionIndices = nil
	b.lut.templates = lut.New[Template]()

	// Perform the 'stage-1' processing of the overloads
	b.lut.parameters = lut.New[Parameter]()
	for _, b := range overloadBuilders {
		if err := b.processStage1(); err != nil {
			return nil, fmt.Errorf("while processing stage 1 of '%v'\n%w", b.overload, err)
		}
	}
	b.Parameters = b.lut.parameters.Compact()
	b.Templates = b.lut.templates.Compact()
	b.lut.parameters = nil
	b.lut.templates = nil

	// Build the Intrinsics
	b.lut.overloads = lut.New[Overload]()
	for _, intrinsics := range intrinsicGroups {
		out := make([]Intrinsic, len(intrinsics.in))
		for i, f := range intrinsics.in {
			overloads := make([]Overload, len(f.Overloads))
			overloadDescriptions := make([]string, len(f.Overloads))
			for i, o := range f.Overloads {
				overloadDescriptions[i] = fmt.Sprint(o.Decl)
				overload, err := overloadToBuilder[o].build()
				if err != nil {
					return nil, err
				}
				overloads[i] = overload
			}
			out[i] = Intrinsic{
				Name:                 f.Name,
				OverloadDescriptions: overloadDescriptions,
				NumOverloads:         len(overloads),
				OverloadsOffset:      b.lut.overloads.Add(overloads),
			}
		}
		*intrinsics.out = out
	}

	b.Overloads = b.lut.overloads.Compact()

	return &b.IntrinsicTable, nil
}

// SplitDisplayName splits displayName into parts, where text wrapped in {}
// braces are not quoted and the rest is quoted. This is used to help process
// the string value of the [[display()]] decoration. For example:
//
//	SplitDisplayName("vec{N}<{T}>")
//
// would return the strings:
//
//	[`"vec"`, `N`, `"<"`, `T`, `">"`]
func SplitDisplayName(displayName string) []string {
	parts := []string{}
	pending := strings.Builder{}
	for _, r := range displayName {
		switch r {
		case '{':
			if pending.Len() > 0 {
				parts = append(parts, fmt.Sprintf(`"%v"`, pending.String()))
				pending.Reset()
			}
		case '}':
			if pending.Len() > 0 {
				parts = append(parts, pending.String())
				pending.Reset()
			}
		default:
			pending.WriteRune(r)
		}
	}
	if pending.Len() > 0 {
		parts = append(parts, fmt.Sprintf(`"%v"`, pending.String()))
	}
	return parts
}

// ElementType returns the nested type for type represented by the fully qualified name.
// If the type is not a composite type, then the fully qualified name is returned
func ElementType(fqn sem.FullyQualifiedName) sem.FullyQualifiedName {
	switch fqn.Target.GetName() {
	case "vec2", "vec3", "vec4":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	case "vec":
		return fqn.TemplateArguments[1].(sem.FullyQualifiedName)
	case "mat":
		return fqn.TemplateArguments[2].(sem.FullyQualifiedName)
	case "array":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	case "runtime_array":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	case "subgroup_matrix":
		return fqn.TemplateArguments[1].(sem.FullyQualifiedName)
	}
	return fqn
}

// DeepestElementType returns the inner most nested type for type represented by the
// fully qualified name.
func DeepestElementType(fqn sem.FullyQualifiedName) sem.FullyQualifiedName {
	switch fqn.Target.GetName() {
	case "vec2", "vec3", "vec4":
		return fqn.TemplateArguments[0].(sem.FullyQualifiedName)
	case "vec":
		return fqn.TemplateArguments[1].(sem.FullyQualifiedName)
	case "mat2x2", "mat2x3", "mat2x4",
		"mat3x2", "mat3x3", "mat3x4",
		"mat4x2", "mat4x3", "mat4x4":
		return DeepestElementType(fqn.TemplateArguments[0].(sem.FullyQualifiedName))
	case "mat":
		return DeepestElementType(fqn.TemplateArguments[2].(sem.FullyQualifiedName))
	case "array":
		return DeepestElementType(fqn.TemplateArguments[0].(sem.FullyQualifiedName))
	case "runtime_array":
		return DeepestElementType(fqn.TemplateArguments[0].(sem.FullyQualifiedName))
	case "ptr":
		return DeepestElementType(fqn.TemplateArguments[1].(sem.FullyQualifiedName))
	case "subgroup_matrix":
		return DeepestElementType(fqn.TemplateArguments[1].(sem.FullyQualifiedName))
	}
	return fqn
}

// IsAbstract returns true if the FullyQualifiedName refers to an abstract numeric type float.
// Use DeepestElementType if you want to include vector, matrices and arrays of abstract types.
func IsAbstract(fqn sem.FullyQualifiedName) bool {
	switch fqn.Target.GetName() {
	case "ia", "fa":
		return true
	}
	return false
}

// IsDeclarable returns false if the FullyQualifiedName refers to an abstract
// numeric type, or if it starts with a leading underscore.
func IsDeclarable(fqn sem.FullyQualifiedName) bool {
	return !IsAbstract(DeepestElementType(fqn)) && !strings.HasPrefix(fqn.Target.GetName(), "_")
}

// IsHostShareable returns true if the FullyQualifiedName refers to a type that is host-sharable.
// See https://www.w3.org/TR/WGSL/#host-shareable-types
func IsHostShareable(fqn sem.FullyQualifiedName) bool {
	return IsDeclarable(fqn) && DeepestElementType(fqn).Target.GetName() != "bool"
}

// OverloadUsesType returns true if the overload uses the given type anywhere in the signature.
func OverloadUsesType(overload sem.Overload, ty string) bool {
	pending := []sem.FullyQualifiedName{}
	for _, param := range overload.Parameters {
		pending = append(pending, param.Type)
	}
	if ret := overload.ReturnType; ret != nil {
		pending = append(pending, *ret)
	}

	for len(pending) > 0 {
		fqn := pending[len(pending)-1]
		pending = pending[:len(pending)-1]

		if fqn.Target.GetName() == ty {
			return true
		}
		for _, arg := range fqn.TemplateArguments {
			switch arg := arg.(type) {
			case sem.FullyQualifiedName:
				pending = append(pending, arg)
			case sem.Named:
				if fqn.Target.GetName() == ty {
					return true
				}
			}
		}
	}

	return false
}

// OverloadUsesReadWriteStorageTexture returns true if the overload uses a read-only or read-write
// storage texture.
func OverloadUsesReadWriteStorageTexture(overload sem.Overload) bool {
	for _, param := range overload.Parameters {
		if strings.HasPrefix(param.Type.Target.GetName(), "texture_storage") {
			access := param.Type.TemplateArguments[1].(sem.FullyQualifiedName).Target.GetName()
			if access == "read" || access == "read_write" {
				return true
			}
		}
	}
	return false
}

// OverloadUsesGLESTexture returns true if the overload uses a texture value for GLSL ES 3.10
func OverloadNeedsDesktopGLSL(overload sem.Overload) bool {
	if overload.Intrinsic.Name == "textureSampleLevel" {
		for _, param := range overload.Parameters {
			if strings.HasPrefix(param.Type.Target.GetName(), "texture_depth_2d_array") ||
				strings.HasPrefix(param.Type.Target.GetName(), "texture_depth_cube") ||
				strings.HasPrefix(param.Type.Target.GetName(), "texture_depth_cube_array") {
				return true
			}
		}
	}
	if overload.Intrinsic.Name == "textureSampleCompareLevel" {
		has_tex := false
		for _, param := range overload.Parameters {
			if strings.HasPrefix(param.Type.Target.GetName(), "texture_depth_2d_array") {
				has_tex = true
				break
			}
		}
		for _, param := range overload.Parameters {
			if param.Name == "offset" {
				if has_tex {
					return true
				}
				break
			}
		}
	}

	for _, param := range overload.Parameters {
		if strings.HasPrefix(param.Type.Target.GetName(), "texture_storage") {
			fmt := param.Type.TemplateArguments[0].(sem.FullyQualifiedName).Target.GetName()
			if fmt == "rg32uint" || fmt == "rg32sint" || fmt == "rg32float" || fmt == "r8unorm" {
				return true
			}
		}
		if strings.HasPrefix(param.Type.Target.GetName(), "texture_cube_array") ||
			strings.HasPrefix(param.Type.Target.GetName(), "texture_depth_cube_array") {
			return true
		}
	}
	return false
}

func loadOrMinusOne(p *int) int {
	if p != nil {
		return *p
	}
	return -1
}
