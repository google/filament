// Copyright 2024 The langsvr Authors
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
	"regexp"
	"strings"

	"github.com/google/langsvr/tools/cmd/gen/json"
	"github.com/google/langsvr/tools/cmd/gen/protocol"
	"github.com/google/langsvr/tools/text"
)

// Resolve resolves the JSON MetaModel to a Protocol, which can be consumed by the templates
func Resolve(model json.MetaModel) (*protocol.Protocol, error) {
	r := resolver{}
	out := r.model(model)
	if r.err != nil {
		return &protocol.Protocol{}, r.err
	}
	return out, nil
}

type resolver struct {
	stack                   []string
	err                     error
	newStructureLiteralType func(*json.StructureLiteralType) protocol.Type
	allReferenceTypes       []*protocol.ReferenceType
	typeDecls               map[string]protocol.TypeDecl
}

func (r *resolver) pushScope(msg string, args ...any) (pop func()) {
	r.stack = append(r.stack, fmt.Sprintf(msg, args...))
	return func() { r.stack = r.stack[:len(r.stack)-1] }
}

func (r *resolver) error(msg string, args ...any) {
	if r.err != nil {
		return
	}
	sb := strings.Builder{}
	sb.WriteString(fmt.Sprintf(msg, args...))
	for i := len(r.stack) - 1; i >= 0; i-- {
		sb.WriteString("\nwhile ")
		sb.WriteString(r.stack[i])
	}
	r.err = fmt.Errorf("%v", sb.String())
}

func (r *resolver) model(in json.MetaModel) *protocol.Protocol {
	defer r.pushScope("resolving model")()
	out := &protocol.Protocol{
		Enumerations:           transform(in.Enumerations, r.enumeration),
		MetaData:               r.metadata(in.MetaData),
		Notifications:          transform(in.Notifications, r.notification),
		Requests:               transform(in.Requests, r.request),
		Structures:             transform(in.Structures, r.structure),
		TypeAliases:            transform(in.TypeAliases, r.typeAlias),
		ServerToClientRequests: []*protocol.Request{},
		ClientToServerRequests: []*protocol.Request{},
	}

	r.resolveTypes(out)

	for _, request := range out.Requests {
		switch request.MessageDirection {
		case protocol.MessageDirectionClientToServer:
			out.ClientToServerRequests = append(out.ClientToServerRequests, request)
		case protocol.MessageDirectionServerToClient:
			out.ServerToClientRequests = append(out.ServerToClientRequests, request)
		case protocol.MessageDirectionBidirectional:
			out.ClientToServerRequests = append(out.ClientToServerRequests, request)
			out.ServerToClientRequests = append(out.ServerToClientRequests, request)
		}
	}
	out.TypeAliases = r.sortTypeAliases(out.TypeAliases)
	out.Structures = r.sortStructures(out.Structures)

	return out
}

func (r *resolver) enumeration(in json.Enumeration) *protocol.Enumeration {
	defer r.pushScope("resolving enumeration '%v'", in.Name)()
	return &protocol.Enumeration{
		Deprecated:           in.Deprecated,
		Documentation:        r.documentation(in.Documentation),
		Name:                 r.className(in.Name),
		Proposed:             in.Proposed,
		Since:                in.Since,
		SupportsCustomValues: in.SupportsCustomValues,
		Type:                 r.type_(in.Type),
		Values:               transform(in.Values, r.enumerationEntry),
	}
}

func (r *resolver) enumerationEntry(in json.EnumerationEntry) *protocol.EnumerationEntry {
	defer r.pushScope("resolving enumeration entry '%v'", in.Name)()
	return &protocol.EnumerationEntry{
		Deprecated:    in.Deprecated,
		Documentation: r.documentation(in.Documentation),
		Name:          r.className(in.Name),
		Proposed:      in.Proposed,
		Since:         in.Since,
		Value:         r.value(in.Value),
	}
}

func (r *resolver) metadata(in json.MetaData) *protocol.MetaData {
	defer r.pushScope("resolving metadata")()
	return &protocol.MetaData{Version: in.Version}
}

func (r *resolver) notification(in json.Notification) *protocol.Notification {
	defer r.pushScope("resolving notification '%v'", in.Method)()
	return &protocol.Notification{
		Deprecated:          in.Deprecated,
		Documentation:       r.documentation(in.Documentation),
		MessageDirection:    r.messageDirection(in.MessageDirection),
		Method:              in.Method,
		Params:              r.types(in.Params),
		Proposed:            in.Proposed,
		RegistrationMethod:  in.RegistrationMethod,
		RegistrationOptions: r.type_(in.RegistrationOptions),
		Since:               in.Since,
		Name:                r.className(in.Method),
	}
}

func (r *resolver) request(in json.Request) *protocol.Request {
	defer r.pushScope("resolving request '%v'", in.Method)()
	return &protocol.Request{
		Deprecated:          in.Deprecated,
		Documentation:       r.documentation(in.Documentation),
		ErrorData:           r.type_(in.ErrorData),
		MessageDirection:    r.messageDirection(in.MessageDirection),
		Method:              in.Method,
		Params:              r.types(in.Params),
		PartialResult:       r.type_(in.PartialResult),
		Proposed:            in.Proposed,
		RegistrationMethod:  in.RegistrationMethod,
		RegistrationOptions: r.type_(in.RegistrationOptions),
		Result:              r.type_(in.Result),
		Since:               in.Since,
		Name:                r.className(in.Method),
	}
}

