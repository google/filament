# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import ast
import collections
from io import StringIO
import logging
import sys
import tokenize

import gclient_utils
from third_party import schema

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

# git_dependencies migration states. Used within the DEPS file to indicate
# the current migration state.
DEPS = 'DEPS'
SYNC = 'SYNC'
SUBMODULES = 'SUBMODULES'


class ConstantString(object):
    def __init__(self, value):
        self.value = value

    def __format__(self, format_spec):
        del format_spec
        return self.value

    def __repr__(self):
        return "Str('" + self.value + "')"

    def __eq__(self, other):
        if isinstance(other, ConstantString):
            return self.value == other.value

        return self.value == other

    def __hash__(self):
        return self.value.__hash__()


class _NodeDict(collections.abc.MutableMapping):
    """Dict-like type that also stores information on AST nodes and tokens."""
    def __init__(self, data=None, tokens=None):
        self.data = collections.OrderedDict(data or [])
        self.tokens = tokens

    def __str__(self):
        return str({k: v[0] for k, v in self.data.items()})

    def __repr__(self):
        return self.__str__()

    def __getitem__(self, key):
        return self.data[key][0]

    def __setitem__(self, key, value):
        self.data[key] = (value, None)

    def __delitem__(self, key):
        del self.data[key]

    def __iter__(self):
        return iter(self.data)

    def __len__(self):
        return len(self.data)

    def MoveTokens(self, origin, delta):
        if self.tokens:
            new_tokens = {}
            for pos, token in self.tokens.items():
                if pos[0] >= origin:
                    pos = (pos[0] + delta, pos[1])
                    token = token[:2] + (pos, ) + token[3:]
                new_tokens[pos] = token

        for value, node in self.data.values():
            if node.lineno >= origin:
                node.lineno += delta
                if isinstance(value, _NodeDict):
                    value.MoveTokens(origin, delta)

    def GetNode(self, key):
        return self.data[key][1]

    def SetNode(self, key, value, node):
        self.data[key] = (value, node)


def _NodeDictSchema(dict_schema):
    """Validate dict_schema after converting _NodeDict to a regular dict."""
    def validate(d):
        schema.Schema(dict_schema).validate(dict(d))
        return True

    return validate


# See https://github.com/keleshev/schema for docs how to configure schema.
_GCLIENT_DEPS_SCHEMA = _NodeDictSchema({
    schema.Optional(str):
    schema.Or(
        None,
        str,
        _NodeDictSchema({
            # Repo and revision to check out under the path
            # (same as if no dict was used).
            'url':
            schema.Or(None, str),

            # Optional condition string. The dep will only be processed
            # if the condition evaluates to True.
            schema.Optional('condition'):
            str,
            schema.Optional('dep_type', default='git'):
            str,
        }),
        # CIPD package.
        _NodeDictSchema({
            'packages': [_NodeDictSchema({
                'package': str,
                'version': str,
            })],
            schema.Optional('condition'):
            str,
            schema.Optional('dep_type', default='cipd'):
            str,
        }),
        # GCS content.
        _NodeDictSchema({
            'bucket':
            str,
            'objects': [
                _NodeDictSchema({
                    'object_name':
                    str,
                    'sha256sum':
                    str,
                    'size_bytes':
                    int,
                    'generation':
                    int,
                    schema.Optional('output_file'):
                    str,
                    # The object will only be processed if the condition
                    # evaluates to True. This is AND with the parent condition.
                    schema.Optional('condition'):
                    str,
                })
            ],
            schema.Optional('condition'):
            str,
            schema.Optional('dep_type', default='gcs'):
            str,
        }),
    ),
})

_GCLIENT_HOOKS_SCHEMA = [
    _NodeDictSchema({
        # Hook action: list of command-line arguments to invoke.
        'action': [schema.Or(str)],

        # Name of the hook. Doesn't affect operation.
        schema.Optional('name'):
        str,

        # Hook pattern (regex). Originally intended to limit some hooks to run
        # only when files matching the pattern have changed. In practice, with
        # git, gclient runs all the hooks regardless of this field.
        schema.Optional('pattern'):
        str,

        # Working directory where to execute the hook.
        schema.Optional('cwd'):
        str,

        # Optional condition string. The hook will only be run
        # if the condition evaluates to True.
        schema.Optional('condition'):
        str,
    })
]

