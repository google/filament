#!/usr/bin/env python3

# Copyright 2022-2025 The Khronos Group Inc.
# Copyright 2003-2019 Paul McGuire
# SPDX-License-Identifier: MIT

# apirequirements.py - parse 'depends' expressions in API XML
# Supported methods:
#   dependency - the expression string
#
# evaluateDependency(dependency, isSupported) evaluates the expression,
# returning a boolean result. isSupported takes an extension or version name
# string and returns a boolean.
#
# dependencyLanguage(dependency) returns an English string equivalent
# to the expression, suitable for header file comments.
#
# dependencyNames(dependency) returns a set of the extension and
# version names in the expression.
#
# dependencyMarkup(dependency) returns a string containing asciidoctor
# markup for English equivalent to the expression, suitable for extension
# appendices.
#
# All may throw a ParseException if the expression cannot be parsed or is
# not completely consumed by parsing.

# Supported expressions at present:
#   - extension names
#   - '+' as AND connector
#   - ',' as OR connector
#   - parenthesization for grouping

# Based on `examples/fourFn.py` from the
# https://github.com/pyparsing/pyparsing/ repository.

from pyparsing import (
    Literal,
    Word,
    Group,
    Forward,
    alphas,
    alphanums,
    Regex,
    ParseException,
    CaselessKeyword,
    Suppress,
    delimitedList,
    infixNotation,
)
import math
import operator
import pyparsing as pp
import re

from apiconventions import APIConventions as APIConventions
conventions = APIConventions()

def markupPassthrough(name):
    """Pass a name (leaf or operator) through without applying markup"""
    return name

def leafMarkupAsciidoc(name):
    """Markup a leaf name as an asciidoc link to an API version or extension
       anchor.

       - name - version or extension name"""

    return conventions.formatVersionOrExtension(name)

def leafMarkupC(name):
    """Markup a leaf name as a C expression, using conventions of the
       Vulkan Validation Layers

       - name - version or extension name"""

    (apivariant, major, minor) = apiVersionNameMatch(name)

    if apivariant is not None:
        return name
    else:
        return f'ext.{name}'

opMarkupAsciidocMap = { '+' : 'and', ',' : 'or' }

def opMarkupAsciidoc(op):
    """Markup an operator as an asciidoc spec markup equivalent

       - op - operator ('+' or ',')"""

    return opMarkupAsciidocMap[op]

opMarkupCMap = { '+' : '&&', ',' : '||' }

def opMarkupC(op):
    """Markup an operator as a C language equivalent

       - op - operator ('+' or ',')"""

    return opMarkupCMap[op]


# Unfortunately global to be used in pyparsing
exprStack = []

def push_first(toks):
    """Push a token on the global stack

       - toks - first element is the token to push"""

    exprStack.append(toks[0])

# An identifier (version, feature boolean, or extension name)
dependencyIdent = Word(alphanums + '_' + ':')

# Infix expression for depends expressions
dependencyExpr = pp.infixNotation(dependencyIdent,
    [ (pp.oneOf(', +'), 2, pp.opAssoc.LEFT), ])

# BNF grammar for depends expressions
_bnf = None
def dependencyBNF():
    """
    boolop  :: '+' | ','
    extname :: Char(alphas)
    atom    :: extname | '(' expr ')'
    expr    :: atom [ boolop atom ]*
    """
    global _bnf
    if _bnf is None:
        and_, or_ = map(Literal, '+,')
        lpar, rpar = map(Suppress, '()')
        boolop = and_ | or_

        expr = Forward()
        expr_list = delimitedList(Group(expr))
        atom = (
            boolop[...]
            + (
                (dependencyIdent).setParseAction(push_first)
                | Group(lpar + expr + rpar)
            )
        )

        expr <<= atom + (boolop + atom).setParseAction(push_first)[...]
        _bnf = expr
    return _bnf


# map operator symbols to corresponding arithmetic operations
_opn = {
    '+': operator.and_,
    ',': operator.or_,
}

