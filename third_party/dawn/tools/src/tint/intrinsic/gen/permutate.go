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

package gen

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

// Permutator generates permutations of intrinsic overloads
type Permutator struct {
	sem      *sem.Sem
	allTypes []sem.FullyQualifiedName
}

// NewPermutator returns a new initialized Permutator
func NewPermutator(s *sem.Sem) (*Permutator, error) {
	// allTypes are the list of FQNs that are used for unconstrained types
	allTypes := []sem.FullyQualifiedName{}
	for _, ty := range s.Types {
		if len(ty.TemplateParams) > 0 {
			// Ignore aggregate types for now.
			// If generation of e2e tests with aggregate types is required then selectively allow
			// them here - just be careful with permutation explosions!
			continue
		}
		allTypes = append(allTypes, sem.FullyQualifiedName{Target: ty})
	}
	return &Permutator{
		sem:      s,
		allTypes: allTypes,
	}, nil
}

// Permutation describes a single permutation of an overload
type Permutation struct {
	sem.Overload         // The permuted overload signature
	ExplicitTemplateArgs []sem.FullyQualifiedName
	Desc                 string // Description of the overload
	Hash                 string // Hash of the overload
}

// Permute generates a set of permutations for the given intrinsic overload
func (p *Permutator) Permute(overload *sem.Overload) ([]Permutation, error) {
	state := permutationState{
		Permutator:      p,
		templateTypes:   map[sem.TemplateParam]sem.FullyQualifiedName{},
		templateNumbers: map[sem.TemplateParam]any{},
		parameters:      map[int]sem.FullyQualifiedName{},
	}

	out := []Permutation{}

	// Map of hash to permutation description. Used to detect collisions.
	hashes := map[string]string{}

	// permute appends a permutation to out.
	// permute may be chained to generate N-dimensional permutations.
	permute := func() error {
		// Generate an overload with the templated types replaced with the permuted types.
		o := sem.Overload{
			Decl:             overload.Decl,
			Intrinsic:        overload.Intrinsic,
			CanBeUsedInStage: overload.CanBeUsedInStage,
		}
		for i, p := range overload.Parameters {
			ty := state.parameters[i]
			if !validate(ty, &o.CanBeUsedInStage) {
				return nil
			}
			o.Parameters = append(o.Parameters, sem.Parameter{
				Name:      p.Name,
				Type:      ty,
				IsConst:   p.IsConst,
				TestValue: p.TestValue,
			})
		}
		if overload.ReturnType != nil {
			retTys, err := state.permuteFQN(*overload.ReturnType)
			if err != nil {
				return fmt.Errorf("while permuting return type: %w", err)
			}
			if len(retTys) != 1 {
				return fmt.Errorf("result type not pinned")
			}
			o.ReturnType = &retTys[0]
		}

		explicitTemplateArgs := make([]sem.FullyQualifiedName, len(overload.ExplicitTemplates))
		for i, t := range overload.ExplicitTemplates {
			ty := state.templateTypes[t]
			explicitTemplateArgs[i] = ty
			o.ExplicitTemplates = append(o.ExplicitTemplates, &sem.TemplateTypeParam{
				Name: t.GetName(),
				Type: &ty,
			})
		}

		desc := fmt.Sprint(o)
		hash := sha256.Sum256([]byte(desc))
		const hashLength = 6
		shortHash := hex.EncodeToString(hash[:])[:hashLength]
		out = append(out, Permutation{
			Overload:             o,
			ExplicitTemplateArgs: explicitTemplateArgs,
			Desc:                 desc,
			Hash:                 shortHash,
		})

		// Check for hash collisions
		if existing, collision := hashes[shortHash]; collision {
			return fmt.Errorf(`hash '%v' collision between
  %v
and
  %v
Increase hashLength in %v`,
				shortHash, existing, desc, fileutils.ThisLine())
		}
		hashes[shortHash] = desc
		return nil
	}
	for i, param := range overload.Parameters {
		i, param := i, param // Capture iterator values for anonymous function
		next := permute      // Permutation chaining
		permute = func() error {
			permutations, err := state.permuteFQN(param.Type)
			if err != nil {
				return fmt.Errorf("while processing parameter %v: %w", i, err)
			}
			if len(permutations) == 0 {
				return fmt.Errorf("parameter %v has no permutations", i)
			}
			for _, fqn := range permutations {
				state.parameters[i] = fqn
				if err := next(); err != nil {
					return err
				}
			}
			return nil
		}
	}

	for _, t := range overload.AllTemplates() {
		t := t          // Capture iterator values for anonymous function
		next := permute // Permutation chaining
		switch t := t.(type) {
		case *sem.TemplateTypeParam:
			permute = func() error {
				types := p.allTypes
				if t.Type != nil {
					var err error
					types, err = state.permuteFQN(*t.Type)
					if err != nil {
						return fmt.Errorf("while permuting template types: %w", err)
					}
				}
				if len(types) == 0 {
					return fmt.Errorf("template type %v has no permutations", t.Name)
				}
				for _, ty := range types {
					state.templateTypes[t] = ty
					if err := next(); err != nil {
						return err
					}
				}
				return nil
			}
		case *sem.TemplateEnumParam:
			var permutations []sem.FullyQualifiedName
			var err error
			if t.Matcher != nil {
				permutations, err = state.permuteFQN(sem.FullyQualifiedName{Target: t.Matcher})
			} else {
				permutations, err = state.permuteFQN(sem.FullyQualifiedName{Target: t.Enum})
			}
			if err != nil {
				return nil, fmt.Errorf("while permuting template numbers: %w", err)
			}
			if len(permutations) == 0 {
				return nil, fmt.Errorf("template type %v has no permutations", t.Name)
			}
			permute = func() error {
				for _, n := range permutations {
					state.templateNumbers[t] = n
					if err := next(); err != nil {
						return err
					}
				}
				return nil
			}
		case *sem.TemplateNumberParam:
			// Currently all open numbers are used for vector / matrices
			permutations := []int{2, 3, 4}

			// Restrict the permutations for subgroup matrix builtins to avoid combinatorial explosion.
			if strings.HasPrefix(overload.Decl.Name, "subgroupMatrix") {
				if t.Name == "AC" {
					permutations = []int{64}
				} else {
					permutations = []int{8}
				}
			}

			permute = func() error {
				for _, n := range permutations {
					state.templateNumbers[t] = n
					if err := next(); err != nil {
						return err
					}
				}
				return nil
			}
		}
	}

	if err := permute(); err != nil {
		return nil, fmt.Errorf("%v %v %w\nState: %v", overload.Decl.Source, overload.Decl, err, state)
	}

	return out, nil
}

