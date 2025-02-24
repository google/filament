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

package resolver

import (
	"fmt"
	"sort"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/ast"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/tok"
)

type resolver struct {
	a *ast.AST
	s *sem.Sem

	globals                   scope
	builtins                  map[string]*sem.Intrinsic
	unaryOperators            map[string]*sem.Intrinsic
	binaryOperators           map[string]*sem.Intrinsic
	constructorsAndConverters map[string]*sem.Intrinsic
	enumEntryMatchers         map[*sem.EnumEntry]*sem.EnumMatcher
}

// Resolve processes the AST
func Resolve(a *ast.AST) (*sem.Sem, error) {
	r := resolver{
		a:                         a,
		s:                         sem.New(),
		globals:                   newScope(nil),
		builtins:                  map[string]*sem.Intrinsic{},
		unaryOperators:            map[string]*sem.Intrinsic{},
		binaryOperators:           map[string]*sem.Intrinsic{},
		constructorsAndConverters: map[string]*sem.Intrinsic{},
		enumEntryMatchers:         map[*sem.EnumEntry]*sem.EnumMatcher{},
	}
	// Declare and resolve all the enumerators
	for _, e := range a.Enums {
		if err := r.enum(e); err != nil {
			return nil, err
		}
	}
	// Declare and resolve all the ty types
	for _, p := range a.Types {
		if err := r.ty(p); err != nil {
			return nil, err
		}
	}
	// Declare and resolve the type matchers
	for _, m := range a.Matchers {
		if err := r.matcher(m); err != nil {
			return nil, err
		}
	}
	// Declare and resolve the builtins
	for _, f := range a.Builtins {
		if err := r.intrinsic(f, r.builtins, &r.s.Builtins); err != nil {
			return nil, err
		}
	}
	// Declare and resolve the unary and binary operators
	for _, o := range a.Operators {
		switch len(o.Parameters) {
		case 1:
			if err := r.intrinsic(o, r.unaryOperators, &r.s.UnaryOperators); err != nil {
				return nil, err
			}
		case 2:
			if err := r.intrinsic(o, r.binaryOperators, &r.s.BinaryOperators); err != nil {
				return nil, err
			}
		default:
			return nil, fmt.Errorf("%v operators must have either 1 or 2 parameters", o.Source)
		}
	}

	// Declare and resolve value constructors and converters
	for _, c := range a.Constructors {
		if err := r.intrinsic(c, r.constructorsAndConverters, &r.s.ConstructorsAndConverters); err != nil {
			return nil, err
		}
	}
	for _, c := range a.Converters {
		if len(c.Parameters) != 1 {
			return nil, fmt.Errorf("%v conversions must have a single parameter", c.Source)
		}
		if err := r.intrinsic(c, r.constructorsAndConverters, &r.s.ConstructorsAndConverters); err != nil {
			return nil, err
		}
	}

	var usages = ast.EnumDecl{}
	for _, e := range a.Enums {
		if e.Name == "usages" {
			usages = e
		}
	}
	// Calculate the unique parameter names
	r.s.UniqueParameterNames = r.calculateUniqueParameterNames(usages)

	return r.s, nil
}

// enum() resolves an enum declaration.
// The resulting sem.Enum is appended to Sem.Enums, and the enum and all its
// entries are registered with the global scope.
func (r *resolver) enum(e ast.EnumDecl) error {
	s := &sem.Enum{
		Decl: e,
		Name: e.Name,
	}

	// Register the enum
	r.s.Enums = append(r.s.Enums, s)
	if err := r.globals.declare(s, e.Source); err != nil {
		return err
	}

	// Register each of the enum entries
	names := container.NewSet[string]()
	for _, ast := range e.Entries {
		entry := &sem.EnumEntry{
			Name: ast.Name,
			Enum: s,
		}
		if internal := ast.Attributes.Take("internal"); internal != nil {
			entry.IsInternal = true
			if len(internal.Values) != 0 {
				return fmt.Errorf("%v unexpected value for internal attribute", ast.Source)
			}
		}
		if len(ast.Attributes) != 0 {
			return fmt.Errorf("%v unknown attribute", ast.Attributes[0].Source)
		}
		s.Entries = append(s.Entries, entry)
		if names.Contains(ast.Name) {
			return fmt.Errorf("%v duplicate enum entry '%v'", ast.Source, ast.Name)
		}
		names.Add(ast.Name)
	}

	// Sort the enum entries into lexicographic order
	sort.Slice(s.Entries, func(i, j int) bool { return s.Entries[i].Name < s.Entries[j].Name })

	return nil
}