_GCLIENT_SCHEMA = schema.Schema(
    _NodeDictSchema({
        # Current state of the git submodule migration.
        # git_dependencies = [DEPS (default) | SUBMODULES | SYNC]
        schema.Optional('git_dependencies'):
        schema.Or(DEPS, SYNC, SUBMODULES),

        # List of host names from which dependencies are allowed (allowlist).
        # NOTE: when not present, all hosts are allowed.
        # NOTE: scoped to current DEPS file, not recursive.
        schema.Optional('allowed_hosts'): [schema.Optional(str)],

        # Mapping from paths to repo and revision to check out under that path.
        # Applying this mapping to the on-disk checkout is the main purpose
        # of gclient, and also why the config file is called DEPS.
        #
        # The following functions are allowed:
        #
        #   Var(): allows variable substitution (either from 'vars' dict below,
        #          or command-line override)
        schema.Optional('deps'):
        _GCLIENT_DEPS_SCHEMA,

        # Similar to 'deps' (see above) - also keyed by OS (e.g. 'linux').
        # Also see 'target_os'.
        schema.Optional('deps_os'):
        _NodeDictSchema({
            schema.Optional(str): _GCLIENT_DEPS_SCHEMA,
        }),

        # Dependency to get gclient_gn_args* settings from. This allows these
        # values to be set in a recursedeps file, rather than requiring that
        # they exist in the top-level solution.
        schema.Optional('gclient_gn_args_from'):
        str,

        # Path to GN args file to write selected variables.
        schema.Optional('gclient_gn_args_file'):
        str,

        # Subset of variables to write to the GN args file (see above).
        schema.Optional('gclient_gn_args'): [schema.Optional(str)],

        # Hooks executed after gclient sync (unless suppressed), or explicitly
        # on gclient hooks. See _GCLIENT_HOOKS_SCHEMA for details.
        # Also see 'pre_deps_hooks'.
        schema.Optional('hooks'):
        _GCLIENT_HOOKS_SCHEMA,

        # Similar to 'hooks', also keyed by OS.
        schema.Optional('hooks_os'):
        _NodeDictSchema({schema.Optional(str): _GCLIENT_HOOKS_SCHEMA}),

        # Rules which #includes are allowed in the directory.
        # Also see 'skip_child_includes' and 'specific_include_rules'.
        schema.Optional('include_rules'): [schema.Optional(str)],

        # Commits that add include_rules entries on the paths with this set
        # will require an OWNERS review from them.
        schema.Optional('new_usages_require_review'):
        bool,

        # Optionally discards rules from parent directories, similar to
        # "noparent" in OWNERS files. For example, if
        # //base/allocator/partition_allocator has "noparent = True" then it
        # will not inherit rules from //base/DEPS and //base/allocator/DEPS,
        # forcing each //base/allocator/partition_allocator/{foo,bar,...} to
        # declare all its dependencies.
        schema.Optional('noparent'):
        bool,

        # Hooks executed before processing DEPS. See 'hooks' for more details.
        schema.Optional('pre_deps_hooks'):
        _GCLIENT_HOOKS_SCHEMA,

        # Recursion limit for nested DEPS.
        schema.Optional('recursion'):
        int,

        # Allowlists deps for which recursion should be enabled.
        schema.Optional('recursedeps'): [
            schema.Optional(schema.Or(str, (str, str), [str, str])),
        ],

        # Blocklists directories for checking 'include_rules'.
        schema.Optional('skip_child_includes'): [schema.Optional(str)],

        # Mapping from paths to include rules specific for that path.
        # See 'include_rules' for more details.
        schema.Optional('specific_include_rules'):
        _NodeDictSchema({schema.Optional(str): [str]}),

        # List of additional OS names to consider when selecting dependencies
        # from deps_os.
        schema.Optional('target_os'): [schema.Optional(str)],

        # For recursed-upon sub-dependencies, check out their own dependencies
        # relative to the parent's path, rather than relative to the .gclient
        # file.
        schema.Optional('use_relative_paths'):
        bool,

        # For recursed-upon sub-dependencies, run their hooks relative to the
        # parent's path instead of relative to the .gclient file.
        schema.Optional('use_relative_hooks'):
        bool,

        # Variables that can be referenced using Var() - see 'deps'.
        schema.Optional('vars'):
        _NodeDictSchema({
            schema.Optional(str):
            schema.Or(ConstantString, str, bool),
        }),
    }))