type permutationState struct {
	*Permutator
	templateTypes   map[sem.TemplateParam]sem.FullyQualifiedName
	templateNumbers map[sem.TemplateParam]any
	parameters      map[int]sem.FullyQualifiedName
}

func (s permutationState) String() string {
	sb := &strings.Builder{}
	sb.WriteString("Template types:\n")
	for ct, ty := range s.templateTypes {
		fmt.Fprintf(sb, "  %v: %v\n", ct.GetName(), ty)
	}
	sb.WriteString("Template numbers:\n")
	for cn, v := range s.templateNumbers {
		fmt.Fprintf(sb, "  %v: %v\n", cn.GetName(), v)
	}
	return sb.String()
}

func (s *permutationState) permuteFQN(in sem.FullyQualifiedName) ([]sem.FullyQualifiedName, error) {
	args := append([]any{}, in.TemplateArguments...)
	out := []sem.FullyQualifiedName{}

	// permute appends a permutation to out.
	// permute may be chained to generate N-dimensional permutations.
	var permute func() error

	switch target := in.Target.(type) {
	case *sem.Type:
		permute = func() error {
			// Inner-most permute lambda.
			// Append a the current permutation to out
			out = append(out, sem.FullyQualifiedName{Target: in.Target, TemplateArguments: args})
			// Clone the argument list, for the next permutation to use.
			args = append([]any{}, args...)
			return nil
		}
	case sem.TemplateParam:
		if ty, ok := s.templateTypes[target]; ok {
			permute = func() error {
				out = append(out, ty)
				return nil
			}
		} else {
			return nil, fmt.Errorf("'%v' was not found in templateTypes", target.GetName())
		}
	case *sem.TypeMatcher:
		permute = func() error {
			for _, ty := range target.Types {
				out = append(out, sem.FullyQualifiedName{Target: ty})
			}
			return nil
		}
	case *sem.EnumMatcher:
		permute = func() error {
			for _, o := range target.Options {
				if !o.IsInternal {
					out = append(out, sem.FullyQualifiedName{Target: o})
				}
			}
			return nil
		}
	case *sem.Enum:
		permute = func() error {
			for _, e := range target.Entries {
				if !e.IsInternal {
					out = append(out, sem.FullyQualifiedName{Target: e})
				}
			}
			return nil
		}
	default:
		return nil, fmt.Errorf("unhandled target type: %T", in.Target)
	}

	for i, arg := range in.TemplateArguments {
		i := i          // Capture iterator value for anonymous functions
		next := permute // Permutation chaining
		switch arg := arg.(type) {
		case sem.FullyQualifiedName:
			switch target := arg.Target.(type) {
			case sem.TemplateParam:
				if ty, ok := s.templateTypes[target]; ok {
					args[i] = ty
				} else if num, ok := s.templateNumbers[target]; ok {
					args[i] = num
				} else {
					return nil, fmt.Errorf("'%v' was not found in templateTypes or templateNumbers", target.GetName())
				}
			default:
				perms, err := s.permuteFQN(arg)
				if err != nil {
					return nil, fmt.Errorf("while processing template argument %v: %v", i, err)
				}
				if len(perms) == 0 {
					return nil, fmt.Errorf("template argument %v has no permutations", i)
				}
				permute = func() error {
					for _, f := range perms {
						args[i] = f
						if err := next(); err != nil {
							return err
						}
					}
					return nil
				}
			}
		default:
			return nil, fmt.Errorf("permuteFQN() unhandled template argument type: %T", arg)
		}
	}

	if err := permute(); err != nil {
		return nil, fmt.Errorf("while processing fully qualified name '%v': %w", in.Target.GetName(), err)
	}

	return out, nil
}