// ty() resolves a type declaration.
// The resulting sem.Type is appended to Sem.Types, and the type is registered
// with the global scope.
func (r *resolver) ty(a ast.TypeDecl) error {
	t := &sem.Type{
		Decl: a,
		Name: a.Name,
	}

	// Register the type
	r.s.Types = append(r.s.Types, t)
	if err := r.globals.declare(t, a.Source); err != nil {
		return err
	}

	// Create a new scope for resolving template parameters
	s := newScope(&r.globals)

	// Resolve the type template parameters
	templateParams, err := r.templateParams(&s, a.TemplateParams)
	if err != nil {
		return err
	}
	t.TemplateParams = templateParams

	// Scan for attributes
	if d := a.Attributes.Take("display"); d != nil {
		if len(d.Values) != 1 {
			return fmt.Errorf("%v expected a single value for 'display' attribute", d.Source)
		}
		t.DisplayName = fmt.Sprint(d.Values[0])
	}
	if d := a.Attributes.Take("precedence"); d != nil {
		if len(d.Values) != 1 {
			return fmt.Errorf("%v expected a single integer value for 'precedence' attribute", d.Source)
		}
		n, ok := d.Values[0].(int)
		if !ok {
			return fmt.Errorf("%v @precedence value must be an integer", d.Source)
		}
		t.Precedence = n
	}

	if len(a.Attributes) != 0 {
		return fmt.Errorf("%v unknown attribute", a.Attributes[0].Source)
	}

	return nil
}

// matcher() resolves a match declaration to either a sem.TypeMatcher or
// sem.EnumMatcher.
// The resulting matcher is appended to either Sem.TypeMatchers or
// Sem.EnumMatchers, and is registered with the global scope.
func (r *resolver) matcher(a ast.MatcherDecl) error {
	isTypeMatcher := len(a.Options.Types) != 0
	isEnumMatcher := len(a.Options.Enums) != 0
	if isTypeMatcher && isEnumMatcher {
		return fmt.Errorf("%v matchers cannot mix enums and types", a.Source)
	}

	if isTypeMatcher {
		options := map[sem.Named]tok.Source{}
		m := &sem.TypeMatcher{
			Decl: a,
			Name: a.Name,
		}

		// Register the matcher
		r.s.TypeMatchers = append(r.s.TypeMatchers, m)
		if err := r.globals.declare(m, a.Source); err != nil {
			return err
		}

		// Resolve each of the types in the options list
		for _, ast := range m.Decl.Options.Types {
			ty, err := r.lookupType(&r.globals, ast)
			if err != nil {
				return err
			}
			m.Types = append(m.Types, ty)
			if s, dup := options[ty]; dup {
				return fmt.Errorf("%v duplicate option '%v' in matcher\nFirst declared here: %v", ast.Source, ast.Name, s)
			}
			options[ty] = ast.Source
		}

		return nil
	}
	if isEnumMatcher {
		owners := container.NewSet[string]()
		for _, enum := range a.Options.Enums {
			owners.Add(enum.Owner)
		}
		if len(owners) > 1 {
			return fmt.Errorf("%v cannot mix enums (%v) in type matcher", a.Source, owners)
		}
		enumName := owners.One()
		lookup := r.globals.lookup(enumName)
		if lookup == nil {
			return fmt.Errorf("%v cannot resolve enum '%v'", a.Source, enumName)
		}
		enum, _ := lookup.object.(*sem.Enum)
		if enum == nil {
			return fmt.Errorf("%v cannot resolve enum '%v'", a.Source, enumName)
		}

		m := &sem.EnumMatcher{
			Decl: a,
			Name: a.Name,
			Enum: enum,
		}

		// Register the matcher
		r.s.EnumMatchers = append(r.s.EnumMatchers, m)
		if err := r.globals.declare(m, a.Source); err != nil {
			return err
		}

		// Resolve each of the enums in the options list
		for _, ast := range m.Decl.Options.Enums {
			entry := enum.FindEntry(ast.Member)
			if entry == nil {
				return fmt.Errorf("%v enum '%v' does not contain '%v'", ast.Source, enum.Name, ast.Member)
			}
			m.Options = append(m.Options, entry)
		}

		return nil
	}

	return fmt.Errorf("%v matcher cannot be empty", a.Source)
}