def _gclient_eval(node_or_string, filename='<unknown>', vars_dict=None):
    """Safely evaluates a single expression. Returns the result."""
    _allowed_names = {'None': None, 'True': True, 'False': False}
    if isinstance(node_or_string, ConstantString):
        return node_or_string.value
    if isinstance(node_or_string, str):
        node_or_string = ast.parse(node_or_string,
                                   filename=filename,
                                   mode='eval')
    if isinstance(node_or_string, ast.Expression):
        node_or_string = node_or_string.body

    def _convert(node):
        if isinstance(node, ast.Str):
            if vars_dict is None:
                return node.s
            try:
                return node.s.format(**vars_dict)
            except KeyError as e:
                raise KeyError(
                    '%s was used as a variable, but was not declared in the vars dict '
                    '(file %r, line %s)' %
                    (e.args[0], filename, getattr(node, 'lineno', '<unknown>')))
        elif isinstance(node, ast.Num):
            return node.n
        elif isinstance(node, ast.Tuple):
            return tuple(map(_convert, node.elts))
        elif isinstance(node, ast.List):
            return list(map(_convert, node.elts))
        elif isinstance(node, ast.Dict):
            node_dict = _NodeDict()
            for key_node, value_node in zip(node.keys, node.values):
                key = _convert(key_node)
                if key in node_dict:
                    raise ValueError(
                        'duplicate key in dictionary: %s (file %r, line %s)' %
                        (key, filename, getattr(key_node, 'lineno',
                                                '<unknown>')))
                node_dict.SetNode(key, _convert(value_node), value_node)
            return node_dict
        elif isinstance(node, ast.Name):
            if node.id not in _allowed_names:
                raise ValueError(
                    'invalid name %r (file %r, line %s)' %
                    (node.id, filename, getattr(node, 'lineno', '<unknown>')))
            return _allowed_names[node.id]
        elif not sys.version_info[:2] < (3, 4) and isinstance(
                node, ast.NameConstant):  # Since Python 3.4
            return node.value
        elif isinstance(node, ast.Call):
            if (not isinstance(node.func, ast.Name)
                    or (node.func.id not in ('Str', 'Var'))):
                raise ValueError(
                    'Str and Var are the only allowed functions (file %r, line %s)'
                    % (filename, getattr(node, 'lineno', '<unknown>')))
            if node.keywords or getattr(node, 'starargs', None) or getattr(
                    node, 'kwargs', None) or len(node.args) != 1:
                raise ValueError(
                    '%s takes exactly one argument (file %r, line %s)' %
                    (node.func.id, filename, getattr(node, 'lineno',
                                                     '<unknown>')))

            if node.func.id == 'Str':
                if isinstance(node.args[0], ast.Str):
                    return ConstantString(node.args[0].s)
                raise ValueError(
                    'Passed a non-string to Str() (file %r, line%s)' %
                    (filename, getattr(node, 'lineno', '<unknown>')))

            arg = _convert(node.args[0])
            if not isinstance(arg, str):
                raise ValueError(
                    'Var\'s argument must be a variable name (file %r, line %s)'
                    % (filename, getattr(node, 'lineno', '<unknown>')))
            if vars_dict is None:
                return '{' + arg + '}'
            if arg not in vars_dict:
                raise KeyError(
                    '%s was used as a variable, but was not declared in the vars dict '
                    '(file %r, line %s)' %
                    (arg, filename, getattr(node, 'lineno', '<unknown>')))
            val = vars_dict[arg]
            if isinstance(val, ConstantString):
                val = val.value
            return val
        elif isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add):
            return _convert(node.left) + _convert(node.right)
        elif isinstance(node, ast.BinOp) and isinstance(node.op, ast.Mod):
            return _convert(node.left) % _convert(node.right)
        else:
            raise ValueError('unexpected AST node: %s %s (file %r, line %s)' %
                             (node, ast.dump(node), filename,
                              getattr(node, 'lineno', '<unknown>')))

    return _convert(node_or_string)