func validate(fqn sem.FullyQualifiedName, uses *sem.StageUses) bool {
	if strings.HasPrefix(fqn.Target.GetName(), "_") {
		return false // Builtin, untypeable return type
	}

	isStorable := func(elTy sem.FullyQualifiedName) bool {
		elTyName := elTy.Target.GetName()
		switch {
		case elTyName == "bool",
			strings.Contains(elTyName, "i8"),
			strings.Contains(elTyName, "u8"),
			strings.Contains(elTyName, "sampler"),
			strings.Contains(elTyName, "texture"),
			IsAbstract(DeepestElementType(elTy)):
			return false
		}
		return true
	}

	switch fqn.Target.GetName() {
	case "array":
	case "runtime_array":
		if !isStorable(fqn.TemplateArguments[0].(sem.FullyQualifiedName)) {
			return false
		}
	case "ptr":
		// https://gpuweb.github.io/gpuweb/wgsl/#address-space
		access := fqn.TemplateArguments[2].(sem.FullyQualifiedName).Target.(*sem.EnumEntry).Name
		addressSpace := fqn.TemplateArguments[0].(sem.FullyQualifiedName).Target.(*sem.EnumEntry).Name
		switch addressSpace {
		case "function", "private":
			if access != "read_write" {
				return false
			}
		case "workgroup":
			uses.Vertex = false
			uses.Fragment = false
			if access != "read_write" {
				return false
			}
			if !isStorable(fqn.TemplateArguments[1].(sem.FullyQualifiedName)) {
				return false
			}
		case "uniform":
			if access != "read" {
				return false
			}
		case "storage":
			if access != "read_write" && access != "read" {
				return false
			}
		case "handle":
			if access != "read" {
				return false
			}
		default:
			return false
		}
	}

	for _, arg := range fqn.TemplateArguments {
		if argFQN, ok := arg.(sem.FullyQualifiedName); ok {
			if !validate(argFQN, uses) {
				return false
			}
		}
	}

	return true
}
