package main

import (
	"fmt"
	"io"
	"math"
	"reflect"
	"slices"
	"strconv"
	"strings"
	"text/template"
)

type Generator struct {
	UseExtPrefix bool
	HeaderName   string
	*Yml
}

func (g *Generator) Gen(dst io.Writer) error {
	t := template.
		New("").
		Funcs(template.FuncMap{
			"SComment":  func(v string, indent int) string { return Comment(v, CommentTypeSingleLine, indent, true) },
			"MComment":  func(v string, indent int) string { return Comment(v, CommentTypeMultiLine, indent, true) },
			"SCommentN": func(v string, indent int) string { return Comment(v, CommentTypeSingleLine, indent, false) },
			"MCommentN": func(v string, indent int) string { return Comment(v, CommentTypeMultiLine, indent, false) },
			"MCommentMainPage": func(v string, indent int) string {
				if v == "" || strings.TrimSpace(v) == "TODO" {
					return ""
				}
				return Comment("\\mainpage\n\n"+strings.TrimSpace(v), CommentTypeMultiLine, indent, true)
			},
			"MCommentEnumValue": func(v string, indent int, e Enum, entryIndex int) string {
				var s string
				v = strings.TrimSpace(v)
				if v != "" && v != "TODO" {
					s += v
				}
				value, _ := g.EnumValue32(e, entryIndex)
				if value == 0 {
					s = "`0`. " + s
				}
				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentBitflagType": func(v string, indent int) string {
				var s string
				v = strings.TrimSpace(v)
				if v != "" && v != "TODO" {
					s += v
				}
				s += "\n\nFor reserved non-standard bitflag values, see @ref BitflagRegistry."
				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentBitflagValue": func(v string, indent int, b Bitflag, entryIndex int) string {
				value, _ := g.BitflagValue(b, entryIndex, true)
				s := value + "\n"
				v = strings.TrimSpace(v)
				if v != "" && v != "TODO" {
					s += v
				}
				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentFunction": func(fn *Function, indent int) string {
				var s string
				{
					var funcDoc = strings.TrimSpace(fn.Doc)
					if funcDoc != "" && funcDoc != "TODO" {
						s += funcDoc
					}
				}
				for _, arg := range fn.Args {
					argDoc := strings.TrimSpace(arg.Doc)
					var sArg string
					if argDoc != "" && argDoc != "TODO" {
						sArg = argDoc
					}

					if arg.PassedWithOwnership != nil {
						if *arg.PassedWithOwnership {
							sArg += "\nThis parameter is @ref ReturnedWithOwnership."
						} else {
							panic("invalid")
						}
					}

					sArg = strings.TrimSpace(sArg)
					if sArg != "" {
						s += "\n\n@param " + CamelCase(arg.Name) + "\n" + sArg
					}
				}
				if fn.Returns != nil {
					returnsDoc := strings.TrimSpace(fn.Returns.Doc)
					var sRet string
					if returnsDoc != "" && returnsDoc != "TODO" {
						sRet = returnsDoc
					}

					if fn.Returns.PassedWithOwnership != nil {
						if *fn.Returns.PassedWithOwnership {
							sRet += "\nThis value is @ref ReturnedWithOwnership."
						} else {
							panic("invalid")
						}
					}

					sRet = strings.TrimSpace(sRet)
					if sRet != "" {
						s += "\n\n@returns\n" + sRet
					}
				}
				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentCallback": func(cb *Callback, indent int) string {
				var s string
				{
					var funcDoc = strings.TrimSpace(cb.Doc)
					if funcDoc != "" && funcDoc != "TODO" {
						s += funcDoc
					}
					s += "\n\nSee also @ref CallbackError."
				}
				for _, arg := range cb.Args {
					var argDoc = strings.TrimSpace(arg.Doc)
					var sArg string
					if argDoc != "" && argDoc != "TODO" {
						sArg += argDoc
					}

					if arg.PassedWithOwnership != nil {
						if *arg.PassedWithOwnership {
							sArg += "\nThis parameter is @ref PassedWithOwnership."
						} else {
							sArg += "\nThis parameter is @ref PassedWithoutOwnership."
						}
					}

					sArg = strings.TrimSpace(sArg)
					if sArg != "" {
						s += "\n\n@param " + CamelCase(arg.Name) + "\n" + sArg
					}
				}
				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentMember": func(member *ParameterType, indent int) string {
				var s string

				var srcDoc = strings.TrimSpace(member.Doc)
				if srcDoc != "" && srcDoc != "TODO" {
					s += srcDoc
				}

				switch member.Type {
				case "nullable_string":
					s += "\n\nThis is a \\ref NullableInputString."
				case "string_with_default_empty":
					s += "\n\nThis is a \\ref NonNullInputString."
				case "out_string":
					s += "\n\nThis is an \\ref OutputString."
				}

				s += "\n\nThe `INIT` macro sets this to " + g.DefaultValue(*member, true /* isDocString */) + "."

				if member.PassedWithOwnership != nil {
					panic("invalid")
				}

				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentStruct": func(st *Struct, indent int) string {
				var s string

				var srcDoc = strings.TrimSpace(st.Doc)
				if srcDoc != "" && srcDoc != "TODO" {
					s += srcDoc
				}

				if st.Type == "extensible_callback_arg" {
					s += "\n\nThis is an @ref ImplementationAllocatedStructChain root.\nArbitrary chains must be handled gracefully by the application!"
				}

				s += "\n\nDefault values can be set using @ref WGPU_" + g.ConstantCaseName(st.Base) + "_INIT as initializer."

				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"MCommentProcPointer": func(name string, indent int) string {
				var s string
				s += "Proc pointer type for @ref wgpu" + name + ":\n"
				s += "> @copydoc wgpu" + name
				return Comment(strings.TrimSpace(s), CommentTypeMultiLine, indent, true)
			},
			"ConstantCase":     ConstantCase,
			"PascalCase":       PascalCase,
			"CamelCase":        CamelCase,
			"ConstantCaseName": g.ConstantCaseName,
			"PascalCaseName":   g.PascalCaseName,
			"CEnumValueName":   g.CEnumValueName,
			"CMethodName":      g.CMethodName,
			"CType":            g.CType,
			"CValue":           g.CValue,
			"EnumValue32":      g.EnumValue32,
			"BitflagValue": func(b Bitflag, entryIndex int) (string, error) {
				return g.BitflagValue(b, entryIndex, false)
			},
			"IsArray": func(typ string) bool {
				return arrayTypeRegexp.Match([]byte(typ))
			},
			"ArrayType": func(typ string, pointer PointerType) string {
				matches := arrayTypeRegexp.FindStringSubmatch(typ)
				if len(matches) == 2 {
					return g.CType(matches[1], pointer)
				}
				return ""
			},
			"Singularize":             Singularize,
			"IsLast":                  func(i int, s any) bool { return i == reflect.ValueOf(s).Len()-1 },
			"FunctionReturns":         g.FunctionReturns,
			"FunctionArgs":            g.FunctionArgs,
			"CallbackArgs":            g.CallbackArgs,
			"StructMember":            g.StructMember,
			"StructMemberArrayCount":  g.StructMemberArrayCount,
			"StructMemberArrayData":   g.StructMemberArrayData,
			"StructMemberInitializer": g.StructMemberInitializer,
		})
	t, err := t.Parse(tmpl)
	if err != nil {
		return fmt.Errorf("GenCHeader: failed to parse template: %w", err)
	}
	if err := t.Execute(dst, g); err != nil {
		return fmt.Errorf("GenCHeader: failed to execute template: %w", err)
	}
	return nil
}

func (g *Generator) FindBaseType(typ string) Base {
	// Handle type names prefixed with the type category.
	category, name, found := strings.Cut(typ, ".")
	if !found {
		panic("Cannot find base type for invalid type identifier: " + typ)
	}

	switch category {
	case "constant":
		idx := slices.IndexFunc(g.Constants, func(c Constant) bool { return c.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Constants[idx].Base
	case "typedef":
		idx := slices.IndexFunc(g.Typedefs, func(t Typedef) bool { return t.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Typedefs[idx].Base
	case "enum":
		idx := slices.IndexFunc(g.Enums, func(e Enum) bool { return e.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Enums[idx].Base
	case "bitflag":
		idx := slices.IndexFunc(g.Bitflags, func(b Bitflag) bool { return b.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Bitflags[idx].Base
	case "struct":
		idx := slices.IndexFunc(g.Structs, func(s Struct) bool { return s.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Structs[idx].Base
	case "callback":
		idx := slices.IndexFunc(g.Callbacks, func(c Callback) bool { return c.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Callbacks[idx].Base
	case "object":
		idx := slices.IndexFunc(g.Objects, func(o Object) bool { return o.Name == name })
		if idx == -1 {
			return Base{Name: name, Namespace: "webgpu"}
		}
		return g.Objects[idx].Base
	default:
		panic("Unable to find unknown category type: " + category + " for identifier: " + typ)
	}
}

// Top-level items: constants, typedefs, and types (objects/enums/bitflags/structs/callbacks)
func (g *Generator) ResolveNamespaceForTopLevelItem(b Base) string {
	if b.Namespace != "" {
		return b.Namespace
	} else if b.Extended {
		// If we're extending an enum, assume it's in the core namespace if not otherwise specified
		return "webgpu"
	} else {
		return g.Name
	}
}

// Items nested inside other items: methods, enum values, and bitflag values
func (g *Generator) ResolveNamespaceForNestedItem(topLevelItem Base, nestedItem Base) string {
	if nestedItem.Namespace != "" {
		return nestedItem.Namespace
	} else if topLevelItem.Namespace != "" {
		return topLevelItem.Namespace
	} else {
		return g.Name
	}
}

func (g *Generator) CanonicalCaseName(prefix string, b Base) string {
	switch prefix {
	case "":
		return b.Name
	default:
		return prefix + "_" + b.Name
	}
}

func (g *Generator) ConstantCaseName(b Base) string {
	prefix := g.GetNamespacePrefix(g.ResolveNamespaceForTopLevelItem(b))
	return ConstantCase(g.CanonicalCaseName(prefix, b))
}

func (g *Generator) PascalCaseName(b Base) string {
	prefix := g.GetNamespacePrefix(g.ResolveNamespaceForTopLevelItem(b))
	return PascalCase(g.CanonicalCaseName(prefix, b))
}

func (g *Generator) GetNamespacePrefixForNestedItem(topLevelItem Base, nestedItem Base) string {
	outerNamespace := g.ResolveNamespaceForTopLevelItem(topLevelItem)
	innerNamespace := g.ResolveNamespaceForNestedItem(topLevelItem, nestedItem)
	if outerNamespace == innerNamespace {
		return ""
	} else {
		return g.GetNamespacePrefix(innerNamespace)
	}
}

func (g *Generator) CEnumValueName(typ Base, entry Base) string {
	entryPrefix := g.GetNamespacePrefixForNestedItem(typ, entry)
	return g.CType(typ, "") + "_" + PascalCase(g.CanonicalCaseName(entryPrefix, entry))
}

func (g *Generator) CMethodName(o Object, m Function) string {
	entryPrefix := g.GetNamespacePrefixForNestedItem(o.Base, m.Base)
	return g.PascalCaseName(o.Base) + PascalCase(g.CanonicalCaseName(entryPrefix, m.Base))
}

func (g *Generator) CValue(s string) (string, error) {
	switch s {
	case "usize_max":
		return "SIZE_MAX", nil
	case "uint32_max":
		return "UINT32_MAX", nil
	case "uint64_max":
		return "UINT64_MAX", nil
	case "nan":
		return "NAN", nil
	default:
		var num string
		var base int
		if strings.HasPrefix(s, "0x") {
			base = 16
			num = strings.TrimPrefix(s, "0x")
		} else {
			base = 10
			num = s
		}
		v, err := strconv.ParseUint(num, base, 64)
		if err != nil {
			return "", fmt.Errorf("CValue: failed to parse \"%s\": %w", s, err)
		}
		var suffix string
		if v <= math.MaxUint32 {
			suffix = "UL"
		} else {
			suffix = "ULL"
		}
		return "0x" + strconv.FormatUint(v, 16) + suffix, nil
	}
}

func (g *Generator) CType(typ any, pointerType PointerType) string {
	appendModifiers := func(s string, pointerType PointerType) string {
		var sb strings.Builder
		sb.WriteString(s)
		switch pointerType {
		case PointerTypeImmutable:
			sb.WriteString(" const *")
		case PointerTypeMutable:
			sb.WriteString(" *")
		}
		return sb.String()
	}

	var ctype string
	switch t := typ.(type) {
	case string:
		{
			switch t {
			case "bool":
				ctype = "WGPUBool"
			case "nullable_string", "string_with_default_empty", "out_string":
				ctype = "WGPUStringView"
			case "uint16":
				ctype = "uint16_t"
			case "uint32":
				ctype = "uint32_t"
			case "uint64":
				ctype = "uint64_t"
			case "usize":
				ctype = "size_t"
			case "int16":
				ctype = "int16_t"
			case "int32":
				ctype = "int32_t"
			case "float32", "nullable_float32":
				ctype = "float"
			case "float64", "float64_supertype":
				ctype = "double"
			case "c_void":
				ctype = "void"
			default:
				// Handle type names prefixed with the type category.
				return g.CType(g.FindBaseType(t), pointerType)
			}
		}
	case Base:
		{
			ctype = "WGPU" + g.PascalCaseName(t)
		}
	default:
		panic("Unknown input for type")
	}

	return appendModifiers(ctype, pointerType)
}

func (g *Generator) FunctionReturns(f Function) string {
	if f.Callback != nil {
		return "WGPUFuture"
	}
	if f.Returns != nil {
		sb := &strings.Builder{}
		if f.Returns.Optional {
			sb.WriteString("WGPU_NULLABLE ")
		}
		sb.WriteString(g.CType(f.Returns.Type, f.Returns.Pointer))
		return sb.String()
	}
	return "void"
}

func (g *Generator) FunctionArgs(f Function, o *Object) string {
	sb := &strings.Builder{}
	if o != nil {
		if len(f.Args) > 0 {
			fmt.Fprintf(sb, "%s %s, ", g.CType(o.Base, ""), CamelCase(o.Name))
		} else {
			fmt.Fprintf(sb, "%s %s", g.CType(o.Base, ""), CamelCase(o.Name))
		}
	}
	for i, arg := range f.Args {
		if arg.Optional {
			sb.WriteString("WGPU_NULLABLE ")
		}
		matches := arrayTypeRegexp.FindStringSubmatch(arg.Type)
		if len(matches) == 2 {
			fmt.Fprintf(sb, "size_t %sCount, ", CamelCase(Singularize(arg.Name)))
			fmt.Fprintf(sb, "%s %s", g.CType(matches[1], arg.Pointer), CamelCase(arg.Name))
		} else {
			fmt.Fprintf(sb, "%s %s", g.CType(arg.Type, arg.Pointer), CamelCase(arg.Name))
		}
		if i != len(f.Args)-1 {
			sb.WriteString(", ")
		}
	}
	if f.Callback != nil {
		fmt.Fprintf(sb, ", %sCallbackInfo callbackInfo", g.CType(*f.Callback, ""))
	}
	return sb.String()
}

func (g *Generator) CallbackArgs(f Callback) string {
	sb := &strings.Builder{}
	for _, arg := range f.Args {
		if arg.Optional {
			sb.WriteString("WGPU_NULLABLE ")
		}
		var structPrefix string
		if strings.HasPrefix(arg.Type, "struct.") {
			structPrefix = "struct "
		}
		matches := arrayTypeRegexp.FindStringSubmatch(arg.Type)
		if len(matches) == 2 {
			fmt.Fprintf(sb, "size_t %sCount, ", CamelCase(Singularize(arg.Name)))
			fmt.Fprintf(sb, "%s%s %s, ", structPrefix, g.CType(matches[1], arg.Pointer), CamelCase(arg.Name))
		} else {
			fmt.Fprintf(sb, "%s%s %s, ", structPrefix, g.CType(arg.Type, arg.Pointer), CamelCase(arg.Name))
		}
	}
	sb.WriteString("WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2")
	return sb.String()
}

func (g *Generator) EnumValue32(e Enum, entryIndex int) (uint32, error) {
	entry := e.Entries[entryIndex]

	var enum_prefix uint16
	if entry.Namespace != "" {
		if g.EnumPrefix != 0 {
			return 0, fmt.Errorf("EnumValue32: entry %s with overridden namespace %s can only be used in core webgpu.h (with global enum_prefix 0)", entry.Name, entry.Namespace)
		}
		if entry.Value == nil {
			return 0, fmt.Errorf("EnumValue32: entry %s with overridden namespace %s must have an explicit value", entry.Name, entry.Namespace)
		}
		switch entry.Namespace {
		case "compatibility_mode":
			enum_prefix = 0x2000
		default:
			return 0, fmt.Errorf("EnumValue32: unknown namespace %s", entry.Namespace)
		}
	} else {
		enum_prefix = g.EnumPrefix
	}

	var value16 uint16
	if entry.Value == nil {
		value16 = uint16(entryIndex)
		if int(value16) != entryIndex {
			return 0, fmt.Errorf("EnumValue32: entry %s default value (entry index %d) is too large", entry.Name, entryIndex)
		}
	} else {
		value16 = *entry.Value
	}

	return uint32(enum_prefix)<<16 | uint32(value16), nil
}

func bitflagEntryValue(entry BitflagEntry, entryIndex int) (uint64, error) {
	if entry.Value == "" {
		value := uint64(math.Pow(2, float64(entryIndex-1)))
		return value, nil
	} else {
		var num string
		var base int
		if strings.HasPrefix(entry.Value, "0x") {
			base = 16
			num = strings.TrimPrefix(entry.Value, "0x")
		} else {
			base = 10
			num = entry.Value
		}
		return strconv.ParseUint(num, base, 64)
	}
}

func (g *Generator) BitflagValue(b Bitflag, entryIndex int, isDocString bool) (string, error) {
	entry := b.Entries[entryIndex]

	var value uint64
	var entryComment string
	if len(entry.ValueCombination) > 0 {
		if entry.Value != "" {
			return "", fmt.Errorf("BitflagValue: found conflicting 'value' and 'value_combination' in '%s'", b.Name)
		}
		entryComment += "`"
		for valueIndex, v := range entry.ValueCombination {
			// find the value by searching in b, bitwise-OR it into the result
			for searchIndex, search := range b.Entries {
				if search.Name == v {
					searchValue, err := bitflagEntryValue(search, searchIndex)
					if err != nil {
						return "", nil
					}
					value |= searchValue
					break
				}
			}
			// construct comment
			idx := slices.IndexFunc(b.Entries, func(e BitflagEntry) bool { return e.Name == v })
			if idx != -1 {
				entryComment += g.PascalCaseName(b.Entries[idx].Base)
			} else {
				entryComment += PascalCase(v)
			}
			if valueIndex != len(entry.ValueCombination)-1 {
				entryComment += " | "
			}
		}
		entryComment += "`."
	} else {
		var err error
		value, err = bitflagEntryValue(entry, entryIndex)
		if err != nil {
			return "", nil
		}
		if value == 0 {
			entryComment = "`0`."
		}
	}
	if isDocString {
		return entryComment, nil
	} else {
		return fmt.Sprintf("0x%.16X", value), nil
	}
}

func (g *Generator) GetNamespacePrefix(namespace string) string {
	switch namespace {
	case "":
		panic("Missing namespace")
	case "webgpu":
		return ""
	case "compatibility_mode":
		return ""
	default:
		if g.UseExtPrefix {
			return namespace
		} else {
			return ""
		}
	}
}

func (g *Generator) StructMember(s Struct, memberIndex int) (string, error) {
	member := s.Members[memberIndex]

	matches := arrayTypeRegexp.FindStringSubmatch(member.Type)
	if len(matches) == 2 {
		panic("StructMember used on array type")
	}

	sb := &strings.Builder{}
	if member.Optional {
		sb.WriteString("WGPU_NULLABLE ")
	}
	if strings.HasPrefix(member.Type, "callback.") {
		fmt.Fprintf(sb, "%sCallbackInfo %s;", g.CType(member.Type, ""), CamelCase(member.Name))
	} else {
		fmt.Fprintf(sb, "%s %s;", g.CType(member.Type, member.Pointer), CamelCase(member.Name))
	}
	return sb.String(), nil
}

func (g *Generator) StructMemberArrayCount(s Struct, memberIndex int) (string, error) {
	member := s.Members[memberIndex]

	matches := arrayTypeRegexp.FindStringSubmatch(member.Type)
	if len(matches) != 2 {
		panic("StructMemberArrayCount used on non-array")
	}

	return fmt.Sprintf("size_t %sCount;", CamelCase(Singularize(member.Name))), nil
}

func (g *Generator) StructMemberArrayData(s Struct, memberIndex int) (string, error) {
	member := s.Members[memberIndex]

	matches := arrayTypeRegexp.FindStringSubmatch(member.Type)
	if len(matches) != 2 {
		panic("StructMemberArrayCount used on non-array")
	}

	sb := &strings.Builder{}
	if member.Optional {
		sb.WriteString("WGPU_NULLABLE ")
	}
	fmt.Fprintf(sb, "%s %s;", g.CType(matches[1], member.Pointer), CamelCase(member.Name))
	return sb.String(), nil
}

func (g *Generator) StructMemberInitializer(s Struct, memberIndex int) (string, error) {
	member := s.Members[memberIndex]
	sb := &strings.Builder{}
	matches := arrayTypeRegexp.FindStringSubmatch(member.Type)
	if len(matches) == 2 {
		fmt.Fprintf(sb, "/*.%sCount=*/0 _wgpu_COMMA \\\n", CamelCase(Singularize(member.Name)))
		fmt.Fprintf(sb, "    /*.%s=*/NULL _wgpu_COMMA \\", CamelCase(member.Name))
	} else {
		fmt.Fprintf(sb, "/*.%s=*/%s _wgpu_COMMA \\", CamelCase(member.Name), g.DefaultValue(member, false /* isDocString */))
	}
	return sb.String(), nil
}

func (g *Generator) DefaultValue(member ParameterType, isDocString bool) string {
	ref := func(s string) string {
		if isDocString {
			return "@ref " + s
		} else {
			return s
		}
	}
	literal := func(s string) string {
		if isDocString {
			return "`" + s + "`"
		} else {
			return s
		}
	}

	switch {
	case member.Pointer != "":
		if member.Default != nil {
			panic("pointer type should not have a default")
		}
		return literal("NULL")

	// Cases that may have member.Default
	case strings.HasPrefix(member.Type, "enum."):
		if member.Default == nil {
			_, name, _ := strings.Cut(member.Type, ".")

			// Find the enum type.
			idx := slices.IndexFunc(g.Enums, func(e Enum) bool { return e.Name == name })
			if idx == -1 {
				panic("Invalid enum type: " + name)
			}

			// If the enum type has an explicit "Undefined" use it, otherwise, use 0.
			enumType := g.Enums[idx]
			undefIdx := slices.IndexFunc(enumType.Entries, func(entry *EnumEntry) bool { return entry != nil && entry.Name == "undefined" })
			if undefIdx == -1 {
				if isDocString {
					return "(@ref " + g.CType(member.Type, "") + ")0"
				} else {
					return "_wgpu_ENUM_ZERO_INIT(" + g.CType(member.Type, "") + ")"
				}
			} else {
				return ref(g.CType(member.Type, "") + "_" + PascalCase("undefined"))
			}
		} else {
			return ref(g.CType(member.Type, "") + "_" + PascalCase(*member.Default))
		}
	case strings.HasPrefix(member.Type, "bitflag."):
		if member.Default == nil {
			return ref(g.CType(member.Type, "") + "_None")
		} else {
			return ref(g.CType(member.Type, "") + "_" + PascalCase(*member.Default))
		}
	case member.Type == "uint16", member.Type == "uint32", member.Type == "uint64", member.Type == "usize", member.Type == "int32":
		if member.Default == nil {
			return literal("0")
		} else if strings.HasPrefix(*member.Default, "constant.") {
			return ref("WGPU_" + g.ConstantCaseName(g.FindBaseType(*member.Default)))
		} else {
			return literal(*member.Default)
		}
	case member.Type == "float32" || member.Type == "nullable_float32":
		if member.Default == nil {
			return literal("0.f")
		} else if strings.HasPrefix(*member.Default, "constant.") {
			return ref("WGPU_" + g.ConstantCaseName(g.FindBaseType(*member.Default)))
		} else if strings.Contains(*member.Default, ".") {
			return literal(*member.Default + "f")
		} else {
			return literal(*member.Default + ".f")
		}
	case member.Type == "float64" || member.Type == "float64_supertype":
		if member.Default == nil {
			return literal("0.")
		} else if strings.HasPrefix(*member.Default, "constant.") {
			return ref("WGPU_" + g.ConstantCaseName(g.FindBaseType(*member.Default)))
		} else {
			return literal(*member.Default)
		}
	case member.Type == "bool":
		if member.Default == nil {
			return literal("WGPU_FALSE")
		} else if strings.HasPrefix(*member.Default, "constant.") {
			return ref("WGPU_" + g.ConstantCaseName(g.FindBaseType(*member.Default)))
		} else if *member.Default == "true" {
			return literal("WGPU_TRUE")
		} else if *member.Default == "false" {
			return literal("WGPU_FALSE")
		} else {
			return *member.Default
		}
	case strings.HasPrefix(member.Type, "struct."):
		if member.Optional {
			return literal("NULL")
		} else if member.Default == nil {
			return ref("WGPU_" + g.ConstantCaseName(g.FindBaseType(member.Type)) + "_INIT")
		} else if *member.Default == "zero" {
			if isDocString {
				return "zero (which sets the entry to `BindingNotUsed`)"
			} else {
				return literal("_wgpu_STRUCT_ZERO_INIT")
			}
		} else {
			panic("unknown default for struct type")
		}
	case member.Default != nil:
		panic(fmt.Errorf("type %s should not have a default", member.Type))

	// Cases that should not have member.Default
	case strings.HasPrefix(member.Type, "callback."):
		return ref("WGPU_" + g.ConstantCaseName(g.FindBaseType(member.Type)) + "_CALLBACK_INFO_INIT")
	case strings.HasPrefix(member.Type, "object."):
		return literal("NULL")
	case strings.HasPrefix(member.Type, "array<"):
		return literal("NULL")
	case member.Type == "out_string", member.Type == "string_with_default_empty", member.Type == "nullable_string":
		return ref("WGPU_STRING_VIEW_INIT")
	case member.Type == "c_void":
		return literal("NULL")
	default:
		panic("invalid prefix: " + member.Type + " in member " + member.Name)
	}
}