def evaluateStack(stack, isSupported):
    """Evaluate an expression stack, returning a boolean result.

     - stack - the stack
     - isSupported - function taking a version or extension name string and
       returning True or False if that name is supported or not."""

    op, num_args = stack.pop(), 0
    if isinstance(op, tuple):
        op, num_args = op

    if op in '+,':
        # Note: operands are pushed onto the stack in reverse order
        op2 = evaluateStack(stack, isSupported)
        op1 = evaluateStack(stack, isSupported)
        return _opn[op](op1, op2)
    elif op[0].isalpha():
        return isSupported(op)
    else:
        raise Exception(f'invalid op: {op}')

def evaluateDependency(dependency, isSupported):
    """Evaluate a dependency expression, returning a boolean result.

     - dependency - the expression
     - isSupported - function taking a version or extension name string and
       returning True or False if that name is supported or not."""

    global exprStack
    exprStack = []
    results = dependencyBNF().parseString(dependency, parseAll=True)
    val = evaluateStack(exprStack[:], isSupported)
    return val

def evalDependencyLanguage(stack, leafMarkup, opMarkup, parenthesize, root):
    """Evaluate an expression stack, returning an English equivalent

     - stack - the stack
     - leafMarkup, opMarkup, parenthesize - same as dependencyLanguage
     - root - True only if this is the outer (root) expression level"""

    op, num_args = stack.pop(), 0
    if isinstance(op, tuple):
        op, num_args = op
    if op in '+,':
        # Could parenthesize, not needed yet
        rhs = evalDependencyLanguage(stack, leafMarkup, opMarkup, parenthesize, root = False)
        opname = opMarkup(op)
        lhs = evalDependencyLanguage(stack, leafMarkup, opMarkup, parenthesize, root = False)
        if parenthesize and not root:
            return f'({lhs} {opname} {rhs})'
        else:
            return f'{lhs} {opname} {rhs}'
    elif op[0].isalpha():
        # This is an extension or feature name
        return leafMarkup(op)
    else:
        raise Exception(f'invalid op: {op}')

def dependencyLanguage(dependency, leafMarkup, opMarkup, parenthesize):
    """Return an API dependency expression translated to a form suitable for
       asciidoctor conditionals or header file comments.

     - dependency - the expression
     - leafMarkup - function taking an extension / version name and
                    returning an equivalent marked up version
     - opMarkup - function taking an operator ('+' / ',') name name and
                  returning an equivalent marked up version
     - parenthesize - True if parentheses should be used in the resulting
                      expression, False otherwise"""

    global exprStack
    exprStack = []
    results = dependencyBNF().parseString(dependency, parseAll=True)
    return evalDependencyLanguage(exprStack, leafMarkup, opMarkup, parenthesize, root = True)

# aka specmacros = False
def dependencyLanguageComment(dependency):
    """Return dependency expression translated to a form suitable for
       comments in headers of emitted C code, as used by the
       docgenerator."""
    return dependencyLanguage(dependency, leafMarkup = markupPassthrough, opMarkup = opMarkupAsciidoc, parenthesize = True)

# aka specmacros = True
def dependencyLanguageSpecMacros(dependency):
    """Return dependency expression translated to a form suitable for
       comments in headers of emitted C code, as used by the
       interfacegenerator."""
    return dependencyLanguage(dependency, leafMarkup = leafMarkupAsciidoc, opMarkup = opMarkupAsciidoc, parenthesize = False)

def dependencyLanguageC(dependency):
    """Return dependency expression translated to a form suitable for
       use in C expressions"""
    return dependencyLanguage(dependency, leafMarkup = leafMarkupC, opMarkup = opMarkupC, parenthesize = True)

def evalDependencyNames(stack):
    """Evaluate an expression stack, returning the set of extension and
       feature names used in the expression.

     - stack - the stack"""

    op, num_args = stack.pop(), 0
    if isinstance(op, tuple):
        op, num_args = op
    if op in '+,':
        # Do not evaluate the operation. We only care about the names.
        return evalDependencyNames(stack) | evalDependencyNames(stack)
    elif op[0].isalpha():
        return { op }
    else:
        raise Exception(f'invalid op: {op}')