func (r *resolver) structure(in json.Structure) *protocol.Structure {
	defer r.pushScope("resolving structure '%v'", in.Name)()
	name := r.className(in.Name)
	out := &protocol.Structure{
		Deprecated:    in.Deprecated,
		Documentation: r.documentation(in.Documentation),
		Extends:       transform(in.Extends, r.type_),
		Mixins:        transform(in.Mixins, r.type_),
		Name:          name,
		Proposed:      in.Proposed,
		Since:         in.Since,
		NestedNames:   []string{name},
	}
	for _, propertyIn := range in.Properties {
		defer scopedAssignment(&r.newStructureLiteralType, func(in *json.StructureLiteralType) protocol.Type {
			name := strings.Title(propertyIn.Name)
			out.NestedStructures = append(out.NestedStructures,
				&protocol.Structure{
					Deprecated:    in.Value.Deprecated,
					Documentation: r.documentation(in.Value.Documentation),
					Properties:    transform(in.Value.Properties, r.property),
					Name:          name,
					Proposed:      in.Value.Proposed,
					Since:         in.Value.Since,
					NestedNames:   append(append([]string{}, out.NestedNames...), name),
				},
			)
			ref := &protocol.ReferenceType{Name: out.Name + "::" + name}
			r.allReferenceTypes = append(r.allReferenceTypes, ref)
			return ref
		})()
		propertyOut := r.property(propertyIn)
		if propertyOut.JsonName == "kind" {
			if lit, ok := propertyOut.Type.(*protocol.StringLiteralType); ok {
				out.Kind = lit.Value
				continue
			}
		}
		out.Properties = append(out.Properties, propertyOut)
	}
	return out
}

func (r *resolver) property(in json.Property) *protocol.Property {
	defer r.pushScope("resolving property '%v'", in.Name)()
	return &protocol.Property{
		Deprecated:    in.Deprecated,
		Documentation: r.documentation(in.Documentation),
		JsonName:      in.Name,
		CppName:       text.SnakeCase(in.Name),
		Optional:      in.Optional,
		Proposed:      in.Proposed,
		Since:         in.Since,
		Type:          r.type_(in.Type),
	}
}

func (r *resolver) typeAlias(in json.TypeAlias) *protocol.TypeAlias {
	defer r.pushScope("resolving type alias '%v'", in.Name)()
	return &protocol.TypeAlias{
		Deprecated:    in.Deprecated,
		Documentation: r.documentation(in.Documentation),
		Name:          r.className(in.Name),
		Proposed:      in.Proposed,
		Since:         in.Since,
		Type:          r.type_(in.Type),
	}
}

func (r *resolver) types(in json.Types) []protocol.Type {
	return transform(in.Impl, r.typeImpl)
}

func (r *resolver) type_(in json.Type) protocol.Type {
	return r.typeImpl(in.Impl)
}

func (r *resolver) value(in any) any {
	switch in := in.(type) {
	case string:
		return fmt.Sprintf(`"%v"`, in)
	default:
		return in
	}
}

func (r *resolver) typeImpl(in json.TypeImpl) protocol.Type {
	switch in := in.(type) {
	case nil:
		return nil
	case *json.BaseType:
		switch in.Name {
		case json.URI:
			return &protocol.URIType{}
		case json.DocumentUri:
			return &protocol.DocumentUriType{}
		case json.Integer:
			return &protocol.IntegerType{}
		case json.Uinteger:
			return &protocol.UintegerType{}
		case json.Decimal:
			return &protocol.DecimalType{}
		case json.RegExp:
			return &protocol.RegExpType{}
		case json.String:
			return &protocol.StringType{}
		case json.Boolean:
			return &protocol.BooleanType{}
		case json.Null:
			return &protocol.NullType{}
		}
	case *json.ArrayType:
		return &protocol.ArrayType{Element: r.type_(in.Element)}
	case *json.ReferenceType:
		out := &protocol.ReferenceType{Name: r.className(in.Name)}
		r.allReferenceTypes = append(r.allReferenceTypes, out)
		return out
	case *json.AndType:
		return &protocol.AndType{Items: transform(in.Items, r.type_)}
	case *json.OrType:
		return &protocol.OrType{Items: transform(in.Items, r.type_)}
	case *json.MapType:
		return &protocol.MapType{Key: r.type_(in.Key), Value: r.type_(in.Value)}
	case *json.StringLiteralType:
		return &protocol.StringLiteralType{Value: in.Value}
	case *json.StructureLiteralType:
		return r.newStructureLiteralType(in)
	case *json.TupleType:
		return &protocol.TupleType{Items: transform(in.Items, r.type_)}
	}
	r.error("invalid type %T %+v", in, in)
	return nil
}