// intrinsic() resolves a intrinsic overload declaration.
// The the first overload for the intrinsic creates and appends the sem.Intrinsic
// to Sem.Intrinsics. Subsequent overloads append their resolved overload to the
// sem.intrinsic.Overloads list.
func (r *resolver) intrinsic(
	a ast.IntrinsicDecl,
	intrinsicsByName map[string]*sem.Intrinsic,
	semIntrinsics *[]*sem.Intrinsic) error {
	// If this is the first overload of the intrinsic, create and register the
	// semantic intrinsic.
	intrinsic := intrinsicsByName[a.Name]
	if intrinsic == nil {
		intrinsic = &sem.Intrinsic{Name: a.Name}
		intrinsicsByName[a.Name] = intrinsic
		*semIntrinsics = append(*semIntrinsics, intrinsic)
	}

	// Create a new scope for resolving template parameters
	s := newScope(&r.globals)

	// Construct the semantic overload
	overload := &sem.Overload{
		Decl:       a,
		Intrinsic:  intrinsic,
		Parameters: make([]sem.Parameter, len(a.Parameters)),
	}

	// Resolve the declared template parameters
	// Explicit template types can use implicit templates, so resolve implicit first
	implicitTemplates, err := r.templateParams(&s, a.ImplicitTemplateParams)
	if err != nil {
		return err
	}
	overload.ImplicitTemplates = implicitTemplates

	explicitTemplates, err := r.templateParams(&s, a.ExplicitTemplateParams)
	if err != nil {
		return err
	}
	overload.ExplicitTemplates = explicitTemplates

	// Process overload attributes
	if stageDeco := a.Attributes.Take("stage"); stageDeco != nil {
		for stageDeco != nil {
			for _, stage := range stageDeco.Values {
				switch stage {
				case "vertex":
					overload.CanBeUsedInStage.Vertex = true
				case "fragment":
					overload.CanBeUsedInStage.Fragment = true
				case "compute":
					overload.CanBeUsedInStage.Compute = true
				default:
					return fmt.Errorf("%v unknown stage '%v'", stageDeco.Source, stage)
				}
			}
			stageDeco = a.Attributes.Take("stage")
		}
	} else {
		overload.CanBeUsedInStage = sem.StageUses{
			Vertex:   true,
			Fragment: true,
			Compute:  true,
		}
	}
	if mustUse := a.Attributes.Take("must_use"); mustUse != nil {
		if len(mustUse.Values) > 0 {
			return fmt.Errorf("%v @must_use does not accept any arguments", mustUse.Source)
		}
		if a.ReturnType == nil {
			return fmt.Errorf("%v @must_use can only be used on a function with a return type", mustUse.Source)
		}
		overload.MustUse = true
	}
	if constEvalFn := a.Attributes.Take("const"); constEvalFn != nil {
		switch len(constEvalFn.Values) {
		case 0:
			switch overload.Decl.Kind {
			case ast.Builtin, ast.Operator:
				overload.ConstEvalFunction = overload.Decl.Name
			case ast.Constructor:
				overload.ConstEvalFunction = "Ctor"
			case ast.Converter:
				overload.ConstEvalFunction = "Conv"
			}
		case 1:
			fn, ok := constEvalFn.Values[0].(string)
			if !ok {
				return fmt.Errorf("%v optional @const value must be a string", constEvalFn.Source)
			}
			overload.ConstEvalFunction = fn
		default:
			return fmt.Errorf("%v too many values for @const attribute", constEvalFn.Source)
		}
	}
	if memberFunction := a.Attributes.Take("member_function"); memberFunction != nil {
		overload.MemberFunction = true
		if len(memberFunction.Values) != 0 {
			return fmt.Errorf("%v unexpected value for member_function attribute", memberFunction.Source)
		}
		if len(a.Parameters) < 1 {
			return fmt.Errorf("%v @member_function can only be used on a function with at least one parameter", memberFunction.Source)
		}
	}
	if deprecated := a.Attributes.Take("deprecated"); deprecated != nil {
		overload.IsDeprecated = true
		if len(deprecated.Values) != 0 {
			return fmt.Errorf("%v unexpected value for deprecated attribute", deprecated.Source)
		}
	}
	if len(a.Attributes) != 0 {
		return fmt.Errorf("%v unknown attribute", a.Attributes[0].Source)
	}

	// Append the overload to the intrinsic
	intrinsic.Overloads = append(intrinsic.Overloads, overload)

	// Update high-water mark of templates
	if n := len(overload.AllTemplates()); r.s.MaxTemplates < n {
		r.s.MaxTemplates = n
	}

	// Resolve the parameters
	for i, p := range a.Parameters {
		usage, err := r.fullyQualifiedName(&s, p.Type)
		if err != nil {
			return err
		}
		isConst := false
		if attribute := p.Attributes.Take("const"); attribute != nil {
			isConst = true
		}
		testValue := 1.0
		if attribute := p.Attributes.Take("test_value"); attribute != nil {
			switch v := attribute.Values[0].(type) {
			case int:
				testValue = float64(v)
			case float64:
				testValue = v
			default:
				return fmt.Errorf("%v @test_value must be an integer or float", p.Attributes[0].Source)
			}
		}
		if len(p.Attributes) != 0 {
			return fmt.Errorf("%v unknown attribute", p.Attributes[0].Source)
		}
		overload.Parameters[i] = sem.Parameter{
			Name:      p.Name,
			Type:      usage,
			IsConst:   isConst,
			TestValue: testValue,
		}
	}

	// Resolve the return type
	if a.ReturnType != nil {
		usage, err := r.fullyQualifiedName(&s, *a.ReturnType)
		if err != nil {
			return err
		}
		switch usage.Target.(type) {
		case *sem.Type, *sem.TemplateTypeParam:
			overload.ReturnType = &usage
		default:
			return fmt.Errorf("%v cannot use '%v' as return type. Must be a type or template type", a.ReturnType.Source, a.ReturnType.Name)
		}
	}

	return nil
}