def Exec(content, filename='<unknown>', vars_override=None, builtin_vars=None):
    """Safely execs a set of assignments."""
    def _validate_statement(node, local_scope):
        if not isinstance(node, ast.Assign):
            raise ValueError('unexpected AST node: %s %s (file %r, line %s)' %
                             (node, ast.dump(node), filename,
                              getattr(node, 'lineno', '<unknown>')))

        if len(node.targets) != 1:
            raise ValueError(
                'invalid assignment: use exactly one target (file %r, line %s)'
                % (filename, getattr(node, 'lineno', '<unknown>')))

        target = node.targets[0]
        if not isinstance(target, ast.Name):
            raise ValueError(
                'invalid assignment: target should be a name (file %r, line %s)'
                % (filename, getattr(node, 'lineno', '<unknown>')))
        if target.id in local_scope:
            raise ValueError(
                'invalid assignment: overrides var %r (file %r, line %s)' %
                (target.id, filename, getattr(node, 'lineno', '<unknown>')))

    node_or_string = ast.parse(content, filename=filename, mode='exec')
    if isinstance(node_or_string, ast.Expression):
        node_or_string = node_or_string.body

    if not isinstance(node_or_string, ast.Module):
        raise ValueError('unexpected AST node: %s %s (file %r, line %s)' %
                         (node_or_string, ast.dump(node_or_string), filename,
                          getattr(node_or_string, 'lineno', '<unknown>')))

    statements = {}
    for statement in node_or_string.body:
        _validate_statement(statement, statements)
        statements[statement.targets[0].id] = statement.value

    tokens = {
        token[2]: list(token)
        for token in tokenize.generate_tokens(StringIO(content).readline)
    }

    local_scope = _NodeDict({}, tokens)

    # Process vars first, so we can expand variables in the rest of the DEPS
    # file.
    vars_dict = {}
    if 'vars' in statements:
        vars_statement = statements['vars']
        value = _gclient_eval(vars_statement, filename)
        local_scope.SetNode('vars', value, vars_statement)
        # Update the parsed vars with the overrides, but only if they are
        # already present (overrides do not introduce new variables).
        vars_dict.update(value)

    if builtin_vars:
        vars_dict.update(builtin_vars)

    if vars_override:
        vars_dict.update(
            {k: v
             for k, v in vars_override.items() if k in vars_dict})

    for name, node in statements.items():
        value = _gclient_eval(node, filename, vars_dict)
        local_scope.SetNode(name, value, node)

    try:
        return _GCLIENT_SCHEMA.validate(local_scope)
    except schema.SchemaError as e:
        raise gclient_utils.Error(str(e))


def _StandardizeDeps(deps_dict, vars_dict):
    """"Standardizes the deps_dict.

    For each dependency:
    - Expands the variable in the dependency name.
    - Ensures the dependency is a dictionary.
    - Set's the 'dep_type' to be 'git' by default.
    """
    new_deps_dict = {}
    for dep_name, dep_info in deps_dict.items():
        dep_name = dep_name.format(**vars_dict)
        if not isinstance(dep_info, collections.abc.Mapping):
            dep_info = {'url': dep_info}
        dep_info.setdefault('dep_type', 'git')
        new_deps_dict[dep_name] = dep_info
    return new_deps_dict


def _MergeDepsOs(deps_dict, os_deps_dict, os_name):
    """Merges the deps in os_deps_dict into conditional dependencies in deps_dict.

    The dependencies in os_deps_dict are transformed into conditional dependencies
    using |'checkout_' + os_name|.
    If the dependency is already present, the URL and revision must coincide.
    """
    for dep_name, dep_info in os_deps_dict.items():
        # Make this condition very visible, so it's not a silent failure.
        # It's unclear how to support None override in deps_os.
        if dep_info['url'] is None:
            logging.error('Ignoring %r:%r in %r deps_os', dep_name, dep_info,
                          os_name)
            continue

        os_condition = 'checkout_' + (os_name if os_name != 'unix' else 'linux')
        UpdateCondition(dep_info, 'and', os_condition)

        if dep_name in deps_dict:
            if deps_dict[dep_name]['url'] != dep_info['url']:
                raise gclient_utils.Error(
                    'Value from deps_os (%r; %r: %r) conflicts with existing deps '
                    'entry (%r).' %
                    (os_name, dep_name, dep_info, deps_dict[dep_name]))

            UpdateCondition(dep_info, 'or',
                            deps_dict[dep_name].get('condition'))

        deps_dict[dep_name] = dep_info