def dependencyNames(dependency):
    """Return a set of the extension and version names in an API dependency
       expression. Used when determining transitive dependencies for spec
       generation with specific extensions included.

     - dependency - the expression"""

    global exprStack
    exprStack = []
    results = dependencyBNF().parseString(dependency, parseAll=True)
    # print(f'names(): stack = {exprStack}')
    return evalDependencyNames(exprStack)

def markupTraverse(expr, level = 0, root = True):
    """Recursively process a dependency in infix form, transforming it into
       asciidoctor markup with expression nesting indicated by indentation
       level.

       - expr - expression to process
       - level - indentation level to render expression at
       - root - True only on initial call"""

    if level > 0:
        prefix = '{nbsp}{nbsp}' * level * 2 + ' '
    else:
        prefix = ''
    str = ''

    for elem in expr:
        if isinstance(elem, pp.ParseResults):
            if not root:
                nextlevel = level + 1
            else:
                # Do not indent the outer expression
                nextlevel = level

            str = str + markupTraverse(elem, level = nextlevel, root = False)
        elif elem in ('+', ','):
            str = str + f'{prefix}{opMarkupAsciidoc(elem)} +\n'
        else:
            str = str + f'{prefix}{leafMarkupAsciidoc(elem)} +\n'

    return str

def dependencyMarkup(dependency):
    """Return asciidoctor markup for a human-readable equivalent of an API
       dependency expression, suitable for use in extension appendix
       metadata.

     - dependency - the expression"""

    parsed = dependencyExpr.parseString(dependency)
    return markupTraverse(parsed)

if __name__ == "__main__":
    for str in [ 'VK_VERSION_1_0', 'cl_khr_extension_name', 'XR_VERSION_3_2', 'CL_VERSION_1_0' ]:
        print(f'{str} -> {conventions.formatVersionOrExtension(str)}')
    import sys
    sys.exit(0)

    termdict = {
        'VK_VERSION_1_1' : True,
        'false' : False,
        'true' : True,
    }
    termSupported = lambda name: name in termdict and termdict[name]

    def test(dependency, expected):
        val = False
        try:
            val = evaluateDependency(dependency, termSupported)
        except ParseException as pe:
            print(dependency, f'failed parse: {dependency}')
        except Exception as e:
            print(dependency, f'failed eval: {dependency}')

        if val == expected:
            True
            # print(f'{dependency} = {val} (as expected)')
        else:
            print(f'{dependency} ERROR: {val} != {expected}')

    # Verify expressions are evaluated left-to-right

    test('false,false+false', False)
    test('false,false+true', False)
    test('false,true+false', False)
    test('false,true+true', True)
    test('true,false+false', False)
    test('true,false+true', True)
    test('true,true+false', False)
    test('true,true+true', True)

    test('false,(false+false)', False)
    test('false,(false+true)', False)
    test('false,(true+false)', False)
    test('false,(true+true)', True)
    test('true,(false+false)', True)
    test('true,(false+true)', True)
    test('true,(true+false)', True)
    test('true,(true+true)', True)


    test('false+false,false', False)
    test('false+false,true', True)
    test('false+true,false', False)
    test('false+true,true', True)
    test('true+false,false', False)
    test('true+false,true', True)
    test('true+true,false', True)
    test('true+true,true', True)

    test('false+(false,false)', False)
    test('false+(false,true)', False)
    test('false+(true,false)', False)
    test('false+(true,true)', False)
    test('true+(false,false)', False)
    test('true+(false,true)', True)
    test('true+(true,false)', True)
    test('true+(true,true)', True)

    # Check formatting
    for dependency in [
        #'true',
        #'true+true+false',
        'true+false',
        'true+(true+false),(false,true)',
        #'true+((true+false),(false,true))',
        'VK_VERSION_1_0+VK_KHR_display',
        #'VK_VERSION_1_1+(true,false)',
    ]:
        print(f'expr = {dependency}\n{dependencyMarkup(dependency)}')
        print(f'  spec language = {dependencyLanguageSpecMacros(dependency)}')
        print(f'  comment language = {dependencyLanguageComment(dependency)}')
        print(f'  C language = {dependencyLanguageC(dependency)}')
        print(f'  names = {dependencyNames(dependency)}')
        print(f'  value = {evaluateDependency(dependency, termSupported)}')