// fullyQualifiedName() resolves the ast.TemplatedName to a sem.FullyQualifiedName.
// The resolved name cannot be a TypeMatcher
func (r *resolver) fullyQualifiedName(s *scope, arg ast.TemplatedName) (sem.FullyQualifiedName, error) {
	target, err := r.lookupNamed(s, arg)
	if err != nil {
		return sem.FullyQualifiedName{}, err
	}
	fqn := sem.FullyQualifiedName{
		Target:            target,
		TemplateArguments: make([]interface{}, len(arg.TemplateArgs)),
	}
	for i, a := range arg.TemplateArgs {
		arg, err := r.fullyQualifiedName(s, a)
		if err != nil {
			return sem.FullyQualifiedName{}, err
		}
		fqn.TemplateArguments[i] = arg
	}
	return fqn, nil
}

// templateParams() resolves the ast.TemplateParams list into a sem.TemplateParam list.
// Each sem.TemplateParam is registered with the scope s.
func (r *resolver) templateParams(s *scope, l []ast.TemplateParam) ([]sem.TemplateParam, error) {
	out := make([]sem.TemplateParam, 0, len(l))
	for _, ast := range l {
		param, err := r.templateParam(s, ast)
		if err != nil {
			return nil, err
		}
		if err := s.declare(param, ast.Source); err != nil {
			return nil, err
		}
		out = append(out, param)
	}
	return out, nil
}