def UpdateCondition(info_dict, op, new_condition):
    """Updates info_dict's condition with |new_condition|.

    An absent value is treated as implicitly True.
    """
    curr_condition = info_dict.get('condition')
    # Easy case: Both are present.
    if curr_condition and new_condition:
        info_dict['condition'] = '(%s) %s (%s)' % (curr_condition, op,
                                                   new_condition)
    # If |op| == 'and', and at least one condition is present, then use it.
    elif op == 'and' and (curr_condition or new_condition):
        info_dict['condition'] = curr_condition or new_condition
    # Otherwise, no condition should be set
    elif curr_condition:
        del info_dict['condition']


def Parse(content, filename, vars_override=None, builtin_vars=None):
    """Parses DEPS strings.

    Executes the Python-like string stored in content, resulting in a Python
    dictionary specified by the schema above. Supports syntax validation and
    variable expansion.

    Args:
        content: str. DEPS file stored as a string.
        filename: str. The name of the DEPS file, or a string describing the source
            of the content, e.g. '<string>', '<unknown>'.
        vars_override: dict, optional. A dictionary with overrides for the variables
            defined by the DEPS file.
        builtin_vars: dict, optional. A dictionary with variables that are provided
            by default.

    Returns:
        A Python dict with the parsed contents of the DEPS file, as specified by the
        schema above.
    """
    result = Exec(content, filename, vars_override, builtin_vars)

    vars_dict = result.get('vars', {})
    if 'deps' in result:
        result['deps'] = _StandardizeDeps(result['deps'], vars_dict)

    if 'deps_os' in result:
        deps = result.setdefault('deps', {})
        for os_name, os_deps in result['deps_os'].items():
            os_deps = _StandardizeDeps(os_deps, vars_dict)
            _MergeDepsOs(deps, os_deps, os_name)
        del result['deps_os']

    if 'hooks_os' in result:
        hooks = result.setdefault('hooks', [])
        for os_name, os_hooks in result['hooks_os'].items():
            for hook in os_hooks:
                UpdateCondition(hook, 'and', 'checkout_' + os_name)
            hooks.extend(os_hooks)
        del result['hooks_os']

    return result