func (r *resolver) structureLiteral(in json.StructureLiteral) *protocol.StructureLiteral {
	defer r.pushScope("resolving structure literal")()
	return &protocol.StructureLiteral{
		Deprecated:    in.Deprecated,
		Documentation: r.documentation(in.Documentation),
		Properties:    transform(in.Properties, r.property),
		Proposed:      in.Proposed,
		Since:         in.Since,
	}
}

func (r *resolver) messageDirection(in json.MessageDirection) protocol.MessageDirection {
	switch in {
	case json.MessageDirectionServerToClient:
		return protocol.MessageDirectionServerToClient
	case json.MessageDirectionClientToServer:
		return protocol.MessageDirectionClientToServer
	case json.MessageDirectionBoth:
		return protocol.MessageDirectionBidirectional
	}
	r.error("invalid message direction %+v", in)
	return ""
}

func (r *resolver) className(name string) string {
	if strings.HasPrefix(name, "_") {
		name = strings.TrimLeft(name, "_") + "Base"
	}
	name = strings.TrimLeft(name, "$")
	name = strings.ReplaceAll(name, "/", "_")
	name = text.PascalCase(name)
	return name
}

var reLinkTag = regexp.MustCompile(`{@link[\s]+([^}]+)}`)

func (r *resolver) documentation(in string) string {
	s := reLinkTag.ReplaceAllString(in, "$1")
	s = strings.ReplaceAll(s, "\n", " ")
	s = strings.ReplaceAll(s, "  ", " ")
	s = strings.ReplaceAll(s, "@proposed", "\n\nProposed in:")
	s = strings.ReplaceAll(s, "@sample", "\n\nExample:")
	s = strings.ReplaceAll(s, "@since", "\n\n@since")
	s = strings.ReplaceAll(s, "@deprecated", "\n\nDeprecated:")
	return strings.TrimSpace(s)
}

func (r *resolver) resolveTypes(p *protocol.Protocol) {
	r.typeDecls = map[string]protocol.TypeDecl{}
	register := func(name string, ty protocol.TypeDecl) {
		if existing, found := r.typeDecls[name]; found {
			r.error("duplicate definition for '%v'. %T and %T", name, ty, existing)
			return
		}
		r.typeDecls[name] = ty
	}
	lookup := func(name string) protocol.TypeDecl {
		typeDecl, found := r.typeDecls[name]
		if !found {
			r.error("referenced type '%v' not found", name)
		}
		return typeDecl
	}

	for _, a := range p.TypeAliases {
		register(a.Name, a)
	}
	for _, e := range p.Enumerations {
		register(e.Name, e)
	}
	for _, s := range p.Structures {
		register(s.Name, s)
		for _, n := range s.NestedStructures {
			register(s.Name+"::"+n.Name, s)
		}
	}

	for _, a := range r.allReferenceTypes {
		a.TypeDecl = lookup(a.Name)
	}
}

func (r *resolver) sortTypeAliases(in []*protocol.TypeAlias) []*protocol.TypeAlias {
	aliases := map[string]*protocol.TypeAlias{}
	for _, a := range in {
		aliases[a.Name] = a
	}

	sorted := make([]*protocol.TypeAlias, 0, len(in))
	seen := map[string]struct{}{}
	stack := append([]*protocol.TypeAlias{}, in...)

	for len(stack) > 0 {
		alias := stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		if _, found := seen[alias.Name]; found {
			continue
		}
		seen[alias.Name] = struct{}{}
		for _, ty := range protocol.RecursiveTypesOf(alias.Type) {
			if ref, ok := ty.(*protocol.ReferenceType); ok {
				if dep, ok := aliases[ref.Name]; ok {
					stack = append(stack, dep)
				}
			}
		}
		sorted = append(sorted, alias)
	}

	return sorted
}

func (r *resolver) sortStructures(in []*protocol.Structure) []*protocol.Structure {
	structures := map[string]*protocol.Structure{}
	for _, s := range in {
		structures[s.Name] = s
	}

	sorted := []*protocol.Structure{}
	seen := map[string]struct{}{}

	var visit func(s *protocol.Structure)
	visit = func(s *protocol.Structure) {
		if _, found := seen[s.Name]; found {
			return
		}
		seen[s.Name] = struct{}{}
		for _, ext := range s.Extends {
			if ref, ok := ext.(*protocol.ReferenceType); ok {
				if dep, ok := structures[ref.Name]; ok {
					visit(dep)
				}
			}
		}
		for _, property := range s.Properties {
			for _, ty := range protocol.RecursiveTypesOf(property.Type) {
				if ref, ok := ty.(*protocol.ReferenceType); ok {
					if dep, ok := structures[ref.Name]; ok {
						visit(dep)
					}
				}
			}
		}
		sorted = append(sorted, s)
	}

	for _, s := range in {
		visit(s)
	}
	return sorted
}

func scopedAssignment[T any](p *T, val T) func() {
	old := *p
	*p = val
	return func() { *p = old }
}

// transform returns a new slice by transforming each element with the function fn
func transform[IN any, OUT any](in []IN, fn func(in IN) OUT) []OUT {
	out := make([]OUT, len(in))
	for i, el := range in {
		out[i] = fn(el)
	}
	return out
}