// templateParams() resolves the ast.TemplateParam into sem.TemplateParam, which
// is either a sem.TemplateEnumParam or a sem.TemplateTypeParam.
func (r *resolver) templateParam(s *scope, a ast.TemplateParam) (sem.TemplateParam, error) {
	if a.Type.Name == "num" {
		return &sem.TemplateNumberParam{Name: a.Name, ASTParam: a}, nil
	}

	if a.Type.Name != "" {
		resolved, err := r.fullyQualifiedName(s, a.Type)
		if err != nil {
			return nil, err
		}
		switch r := resolved.Target.(type) {
		case *sem.Enum:
			return &sem.TemplateEnumParam{Name: a.Name, ASTParam: a, Enum: r}, nil
		case *sem.EnumMatcher:
			return &sem.TemplateEnumParam{Name: a.Name, ASTParam: a, Enum: r.Enum, Matcher: r}, nil
		case *sem.TypeMatcher:
			return &sem.TemplateTypeParam{Name: a.Name, ASTParam: a, Type: &resolved}, nil
		case *sem.Type:
			return &sem.TemplateTypeParam{Name: a.Name, ASTParam: a, Type: &resolved}, nil
		default:
			return nil, fmt.Errorf("%v invalid template parameter type '%v'", a.Source, a.Type.Name)
		}
	}

	return &sem.TemplateTypeParam{Name: a.Name}, nil
}

// lookupType() searches the scope `s` and its ancestors for the sem.Type with
// the given name.
func (r *resolver) lookupType(s *scope, a ast.TemplatedName) (*sem.Type, error) {
	resolved, err := r.lookupNamed(s, a)
	if err != nil {
		return nil, err
	}
	// Something with the given name was found...
	if ty, ok := resolved.(*sem.Type); ok {
		return ty, nil
	}
	// ... but that something was not a sem.Type
	return nil, fmt.Errorf("%v '%v' resolves to %v but type is expected", a.Source, a.Name, describe(resolved))
}

// lookupNamed() searches `s` and its ancestors for the sem.Named object with
// the given name. If there are template arguments for the name `a`, then
// lookupNamed() performs basic validation that those arguments can be passed
// to the named object.
func (r *resolver) lookupNamed(s *scope, a ast.TemplatedName) (sem.Named, error) {
	target := s.lookup(a.Name)
	if target == nil {
		return nil, fmt.Errorf("%v cannot resolve '%v'", a.Source, a.Name)
	}

	// Something with the given name was found...
	var params []sem.TemplateParam
	var ty sem.ResolvableType
	switch target := target.object.(type) {
	case *sem.Type:
		ty = target
		params = target.TemplateParams
	case *sem.TypeMatcher:
		ty = target
		params = target.TemplateParams
	case sem.TemplateParam:
		if len(a.TemplateArgs) != 0 {
			return nil, fmt.Errorf("%v '%v' template parameters do not accept template arguments", a.Source, a.Name)
		}
		return target.(sem.Named), nil
	case sem.Named:
		return target, nil
	default:
		panic(fmt.Errorf("unknown resolved type %T", target))
	}
	// ... and that something takes template parameters
	// Check the number of templated name template arguments match the number of
	// templated parameters for the target.
	args := a.TemplateArgs
	if len(params) != len(args) {
		return nil, fmt.Errorf("%v '%v' requires %d template arguments, but %d were provided", a.Source, a.Name, len(params), len(args))
	}

	// Check templated name template argument kinds match the parameter kinds
	for i, ast := range args {
		param := params[i]
		arg, err := r.lookupNamed(s, args[i])
		if err != nil {
			return nil, err
		}

		if err := checkCompatible(arg, param); err != nil {
			return nil, fmt.Errorf("%v %w", ast.Source, err)
		}
	}
	return ty, nil
}

// calculateUniqueParameterNames() iterates over all the parameters of all
// builtin overloads, calculating the list of unique parameter names
func (r *resolver) calculateUniqueParameterNames(usages ast.EnumDecl) []string {
	set := map[string]struct{}{"": {}}
	names := []string{}

	for _, e := range usages.Entries {
		set[e.Name] = struct{}{}
		names = append(names, e.Name)
	}

	for _, intrinsics := range [][]*sem.Intrinsic{
		r.s.Builtins,
		r.s.UnaryOperators,
		r.s.BinaryOperators,
		r.s.ConstructorsAndConverters,
	} {
		for _, i := range intrinsics {
			for _, o := range i.Overloads {
				for _, p := range o.Parameters {
					if _, dup := set[p.Name]; !dup {
						set[p.Name] = struct{}{}
						names = append(names, p.Name)
					}
				}
			}
		}
	}
	sort.Strings(names)
	return names
}