def EvaluateCondition(condition, variables, referenced_variables=None):
    """Safely evaluates a boolean condition. Returns the result."""
    if not referenced_variables:
        referenced_variables = set()
    _allowed_names = {'None': None, 'True': True, 'False': False}
    main_node = ast.parse(condition, mode='eval')
    if isinstance(main_node, ast.Expression):
        main_node = main_node.body

    def _convert(node, allow_tuple=False):
        if isinstance(node, ast.Str):
            return node.s

        if isinstance(node, ast.Tuple) and allow_tuple:
            return tuple(map(_convert, node.elts))

        if isinstance(node, ast.Name):
            if node.id in referenced_variables:
                raise ValueError('invalid cyclic reference to %r (inside %r)' %
                                 (node.id, condition))

            if node.id in _allowed_names:
                return _allowed_names[node.id]

            if node.id in variables:
                value = variables[node.id]

                # Allow using "native" types, without wrapping everything in
                # strings. Note that schema constraints still apply to
                # variables.
                if not isinstance(value, str):
                    return value

                # Recursively evaluate the variable reference.
                return EvaluateCondition(variables[node.id], variables,
                                         referenced_variables.union([node.id]))

            # Implicitly convert unrecognized names to strings.
            # If we want to change this, we'll need to explicitly distinguish
            # between arguments for GN to be passed verbatim, and ones to
            # be evaluated.
            return node.id

        if not sys.version_info[:2] < (3, 4) and isinstance(
                node, ast.NameConstant):  # Since Python 3.4
            return node.value

        if isinstance(node, ast.BoolOp) and isinstance(node.op, ast.Or):
            bool_values = []
            for value in node.values:
                bool_values.append(_convert(value))
                if not isinstance(bool_values[-1], bool):
                    raise ValueError('invalid "or" operand %r (inside %r)' %
                                     (bool_values[-1], condition))
            return any(bool_values)

        if isinstance(node, ast.BoolOp) and isinstance(node.op, ast.And):
            bool_values = []
            for value in node.values:
                bool_values.append(_convert(value))
                if not isinstance(bool_values[-1], bool):
                    raise ValueError('invalid "and" operand %r (inside %r)' %
                                     (bool_values[-1], condition))
            return all(bool_values)

        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.Not):
            value = _convert(node.operand)
            if not isinstance(value, bool):
                raise ValueError('invalid "not" operand %r (inside %r)' %
                                 (value, condition))
            return not value

        if isinstance(node, ast.Compare):
            if len(node.ops) != 1:
                raise ValueError(
                    'invalid compare: exactly 1 operator required (inside %r)' %
                    (condition))
            if len(node.comparators) != 1:
                raise ValueError(
                    'invalid compare: exactly 1 comparator required (inside %r)'
                    % (condition))

            left = _convert(node.left)
            right = _convert(node.comparators[0],
                             allow_tuple=isinstance(node.ops[0], ast.In))

            if isinstance(node.ops[0], ast.Eq):
                return left == right
            if isinstance(node.ops[0], ast.NotEq):
                return left != right
            if isinstance(node.ops[0], ast.In):
                return left in right

            raise ValueError('unexpected operator: %s %s (inside %r)' %
                             (node.ops[0], ast.dump(node), condition))

        raise ValueError('unexpected AST node: %s %s (inside %r)' %
                         (node, ast.dump(node), condition))

    return _convert(main_node)


def RenderDEPSFile(gclient_dict):
    contents = sorted(gclient_dict.tokens.values(), key=lambda token: token[2])
    return tokenize.untokenize(contents)


def _UpdateAstString(tokens, node, value):
    if isinstance(node, ast.Call):
        node = node.args[0]
    position = node.lineno, node.col_offset
    quote_char = ''
    if isinstance(node, ast.Str):
        quote_char = tokens[position][1][0]
        value = value.encode('unicode_escape').decode('utf-8')
    tokens[position][1] = quote_char + value + quote_char
    node.s = value


def _ShiftLinesInTokens(tokens, delta, start):
    new_tokens = {}
    for token in tokens.values():
        if token[2][0] >= start:
            token[2] = token[2][0] + delta, token[2][1]
            token[3] = token[3][0] + delta, token[3][1]
        new_tokens[token[2]] = token
    return new_tokens


def AddVar(gclient_dict, var_name, value):
    if not isinstance(gclient_dict, _NodeDict) or gclient_dict.tokens is None:
        raise ValueError(
            "Can't use SetVar for the given gclient dict. It contains no "
            "formatting information.")

    if 'vars' not in gclient_dict:
        raise KeyError("vars dict is not defined.")

    if var_name in gclient_dict['vars']:
        raise ValueError(
            "%s has already been declared in the vars dict. Consider using SetVar "
            "instead." % var_name)

    if not gclient_dict['vars']:
        raise ValueError('vars dict is empty. This is not yet supported.')

    # We will attempt to add the var right after 'vars = {'.
    node = gclient_dict.GetNode('vars')
    if node is None:
        raise ValueError("The vars dict has no formatting information." %
                         var_name)
    line = node.lineno + 1

    # We will try to match the new var's indentation to the next variable.
    col = node.keys[0].col_offset

    # We use a minimal Python dictionary, so that ast can parse it.
    var_content = '{\n%s"%s": "%s",\n}\n' % (' ' * col, var_name, value)
    var_ast = ast.parse(var_content).body[0].value

    # Set the ast nodes for the key and value.
    vars_node = gclient_dict.GetNode('vars')

    var_name_node = var_ast.keys[0]
    var_name_node.lineno += line - 2
    vars_node.keys.insert(0, var_name_node)

    value_node = var_ast.values[0]
    value_node.lineno += line - 2
    vars_node.values.insert(0, value_node)

    # Update the tokens.
    var_tokens = list(tokenize.generate_tokens(StringIO(var_content).readline))
    var_tokens = {
        token[2]: list(token)
        # Ignore the tokens corresponding to braces and new lines.
        for token in var_tokens[2:-3]
    }

    gclient_dict.tokens = _ShiftLinesInTokens(gclient_dict.tokens, 1, line)
    gclient_dict.tokens.update(_ShiftLinesInTokens(var_tokens, line - 2, 0))


