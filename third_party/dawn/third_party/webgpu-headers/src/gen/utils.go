package main

import (
	"bufio"
	"flag"
	"fmt"
	"strings"
	"unicode"
)

type StringListFlag []string

var StringListFlagI flag.Value = &StringListFlag{}

func (f *StringListFlag) String() string     { return fmt.Sprintf("%#v", f) }
func (f *StringListFlag) Set(v string) error { *f = append(*f, v); return nil }

type CommentType uint8

const (
	CommentTypeSingleLine CommentType = iota
	CommentTypeMultiLine
)

func Comment(in string, mode CommentType, indent int, newline bool) string {
	if in == "" || strings.TrimSpace(in) == "TODO" {
		return ""
	}

	const space = ' '
	var out strings.Builder
	if newline {
		out.WriteString("\n")
	}
	if mode == CommentTypeMultiLine {
		for i := 0; i < indent; i++ {
			out.WriteRune(space)
		}
		out.WriteString("/**\n")
	}
	sc := bufio.NewScanner(strings.NewReader(strings.TrimSpace(in)))
	for sc.Scan() {
		line := sc.Text()
		for i := 0; i < indent; i++ {
			out.WriteRune(space)
		}
		switch mode {
		case CommentTypeSingleLine:
			out.WriteString("//")
		case CommentTypeMultiLine:
			out.WriteString(" *")
		default:
			panic("unreachable")
		}
		if line != "" {
			out.WriteString(" ")
			out.WriteString(line)
		}
		out.WriteString("\n")
	}
	if mode == CommentTypeMultiLine {
		for i := 0; i < indent; i++ {
			out.WriteRune(space)
		}
		out.WriteString(" */")
	}
	return out.String()
}

func ConstantCase(v string) string {
	return strings.ToUpper(v)
}

func PascalCase(s string) string {
	var out strings.Builder
	out.Grow(len(s))
	nextUpper := true
	for _, c := range s {
		if nextUpper {
			out.WriteRune(unicode.ToUpper(c))
			nextUpper = false
		} else {
			if c == '_' {
				nextUpper = true
			} else {
				out.WriteRune(c)
			}
		}
	}
	return out.String()
}

func CamelCase(s string) string {
	var out strings.Builder
	out.Grow(len(s))
	nextUpper := false
	for _, c := range s {
		if nextUpper {
			out.WriteRune(unicode.ToUpper(c))
			nextUpper = false
		} else {
			if c == '_' {
				nextUpper = true
			} else {
				out.WriteRune(c)
			}
		}
	}
	return out.String()
}

func Singularize(s string) string {
	switch s {
	case "entries":
		return "entry"
	default:
		return strings.TrimSuffix(s, "s")
	}
}

func TrimTypePrefix(s string) string {
	switch {
	case strings.HasPrefix(s, "enum."):
		return strings.TrimPrefix(s, "enum.")
	case strings.HasPrefix(s, "bitflag."):
		return strings.TrimPrefix(s, "bitflag.")
	case strings.HasPrefix(s, "struct."):
		return strings.TrimPrefix(s, "struct.")
	case strings.HasPrefix(s, "callback."):
		return strings.TrimPrefix(s, "callback.")
	case strings.HasPrefix(s, "object."):
		return strings.TrimPrefix(s, "object.")
	default:
		return ""
	}
}