// describe() returns a string describing a sem.Named
func describe(n sem.Named) string {
	switch n := n.(type) {
	case *sem.Type:
		return "type '" + n.Name + "'"
	case *sem.TypeMatcher:
		return "type matcher '" + n.Name + "'"
	case *sem.Enum:
		return "enum '" + n.Name + "'"
	case *sem.EnumMatcher:
		return "enum matcher '" + n.Name + "'"
	case *sem.TemplateTypeParam:
		return "template type"
	case *sem.TemplateEnumParam:
		return "template enum '" + n.Enum.Name + "'"
	case *sem.EnumEntry:
		return "enum entry '" + n.Enum.Name + "." + n.Name + "'"
	case *sem.TemplateNumberParam:
		return "template number"
	default:
		panic(fmt.Errorf("unhandled type %T", n))
	}
}

// checkCompatible() returns an error if `arg` cannot be used as an argument for
// a parameter of `param`.
func checkCompatible(arg, param sem.Named) error {
	// asEnum() returns the underlying sem.Enum if n is a enum matcher,
	// templated enum parameter or an enum entry, otherwise nil
	asEnum := func(n sem.Named) *sem.Enum {
		switch n := n.(type) {
		case *sem.EnumMatcher:
			return n.Enum
		case *sem.TemplateEnumParam:
			return n.Enum
		case *sem.EnumEntry:
			return n.Enum
		default:
			return nil
		}
	}

	if arg := asEnum(arg); arg != nil {
		param := asEnum(param)
		if arg == param {
			return nil
		}
	}

	anyNumber := "any number"
	// asNumber() returns anyNumber if n is a TemplateNumberParam.
	asNumber := func(n sem.Named) interface{} {
		switch n.(type) {
		case *sem.TemplateNumberParam:
			return anyNumber
		default:
			return nil
		}
	}

	if arg := asNumber(arg); arg != nil {
		param := asNumber(param)
		if arg == param {
			return nil
		}
	}

	anyType := &sem.Type{}
	// asNumber() returns the sem.Type, sem.TypeMatcher if the named object
	// resolves to one of these, or anyType if n is a unconstrained template
	// type parameter.
	asResolvableType := func(n sem.Named) sem.ResolvableType {
		switch n := n.(type) {
		case *sem.TemplateTypeParam:
			if n.Type != nil {
				return n.Type.Target.(sem.ResolvableType)
			}
			return anyType
		case *sem.Type:
			return n
		case *sem.TypeMatcher:
			return n
		default:
			return nil
		}
	}

	if arg := asResolvableType(arg); arg != nil {
		param := asResolvableType(param)
		if arg == param || param == anyType {
			return nil
		}
	}

	return fmt.Errorf("cannot use %v as %v", describe(arg), describe(param))
}

// scope is a basic hierarchical name to object table
type scope struct {
	objects map[string]objectAndSource
	parent  *scope
}

// objectAndSource is a sem.Named object with a source
type objectAndSource struct {
	object sem.Named
	source tok.Source
}

// newScope returns a newly initalized scope
func newScope(parent *scope) scope {
	return scope{objects: map[string]objectAndSource{}, parent: parent}
}

// lookup() searches the scope and then its parents for the symbol with the
// given name.
func (s *scope) lookup(name string) *objectAndSource {
	if o, found := s.objects[name]; found {
		return &o
	}
	if s.parent == nil {
		return nil
	}
	return s.parent.lookup(name)
}

// declare() declares the symbol with the given name, erroring on symbol
// collision.
func (s *scope) declare(object sem.Named, source tok.Source) error {
	name := object.GetName()
	if existing := s.lookup(name); existing != nil {
		return fmt.Errorf("%v '%v' already declared\nFirst declared here: %v", source, name, existing.source)
	}
	s.objects[name] = objectAndSource{object, source}
	return nil
}