def SetVar(gclient_dict, var_name, value):
    if not isinstance(gclient_dict, _NodeDict) or gclient_dict.tokens is None:
        raise ValueError(
            "Can't use SetVar for the given gclient dict. It contains no "
            "formatting information.")
    tokens = gclient_dict.tokens

    if 'vars' not in gclient_dict:
        raise KeyError("vars dict is not defined.")

    if var_name not in gclient_dict['vars']:
        raise ValueError(
            "%s has not been declared in the vars dict. Consider using AddVar "
            "instead." % var_name)

    node = gclient_dict['vars'].GetNode(var_name)
    if node is None:
        raise ValueError(
            "The vars entry for %s has no formatting information." % var_name)

    _UpdateAstString(tokens, node, value)
    gclient_dict['vars'].SetNode(var_name, value, node)


def _GetVarName(node):
    if isinstance(node, ast.Call):
        return node.args[0].s

    if node.s.endswith('}'):
        last_brace = node.s.rfind('{')
        return node.s[last_brace + 1:-1]
    return None


def SetGCS(gclient_dict, dep_name, new_objects):
    if not isinstance(gclient_dict, _NodeDict) or gclient_dict.tokens is None:
        raise ValueError(
            "Can't use SetGCS for the given gclient dict. It contains no "
            "formatting information.")
    tokens = gclient_dict.tokens

    if 'deps' not in gclient_dict or dep_name not in gclient_dict['deps']:
        raise KeyError("Could not find any dependency called %s." % dep_name)

    node = gclient_dict['deps'][dep_name]
    objects_node = node.GetNode('objects')
    if len(objects_node.elts) != len(new_objects):
        raise ValueError("Number of revision objects must match the current "
                         "number of objects.")

    # Allow only `keys_to_update` to be updated.
    keys_to_update = ('object_name', 'sha256sum', 'size_bytes', 'generation')
    for index, object_node in enumerate(objects_node.elts):
        for key, value in zip(object_node.keys, object_node.values):
            if key.s not in keys_to_update:
                continue
            _UpdateAstString(tokens, value, new_objects[index][key.s])

    node.SetNode('objects', new_objects, objects_node)


def SetCIPD(gclient_dict, dep_name, package_name, new_version):
    if not isinstance(gclient_dict, _NodeDict) or gclient_dict.tokens is None:
        raise ValueError(
            "Can't use SetCIPD for the given gclient dict. It contains no "
            "formatting information.")
    tokens = gclient_dict.tokens

    if 'deps' not in gclient_dict or dep_name not in gclient_dict['deps']:
        raise KeyError("Could not find any dependency called %s." % dep_name)

    # Find the package with the given name
    packages = [
        package for package in gclient_dict['deps'][dep_name]['packages']
        if package['package'] == package_name
    ]
    if len(packages) != 1:
        raise ValueError(
            "There must be exactly one package with the given name (%s), "
            "%s were found." % (package_name, len(packages)))

    # TODO(ehmaldonado): Support Var in package's version.
    node = packages[0].GetNode('version')
    if node is None:
        raise ValueError(
            "The deps entry for %s:%s has no formatting information." %
            (dep_name, package_name))

    if not isinstance(node, ast.Call) and not isinstance(node, ast.Str):
        raise ValueError(
            "Unsupported dependency revision format. Please file a bug to the "
            "Infra>SDK component in crbug.com")

    var_name = _GetVarName(node)
    if var_name is not None:
        SetVar(gclient_dict, var_name, new_version)
    else:
        _UpdateAstString(tokens, node, new_version)
        packages[0].SetNode('version', new_version, node)


def SetRevision(gclient_dict, dep_name, new_revision):
    def _UpdateRevision(dep_dict, dep_key, new_revision):
        dep_node = dep_dict.GetNode(dep_key)
        if dep_node is None:
            raise ValueError(
                "The deps entry for %s has no formatting information." %
                dep_name)

        node = dep_node
        if isinstance(node, ast.BinOp):
            node = node.right

        if isinstance(node, ast.Str):
            token = _gclient_eval(tokens[node.lineno, node.col_offset][1])
            if token != node.s:
                raise ValueError(
                    'Can\'t update value for %s. Multiline strings and implicitly '
                    'concatenated strings are not supported.\n'
                    'Consider reformatting the DEPS file.' % dep_key)

        if not isinstance(node, ast.Call) and not isinstance(node, ast.Str):
            raise ValueError(
                "Unsupported dependency revision format. Please file a bug to the "
                "Infra>SDK component in crbug.com")

        var_name = _GetVarName(node)
        if var_name is not None:
            SetVar(gclient_dict, var_name, new_revision)
        else:
            if '@' in node.s:
                # '@' is part of the last string, which we want to modify.
                # Discard whatever was after the '@' and put the new revision in
                # its place.
                new_revision = node.s.split('@')[0] + '@' + new_revision
            elif '@' not in dep_dict[dep_key]:
                # '@' is not part of the URL at all. This mean the dependency is
                # unpinned and we should pin it.
                new_revision = node.s + '@' + new_revision
            _UpdateAstString(tokens, node, new_revision)
            dep_dict.SetNode(dep_key, new_revision, node)

    if not isinstance(gclient_dict, _NodeDict) or gclient_dict.tokens is None:
        raise ValueError(
            "Can't use SetRevision for the given gclient dict. It contains no "
            "formatting information.")
    tokens = gclient_dict.tokens

    if 'deps' not in gclient_dict or dep_name not in gclient_dict['deps']:
        raise KeyError("Could not find any dependency called %s." % dep_name)

    if isinstance(gclient_dict['deps'][dep_name], _NodeDict):
        _UpdateRevision(gclient_dict['deps'][dep_name], 'url', new_revision)
    else:
        _UpdateRevision(gclient_dict['deps'], dep_name, new_revision)


def GetVar(gclient_dict, var_name):
    if 'vars' not in gclient_dict or var_name not in gclient_dict['vars']:
        raise KeyError("Could not find any variable called %s." % var_name)

    val = gclient_dict['vars'][var_name]
    if isinstance(val, ConstantString):
        return val.value
    return val


def GetCIPD(gclient_dict, dep_name, package_name):
    if 'deps' not in gclient_dict or dep_name not in gclient_dict['deps']:
        raise KeyError("Could not find any dependency called %s." % dep_name)

    # Find the package with the given name
    packages = [
        package for package in gclient_dict['deps'][dep_name]['packages']
        if package['package'] == package_name
    ]
    if len(packages) != 1:
        raise ValueError(
            "There must be exactly one package with the given name (%s), "
            "%s were found." % (package_name, len(packages)))

    return packages[0]['version']


def GetRevision(gclient_dict, dep_name):
    if 'deps' not in gclient_dict or dep_name not in gclient_dict['deps']:
        suggestions = []
        if 'deps' in gclient_dict:
            for key in gclient_dict['deps']:
                if dep_name in key:
                    suggestions.append(key)
        if suggestions:
            raise KeyError(
                "Could not find any dependency called %s. Did you mean %s" %
                (dep_name, ' or '.join(suggestions)))
        raise KeyError("Could not find any dependency called %s." % dep_name)

    dep = gclient_dict['deps'][dep_name]
    if dep is None:
        return None

    if isinstance(dep, str):
        _, _, revision = dep.partition('@')
        return revision or None

    if isinstance(dep, collections.abc.Mapping) and 'url' in dep:
        _, _, revision = dep['url'].partition('@')
        return revision or None

    if isinstance(gclient_dict, _NodeDict) and 'objects' in dep:
        return dep['objects']

    raise ValueError('%s is not a valid git or gcs dependency.' % dep_name)
