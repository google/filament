"""
Find modules used by a script, using bytecode analysis.

Based on the stdlib modulefinder by Thomas Heller and Just van Rossum,
but uses a graph data structure and 2.3 features
"""
from __future__ import absolute_import, print_function

import ast
import codecs
import dis
import imp
import marshal
import os
import re
import struct
import sys
import warnings
import zipimport
from collections import deque, namedtuple

import pkg_resources
from altgraph import GraphError
from altgraph.ObjectGraph import ObjectGraph

from . import util, zipio

if sys.version_info[0] == 2:
    from urllib import pathname2url

    from StringIO import StringIO
    from StringIO import StringIO as BytesIO

    def _Bchr(value):
        return chr(value)


else:
    from io import BytesIO, StringIO
    from urllib.request import pathname2url

    def _Bchr(value):
        return value


# File open mode for reading (univeral newlines)
if sys.version_info[0] == 2:
    _READ_MODE = "rU"
else:
    _READ_MODE = "r"


BOM = codecs.BOM_UTF8.decode("utf-8")


# Modulegraph does a good job at simulating Python's, but it can not
# handle packagepath modifications packages make at runtime.  Therefore there
# is a mechanism whereby you can register extra paths in this map for a
# package, and it will be honored.
#
# Note this is a mapping is lists of paths.
_packagePathMap = {}

# Prefix used in magic .pth files used by setuptools to create namespace
# packages without an __init__.py file.
#
# The value is a list of such prefixes as the prefix varies with versions of
# setuptools.
_SETUPTOOLS_NAMESPACEPKG_PTHs = (
    # setuptools 31.0.0
    (
        "import sys, types, os;has_mfs = sys.version_info > (3, 5);"
        "p = os.path.join(sys._getframe(1).f_locals['sitedir'], *('"
    ),
    # distribute 0.6.10
    (
        "import sys,types,os; p = os.path.join("
        "sys._getframe(1).f_locals['sitedir'], *('"
    ),
    # setuptools 0.6c9, distribute 0.6.12
    (
        "import sys,new,os; p = os.path.join(sys._getframe("
        "1).f_locals['sitedir'], *('"
    ),
    # setuptools 28.1.0
    (
        "import sys, types, os;p = os.path.join("
        "sys._getframe(1).f_locals['sitedir'], *('"
    ),
    # setuptools 28.7.0
    (
        "import sys, types, os;pep420 = sys.version_info > (3, 3);"
        "p = os.path.join(sys._getframe(1).f_locals['sitedir'], *('"
    ),
)


class InvalidRelativeImportError(ImportError):
    pass


def _namespace_package_path(fqname, pathnames, path=None):
    """
    Return the __path__ for the python package in *fqname*.

    This function uses setuptools metadata to extract information
    about namespace packages from installed eggs.
    """
    working_set = pkg_resources.WorkingSet(path)

    path = list(pathnames)

    for dist in working_set:
        if dist.has_metadata("namespace_packages.txt"):
            namespaces = dist.get_metadata("namespace_packages.txt").splitlines()
            if fqname in namespaces:
                nspath = os.path.join(dist.location, *fqname.split("."))
                if nspath not in path:
                    path.append(nspath)

    return path


_strs = re.compile(r"""^\s*["']([A-Za-z0-9_]+)["'],?\s*""")  # "<- emacs happy


def _eval_str_tuple(value):
    """
    Input is the repr of a tuple of strings, output
    is that tuple.

    This only works with a tuple where the members are
    python identifiers.
    """
    if not (value.startswith("(") and value.endswith(")")):
        raise ValueError(value)

    orig_value = value
    value = value[1:-1]

    result = []
    while value:
        m = _strs.match(value)
        if m is None:
            raise ValueError(orig_value)

        result.append(m.group(1))
        value = value[len(m.group(0)) :]  # noqa: E203

    return tuple(result)


def _path_from_importerror(exc, default):
    # This is a hack, but sadly enough the necessary information
    # isn't available otherwise.
    m = re.match(r"^No module named (\S+)$", str(exc))
    if m is not None:
        return m.group(1)

    return default


def os_listdir(path):
    """
    Deprecated name
    """
    warnings.warn("Use zipio.listdir instead of os_listdir", DeprecationWarning)
    return zipio.listdir(path)


def _code_to_file(co):
    """Convert code object to a .pyc pseudo-file"""
    return BytesIO(imp.get_magic() + b"\0\0\0\0" + marshal.dumps(co))


def find_module(name, path=None):
    """
    Get a 3-tuple detailing the physical location of the Python module with
    the passed name if that module is found *or* raise `ImportError` otherwise.

    This low-level function is a variant on the standard `imp.find_module()`
    function with additional support for:

    * Multiple search paths. The passed list of absolute paths will be
      iteratively searched for the first directory containing a file
      corresponding to this module.
    * Compressed (e.g., zipped) packages.

    For efficiency, the high-level `ModuleGraph.find_module()` method wraps
    this function with graph-based module caching.

    Parameters
    ----------
    name : str
        Fully-qualified name of the Python module to be found.
    path : list
        List of the absolute paths of all directories to search for this module
        *or* `None` if the default path list `sys.path` is to be searched.

    Returns
    ----------
    (file_handle, filename, metadata)
        3-tuple detailing the physical location of this module, where:
        * `file_handle` is an open read-only file handle from which the
            contents of this module may be read.
        * `filename` is the absolute path of this file.
        * `metadata` is itself a 3-tuple `(file_suffix, mode, imp_type)`.  See
          `load_module()` for details.
    """
    if path is None:
        path = sys.path

    # Support for the PEP302 importer for normal imports:
    # - Python 2.5 has pkgutil.ImpImporter
    # - In setuptools 0.7 and later there's _pkgutil.ImpImporter
    # - In earlier setuptools versions you pkg_resources.ImpWrapper
    #
    # This is a bit of a hack, should check if we can just rely on
    # PEP302's get_code() method with all recent versions of pkgutil and/or
    # setuptools (setuptools 0.6.latest, setuptools trunk and python2.[45])
    #
    # For python 3.4 this code should be replaced by code calling
    # importlib.util.find_spec().
    # For python 3.3 this code should be replaced by code using importlib,
    # for python 3.2 and 2.7 this should be cleaned up a lot.
    try:
        from pkgutil import ImpImporter
    except ImportError:
        try:
            from _pkgutil import ImpImporter
        except ImportError:
            ImpImporter = pkg_resources.ImpWrapper

    try:
        from importlib.machinery import ExtensionFileLoader
    except ImportError:
        ExtensionFileLoader = None

    namespace_path = []
    fp = None
    for entry in path:
        importer = pkg_resources.get_importer(entry)
        if importer is None:
            continue

        if sys.version_info[:2] >= (3, 3) and hasattr(importer, "find_loader"):
            loader, portions = importer.find_loader(name)

        else:
            loader = importer.find_module(name)
            portions = []

        namespace_path.extend(portions)

        if loader is None:
            continue

        if ExtensionFileLoader is not None and isinstance(loader, ExtensionFileLoader):
            filename = loader.path

            for _sfx, _mode, _type in imp.get_suffixes():
                if _type == imp.C_EXTENSION and filename.endswith(_sfx):
                    description = (_sfx, "rb", imp.C_EXTENSION)
                    break
            else:
                raise RuntimeError("Don't know how to handle %r" % (importer,))

            return (None, filename, description)

        elif isinstance(importer, ImpImporter):
            filename = loader.filename
            if filename.endswith(".pyc") or filename.endswith(".pyo"):
                fp = open(filename, "rb")
                description = (".pyc", "rb", imp.PY_COMPILED)
                return (fp, filename, description)

            elif filename.endswith(".py"):
                if sys.version_info[0] == 2:
                    fp = open(filename, _READ_MODE)
                else:
                    with open(filename, "rb") as fp:
                        encoding = util.guess_encoding(fp)

                    fp = open(filename, _READ_MODE, encoding=encoding)
                description = (".py", _READ_MODE, imp.PY_SOURCE)
                return (fp, filename, description)

            else:
                for _sfx, _mode, _type in imp.get_suffixes():
                    if _type == imp.C_EXTENSION and filename.endswith(_sfx):
                        description = (_sfx, "rb", imp.C_EXTENSION)
                        break
                else:
                    description = ("", "", imp.PKG_DIRECTORY)

                return (None, filename, description)

        if hasattr(loader, "path"):
            if loader.path.endswith(".pyc") or loader.path.endswith(".pyo"):
                fp = open(loader.path, "rb")
                description = (".pyc", "rb", imp.PY_COMPILED)
                return (fp, loader.path, description)

        if hasattr(loader, "get_source"):
            source = loader.get_source(name)
            fp = StringIO(source)
            co = None

        else:
            source = None

        if source is None:
            if hasattr(loader, "get_code"):
                co = loader.get_code(name)
                fp = _code_to_file(co)

            else:
                fp = None
                co = None

        pathname = os.path.join(entry, *name.split("."))

        if isinstance(loader, zipimport.zipimporter):
            # Check if this happens to be a wrapper module introduced by
            # setuptools, if it is we return the actual extension.
            zn = "/".join(name.split("."))
            for _sfx, _mode, _type in imp.get_suffixes():
                if _type == imp.C_EXTENSION:
                    p = loader.prefix + zn + _sfx
                    if loader._files is None:
                        loader_files = zipimport._zip_directory_cache[loader.archive]
                    else:
                        loader_files = loader._files

                    if p in loader_files:
                        description = (_sfx, "rb", imp.C_EXTENSION)
                        return (None, pathname + _sfx, description)

        if hasattr(loader, "is_package") and loader.is_package(name):
            return (None, pathname, ("", "", imp.PKG_DIRECTORY))

        if co is None:
            if hasattr(loader, "path"):
                filename = loader.path
            elif hasattr(loader, "get_filename"):
                filename = loader.get_filename(name)
                if source is not None:
                    if filename.endswith(".pyc") or filename.endswith(".pyo"):
                        filename = filename[:-1]
            else:
                filename = None

            if filename is not None and (
                filename.endswith(".py") or filename.endswith(".pyw")
            ):
                return (fp, filename, (".py", "rU", imp.PY_SOURCE))
            else:
                if fp is not None:
                    fp.close()
                return (
                    None,
                    filename,
                    (os.path.splitext(filename)[-1], "rb", imp.C_EXTENSION),
                )

        else:
            if hasattr(loader, "path"):
                return (fp, loader.path, (".pyc", "rb", imp.PY_COMPILED))
            else:
                return (fp, pathname + ".pyc", (".pyc", "rb", imp.PY_COMPILED))

    if namespace_path:
        if fp is not None:
            fp.close()
        return (None, namespace_path[0], ("", namespace_path, imp.PKG_DIRECTORY))

    raise ImportError(name)


def moduleInfoForPath(path):
    for (ext, readmode, typ) in imp.get_suffixes():
        if path.endswith(ext):
            return os.path.basename(path)[: -len(ext)], readmode, typ
    return None


def AddPackagePath(packagename, path):
    warnings.warn("Use addPackagePath instead of AddPackagePath", DeprecationWarning)

    addPackagePath(packagename, path)


def addPackagePath(packagename, path):
    paths = _packagePathMap.get(packagename, [])
    paths.append(path)
    _packagePathMap[packagename] = paths


_replacePackageMap = {}


# This ReplacePackage mechanism allows modulefinder to work around the
# way the _xmlplus package injects itself under the name "xml" into
# sys.modules at runtime by calling ReplacePackage("_xmlplus", "xml")
# before running ModuleGraph.
def ReplacePackage(oldname, newname):
    warnings.warn("use replacePackage instead of ReplacePackage", DeprecationWarning)
    replacePackage(oldname, newname)


def replacePackage(oldname, newname):
    _replacePackageMap[oldname] = newname


class DependencyInfo(
    namedtuple("DependencyInfo", ["conditional", "function", "tryexcept", "fromlist"])
):
    __slots__ = ()

    def _merged(self, other):
        if (not self.conditional and not self.function and not self.tryexcept) or (
            not other.conditional and not other.function and not other.tryexcept
        ):
            return DependencyInfo(
                conditional=False,
                function=False,
                tryexcept=False,
                fromlist=self.fromlist and other.fromlist,
            )

        else:
            return DependencyInfo(
                conditional=self.conditional or other.conditional,
                function=self.function or other.function,
                tryexcept=self.tryexcept or other.tryexcept,
                fromlist=self.fromlist and other.fromlist,
            )


class Node(object):
    def __init__(self, identifier):
        self.debug = 0
        self.graphident = identifier
        self.identifier = identifier
        self._namespace = {}
        self.filename = None
        self.packagepath = None
        self.code = None
        # The set of global names that are assigned to in the module.
        # This includes those names imported through starimports of
        # Python modules.
        self.globalnames = set()
        # The set of starimports this module did that could not be
        # resolved, ie. a starimport from a non-Python module.
        self.starimports = set()

    def __contains__(self, name):
        return name in self._namespace

    def __getitem__(self, name):
        return self._namespace[name]

    def __setitem__(self, name, value):
        self._namespace[name] = value

    def get(self, *args):
        return self._namespace.get(*args)

    def __cmp__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return NotImplemented

        return cmp(self.graphident, otherIdent)  # noqa: F821

    def __eq__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return False

        return self.graphident == otherIdent

    def __ne__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return True

        return self.graphident != otherIdent

    def __lt__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return NotImplemented

        return self.graphident < otherIdent

    def __le__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return NotImplemented

        return self.graphident <= otherIdent

    def __gt__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return NotImplemented

        return self.graphident > otherIdent

    def __ge__(self, other):
        try:
            otherIdent = other.graphident
        except AttributeError:
            return NotImplemented

        return self.graphident >= otherIdent

    def __hash__(self):
        return hash(self.graphident)

    def infoTuple(self):
        return (self.identifier,)

    def __repr__(self):
        return "%s%r" % (type(self).__name__, self.infoTuple())


class Alias(str):
    pass


class AliasNode(Node):
    def __init__(self, name, node):
        super(AliasNode, self).__init__(name)
        for k in (
            "identifier",
            "packagepath",
            "_namespace",
            "globalnames",
            "starimports",
        ):
            setattr(self, k, getattr(node, k, None))

    def infoTuple(self):
        return (self.graphident, self.identifier)


class BadModule(Node):
    pass


class ExcludedModule(BadModule):
    pass


class MissingModule(BadModule):
    pass


class InvalidRelativeImport(BadModule):
    def __init__(self, relative_path, from_name):
        identifier = relative_path
        if relative_path.endswith("."):
            identifier += from_name
        else:
            identifier += "." + from_name
        super(InvalidRelativeImport, self).__init__(identifier)
        self.relative_path = relative_path
        self.from_name = from_name

    def infoTuple(self):
        return (self.relative_path, self.from_name)


class Script(Node):
    def __init__(self, filename):
        super(Script, self).__init__(filename)
        self.filename = filename

    def infoTuple(self):
        return (self.filename,)


class BaseModule(Node):
    def __init__(self, name, filename=None, path=None):
        super(BaseModule, self).__init__(name)
        self.filename = filename
        self.packagepath = path

    def infoTuple(self):
        return tuple(filter(None, (self.identifier, self.filename, self.packagepath)))


class BuiltinModule(BaseModule):
    pass


class RuntimeModule(MissingModule):
    """
    Graph node representing a Python module dynamically defined at runtime.

    Most modules are statically defined at creation time in external Python
    files. Some modules, however, are dynamically defined at runtime (e.g.,
    `six.moves`, dynamically defined by the statically defined `six` module).

    This node represents such a module. To ensure that the parent module of
    this module is also imported and added to the graph, this node is typically
    added to the graph by calling the `ModuleGraph.add_module()` method.
    """

    pass


class SourceModule(BaseModule):
    pass


class InvalidSourceModule(SourceModule):
    pass


class CompiledModule(BaseModule):
    pass


class InvalidCompiledModule(BaseModule):
    pass


class Package(BaseModule):
    pass


class NamespacePackage(Package):
    pass


class Extension(BaseModule):
    pass


class FlatPackage(BaseModule):
    def __init__(self, *args, **kwds):
        warnings.warn(
            "This class will be removed in a future version of modulegraph",
            DeprecationWarning,
        )
        super(FlatPackage, *args, **kwds)


class ArchiveModule(BaseModule):
    def __init__(self, *args, **kwds):
        warnings.warn(
            "This class will be removed in a future version of modulegraph",
            DeprecationWarning,
        )
        super(FlatPackage, *args, **kwds)


# HTML templates for ModuleGraph generator
header = """\
<html>
  <head>
    <title>%(TITLE)s</title>
    <style>
      .node { margin:1em 0; }
    </style>
  </head>
  <body>
    <h1>%(TITLE)s</h1>"""
entry = """
<div class="node">
  <a name="%(NAME)s" />
  %(CONTENT)s
</div>"""
contpl = """<tt>%(NAME)s</tt> %(TYPE)s"""
contpl_linked = """\
<a target="code" href="%(URL)s" type="text/plain"><tt>%(NAME)s</tt></a>"""
imports = """\
  <div class="import">
%(HEAD)s:
  %(LINKS)s
  </div>
"""
footer = """
  </body>
</html>"""


def _ast_names(names):
    result = []
    for nm in names:
        if isinstance(nm, ast.alias):
            result.append(nm.name)
        else:
            result.append(nm)
    return result


if sys.version_info[0] == 2:
    DEFAULT_IMPORT_LEVEL = -1
else:
    DEFAULT_IMPORT_LEVEL = 0


class _Visitor(ast.NodeVisitor):
    def __init__(self, module):
        self._module = module
        self._level = DEFAULT_IMPORT_LEVEL
        self._in_if = [False]
        self._in_def = [False]
        self._in_tryexcept = [False]

        self.imports = []

    @property
    def in_if(self):
        return self._in_if[-1]

    @property
    def in_def(self):
        return self._in_def[-1]

    @property
    def in_tryexcept(self):
        return self._in_tryexcept[-1]

    def _process_import(self, name, fromlist, level):

        if sys.version_info[0] == 2:
            if name == "__future__" and "absolute_import" in (fromlist or ()):
                self._level = 0

        have_star = False
        if fromlist is not None:
            fromlist = set(fromlist)
            if "*" in fromlist:
                fromlist.remove("*")
                have_star = True

        self.imports.append(
            (
                name,
                have_star,
                (name, self._module, fromlist, level),
                {
                    "attr": DependencyInfo(
                        conditional=self.in_if,
                        tryexcept=self.in_tryexcept,
                        function=self.in_def,
                        fromlist=False,
                    )
                },
            )
        )

    def visit_Import(self, node):
        for nm in _ast_names(node.names):
            self._process_import(nm, None, self._level)

    def visit_ImportFrom(self, node):
        level = node.level if node.level != 0 else self._level
        self._process_import(node.module or "", _ast_names(node.names), level)

    def visit_If(self, node):
        self._in_if.append(True)
        self.generic_visit(node)
        self._in_if.pop()

    def visit_FunctionDef(self, node):
        self._in_def.append(True)
        self.generic_visit(node)
        self._in_def.pop()

    visit_AsyncFunctionDef = visit_FunctionDef

    def visit_Try(self, node):
        self._in_tryexcept.append(True)
        self.generic_visit(node)
        self._in_tryexcept.pop()

    def visit_TryExcept(self, node):
        self._in_tryexcept.append(True)
        self.generic_visit(node)
        self._in_tryexcept.pop()

    def visit_ExceptHandler(self, node):
        self._in_tryexcept.append(True)
        self.generic_visit(node)
        self._in_tryexcept.pop()

    def visit_Expression(self, node):
        # Expression node's cannot contain import statements or
        # other nodes that are relevant for us.
        pass

    # Expression isn't actually used as such in AST trees,
    # therefore define visitors for all kinds of expression nodes.
    visit_BoolOp = visit_Expression
    visit_BinOp = visit_Expression
    visit_UnaryOp = visit_Expression
    visit_Lambda = visit_Expression
    visit_IfExp = visit_Expression
    visit_Dict = visit_Expression
    visit_Set = visit_Expression
    visit_ListComp = visit_Expression
    visit_SetComp = visit_Expression
    visit_ListComp = visit_Expression
    visit_GeneratorExp = visit_Expression
    visit_Compare = visit_Expression
    visit_Yield = visit_Expression
    visit_YieldFrom = visit_Expression
    visit_Await = visit_Expression
    visit_Call = visit_Expression
    visit_Await = visit_Expression


class ModuleGraph(ObjectGraph):
    """
    Directed graph whose nodes represent modules and edges represent
    dependencies between these modules.
    """

    def __init__(
        self, path=None, excludes=(), replace_paths=(), implies=(), graph=None, debug=0
    ):
        super(ModuleGraph, self).__init__(graph=graph, debug=debug)
        if path is None:
            path = sys.path
        self.path = path
        self.lazynodes = {}
        # excludes is stronger than implies
        self.lazynodes.update(dict(implies))
        for m in excludes:
            self.lazynodes[m] = None
        self.replace_paths = replace_paths

        self.nspackages = self._calc_setuptools_nspackages()

    def _calc_setuptools_nspackages(self):
        # Setuptools has some magic handling for namespace
        # packages when using 'install --single-version-externally-managed'
        # (used by system packagers and also by pip)
        #
        # When this option is used namespace packages are writting to
        # disk *without* an __init__.py file, which means the regular
        # import machinery will not find them.
        #
        # We therefore explicitly look for the hack used by
        # setuptools to get this kind of namespace packages to work.

        pkgmap = {}

        try:
            from pkgutil import ImpImporter
        except ImportError:
            try:
                from _pkgutil import ImpImporter
            except ImportError:
                ImpImporter = pkg_resources.ImpWrapper

        if sys.version_info[:2] >= (3, 3):
            import importlib.machinery

            ImpImporter = importlib.machinery.FileFinder

        for entry in self.path:
            importer = pkg_resources.get_importer(entry)

            if isinstance(importer, ImpImporter):
                try:
                    ldir = os.listdir(entry)
                except os.error:
                    continue

                for fn in ldir:
                    if fn.endswith("-nspkg.pth"):
                        fp = open(os.path.join(entry, fn))
                        try:
                            for ln in fp:
                                for pfx in _SETUPTOOLS_NAMESPACEPKG_PTHs:
                                    if ln.startswith(pfx):
                                        try:
                                            start = len(pfx) - 2
                                            stop = ln.index(")", start) + 1
                                        except ValueError:
                                            continue

                                        pkg = _eval_str_tuple(ln[start:stop])
                                        identifier = ".".join(pkg)
                                        subdir = os.path.join(entry, *pkg)
                                        if os.path.exists(
                                            os.path.join(subdir, "__init__.py")
                                        ):
                                            # There is a real __init__.py,
                                            # ignore the setuptools hack
                                            continue

                                        if identifier in pkgmap:
                                            pkgmap[identifier].append(subdir)
                                        else:
                                            pkgmap[identifier] = [subdir]
                                        break
                        finally:
                            fp.close()

        return pkgmap

    def implyNodeReference(self, node, other, edge_data=None):
        """
        Create a reference from the passed source node to the passed other node,
        implying the former to depend upon the latter.

        While the source node *must* be an existing graph node, the target node
        may be either an existing graph node *or* a fully-qualified module name.
        In the latter case, the module with that name and all parent packages of
        that module will be imported *without* raising exceptions and for each
        newly imported module or package:

        * A new graph node will be created for that module or package.
        * A reference from the passed source node to that module or package will
          be created.

        This method allows dependencies between Python objects *not* importable
        with standard techniques (e.g., module aliases, C extensions).

        Parameters
        ----------
        node : str
            Graph node for this reference's source module or package.
        other : {Node, str}
            Either a graph node *or* fully-qualified name for this reference's
            target module or package.
        """
        if isinstance(other, Node):
            self._updateReference(node, other, edge_data)

        else:
            if isinstance(other, tuple):
                raise ValueError(other)

            others = self._safe_import_hook(other, node, None)
            for other in others:
                self._updateReference(node, other, edge_data)

    def getReferences(self, fromnode):
        """
        Yield all nodes that 'fromnode' dependes on (that is,
        all modules that 'fromnode' imports.
        """
        node = self.findNode(fromnode)
        out_edges, _ = self.get_edges(node)
        return out_edges

    def getReferers(self, tonode, collapse_missing_modules=True):
        node = self.findNode(tonode)
        _, in_edges = self.get_edges(node)

        if collapse_missing_modules:
            for n in in_edges:
                if isinstance(n, MissingModule):
                    for n in self.getReferers(n, False):
                        yield n

                else:
                    yield n

        else:
            for n in in_edges:
                yield n

    def hasEdge(self, fromnode, tonode):
        """Return True iff there is an edge from 'fromnode' to 'tonode'"""
        fromnode = self.findNode(fromnode)
        tonode = self.findNode(tonode)

        return self.graph.edge_by_node(fromnode, tonode) is not None

    def foldReferences(self, packagenode):
        """
        Create edges to/from 'packagenode' based on the
        edges to/from modules in package. The module nodes
        are then hidden.
        """
        pkg = self.findNode(packagenode)

        for n in self.nodes():
            if not n.identifier.startswith(pkg.identifier + "."):
                continue

            iter_out, iter_inc = self.get_edges(n)
            for other in iter_out:
                if other.identifier.startswith(pkg.identifier + "."):
                    continue

                if not self.hasEdge(pkg, other):
                    # Ignore circular dependencies
                    self._updateReference(pkg, other, "pkg-internal-import")

            for other in iter_inc:
                if other.identifier.startswith(pkg.identifier + "."):
                    # Ignore circular dependencies
                    continue

                if not self.hasEdge(other, pkg):
                    self._updateReference(other, pkg, "pkg-import")

            self.graph.hide_node(n)

    def _updateReference(self, fromnode, tonode, edge_data):
        try:
            ed = self.edgeData(fromnode, tonode)
        except (KeyError, GraphError):
            return self.createReference(fromnode, tonode, edge_data)

        if not (
            isinstance(ed, DependencyInfo) and isinstance(edge_data, DependencyInfo)
        ):
            self.updateEdgeData(fromnode, tonode, edge_data)
        else:
            self.updateEdgeData(fromnode, tonode, ed._merged(edge_data))

    def createReference(self, fromnode, tonode, edge_data="direct"):
        """
        Create a reference from fromnode to tonode
        """
        return super(ModuleGraph, self).createReference(
            fromnode, tonode, edge_data=edge_data
        )

    def findNode(self, name):
        """
        Find a node by identifier.  If a node by that identifier exists,
        it will be returned.

        If a lazy node exists by that identifier with no dependencies (excluded),
        it will be instantiated and returned.

        If a lazy node exists by that identifier with dependencies, it and its
        dependencies will be instantiated and scanned for additional dependencies.
        """
        data = super(ModuleGraph, self).findNode(name)
        if data is not None:
            return data
        if name in self.lazynodes:
            deps = self.lazynodes.pop(name)
            if deps is None:
                # excluded module
                m = self.createNode(ExcludedModule, name)
            elif isinstance(deps, Alias):
                other = self._safe_import_hook(deps, None, None).pop()
                m = self.createNode(AliasNode, name, other)
                self.implyNodeReference(m, other)

            else:
                m = self._safe_import_hook(name, None, None).pop()
                for dep in deps:
                    self.implyNodeReference(m, dep)
            return m

        if name in self.nspackages:
            # name is a --single-version-externally-managed
            # namespace package (setuptools/distribute)
            pathnames = self.nspackages.pop(name)
            m = self.createNode(NamespacePackage, name)

            # The filename must be set to a string to ensure that py2app
            # works, it is not clear yet why that is. Setting to None would be
            # cleaner.
            m.filename = "-"
            m.packagepath = _namespace_package_path(name, pathnames, self.path)

            # As per comment at top of file, simulate runtime packagepath additions.
            m.packagepath = m.packagepath + _packagePathMap.get(name, [])
            return m

        return None

    def run_script(self, pathname, caller=None):
        """
        Create a node by path (not module name).  It is expected to be a Python
        source file, and will be scanned for dependencies.
        """
        self.msg(2, "run_script", pathname)
        pathname = os.path.realpath(pathname)
        m = self.findNode(pathname)
        if m is not None:
            return m

        if sys.version_info[0] != 2:
            with open(pathname, "rb") as fp:
                encoding = util.guess_encoding(fp)

            with open(pathname, _READ_MODE, encoding=encoding) as fp:
                contents = fp.read() + "\n"

            if contents.startswith(BOM):
                # Ignore BOM at start of input
                contents = contents[1:]

        else:
            with open(pathname, _READ_MODE) as fp:
                contents = fp.read() + "\n"

        co = compile(contents, pathname, "exec", ast.PyCF_ONLY_AST, True)
        m = self.createNode(Script, pathname)
        self._updateReference(caller, m, None)
        self._scan_code(co, m)
        m.code = compile(co, pathname, "exec", 0, True)
        if self.replace_paths:
            m.code = self._replace_paths_in_code(m.code)
        return m

    def import_hook(
        self, name, caller=None, fromlist=None, level=DEFAULT_IMPORT_LEVEL, attr=None
    ):
        """
        Import a module

        Return the set of modules that are imported
        """
        self.msg(3, "import_hook", name, caller, fromlist, level)
        parent = self._determine_parent(caller)
        q, tail = self._find_head_package(parent, name, level)
        m = self._load_tail(q, tail)
        modules = [m]
        if fromlist and m.packagepath:
            for s in self._ensure_fromlist(m, fromlist):
                if s not in modules:
                    modules.append(s)
        for m in modules:
            self._updateReference(caller, m, edge_data=attr)
        return modules

    def _determine_parent(self, caller):
        """
        Determine the package containing a node
        """
        self.msgin(4, "determine_parent", caller)
        parent = None
        if caller:
            pname = caller.identifier

            if isinstance(caller, Package):
                parent = caller

            elif "." in pname:
                pname = pname[: pname.rfind(".")]
                parent = self.findNode(pname)

            elif caller.packagepath:
                parent = self.findNode(pname)

        self.msgout(4, "determine_parent ->", parent)
        return parent

    def _find_head_package(self, parent, name, level=DEFAULT_IMPORT_LEVEL):
        """
        Given a calling parent package and an import name determine the containing
        package for the name
        """
        self.msgin(4, "find_head_package", parent, name, level)
        if "." in name:
            head, tail = name.split(".", 1)
        else:
            head, tail = name, ""

        if level == -1:
            if parent:
                qname = parent.identifier + "." + head
            else:
                qname = head

        elif level == 0:
            qname = head

            # Absolute import, ignore the parent
            parent = None

        else:
            if parent is None:
                self.msg(2, "Relative import outside of package")
                raise InvalidRelativeImportError(
                    "Relative import outside of package (name=%r, parent=%r, level=%r)"
                    % (name, parent, level)
                )

            for _i in range(level - 1):
                if "." not in parent.identifier:
                    self.msg(2, "Relative import outside of package")
                    raise InvalidRelativeImportError(
                        "Relative import outside of package (name=%r, parent=%r, level=%r)"
                        % (name, parent, level)
                    )

                p_fqdn = parent.identifier.rsplit(".", 1)[0]
                new_parent = self.findNode(p_fqdn)
                if new_parent is None:
                    self.msg(2, "Relative import outside of package")
                    raise InvalidRelativeImportError(
                        "Relative import outside of package (name=%r, parent=%r, level=%r)"
                        % (name, parent, level)
                    )

                assert new_parent is not parent, (new_parent, parent)
                parent = new_parent

            if head:
                qname = parent.identifier + "." + head
            else:
                qname = parent.identifier

        q = self._import_module(head, qname, parent)
        if q:
            self.msgout(4, "find_head_package ->", (q, tail))
            return q, tail
        if parent:
            qname = head
            parent = None
            q = self._import_module(head, qname, parent)
            if q:
                self.msgout(4, "find_head_package ->", (q, tail))
                return q, tail
        self.msgout(4, "raise ImportError: No module named", qname)
        raise ImportError("No module named " + qname)

    def _load_tail(self, mod, tail):
        self.msgin(4, "load_tail", mod, tail)
        result = mod
        while tail:
            i = tail.find(".")
            if i < 0:
                i = len(tail)
            head, tail = tail[:i], tail[i + 1 :]  # noqa: E203
            mname = "%s.%s" % (result.identifier, head)
            result = self._import_module(head, mname, result)
            if result is None:
                result = self.createNode(MissingModule, mname)
        self.msgout(4, "load_tail ->", result)
        return result

    def _ensure_fromlist(self, m, fromlist):
        fromlist = set(fromlist)
        self.msg(4, "ensure_fromlist", m, fromlist)
        if "*" in fromlist:
            fromlist.update(self._find_all_submodules(m))
            fromlist.remove("*")
        for sub in fromlist:
            submod = m.get(sub)
            if submod is None:
                if sub in m.globalnames:
                    # Name is a global in the module
                    continue

                fullname = m.identifier + "." + sub
                submod = self._import_module(sub, fullname, m)
                if submod is None:
                    raise ImportError("No module named " + fullname)
            yield submod

    def _find_all_submodules(self, m):
        if not m.packagepath:
            return
        # 'suffixes' used to be a list hardcoded to [".py", ".pyc", ".pyo"].
        # But we must also collect Python extension modules - although
        # we cannot separate normal dlls from Python extensions.
        for path in m.packagepath:
            try:
                names = zipio.listdir(path)
            except (os.error, IOError):
                self.msg(2, "can't list directory", path)
                continue
            for info in (moduleInfoForPath(p) for p in names):
                if info is None:
                    continue
                if info[0] != "__init__":
                    yield info[0]

    def alias_module(self, src_module_name, trg_module_name):
        """
        Alias the source module to the target module with the passed names.

        This method ensures that the next call to findNode() given the target
        module name will resolve this alias. This includes importing and adding
        a graph node for the source module if needed as well as adding a
        reference from the target to source module.

        Parameters
        ----------
        src_module_name : str
            Fully-qualified name of the existing **source module** (i.e., the
            module being aliased).
        trg_module_name : str
            Fully-qualified name of the non-existent **target module** (i.e.,
            the alias to be created).
        """
        self.msg(3, 'alias_module "%s" -> "%s"' % (src_module_name, trg_module_name))
        # print('alias_module "%s" -> "%s"' % (src_module_name, trg_module_name))
        assert isinstance(src_module_name, str), '"%s" not a module name.' % str(
            src_module_name
        )
        assert isinstance(trg_module_name, str), '"%s" not a module name.' % str(
            trg_module_name
        )

        # If the target module has already been added to the graph as either a
        # non-alias or as a different alias, raise an exception.
        trg_module = self.findNode(trg_module_name)
        if trg_module is not None and not (
            isinstance(trg_module, AliasNode)
            and trg_module.identifier == src_module_name
        ):
            raise ValueError(
                'Target module "%s" already imported as "%s".'
                % (trg_module_name, trg_module)
            )

        # See findNode() for details.
        self.lazynodes[trg_module_name] = Alias(src_module_name)

    def add_module(self, module):
        """
        Add the passed module node to the graph if not already added.

        If that module has a parent module or package with a previously added
        node, this method also adds a reference from this module node to its
        parent node and adds this module node to its parent node's namespace.

        This high-level method wraps the low-level `addNode()` method, but is
        typically *only* called by graph hooks adding runtime module nodes. For
        all other node types, the `import_module()` method should be called.

        Parameters
        ----------
        module : BaseModule
            Graph node for the module to be added.
        """
        self.msg(3, "import_module_runtime", module)

        # If no node exists for this module, add such a node.
        module_added = self.findNode(module.identifier)
        if module_added is None:
            self.addNode(module)
        else:
            assert module == module_added, "New module %r != previous %r" % (
                module,
                module_added,
            )

        # If this module has a previously added parent, reference this module to
        # its parent and add this module to its parent's namespace.
        parent_name, _, module_basename = module.identifier.rpartition(".")
        if parent_name:
            parent = self.findNode(parent_name)
            if parent is None:
                self.msg(4, "import_module_runtime parent not found:", parent_name)
            else:
                self.createReference(module, parent)
                parent[module_basename] = module

    def _import_module(self, partname, fqname, parent):
        """
        Import the Python module with the passed name from the parent package
        signified by the passed graph node.

        Parameters
        ----------
        partname : str
            Unqualified name of the module to be imported (e.g., `text`).
        fqname : str
            Fully-qualified name of this module (e.g., `email.mime.text`).
        parent : Package
            Graph node for the package providing this module *or* `None` if
            this module is a top-level module.

        Returns
        ----------
        Node
            Graph node created for this module.
        """
        self.msgin(3, "import_module", partname, fqname, parent)
        m = self.findNode(fqname)
        if m is not None:
            self.msgout(3, "import_module ->", m)
            if parent:
                self._updateReference(
                    m,
                    parent,
                    edge_data=DependencyInfo(
                        conditional=False,
                        fromlist=False,
                        function=False,
                        tryexcept=False,
                    ),
                )
            return m

        if parent and parent.packagepath is None:
            self.msgout(3, "import_module -> None")
            return None

        try:
            searchpath = None
            if parent is not None and parent.packagepath:
                searchpath = parent.packagepath

            fp, pathname, stuff = self._find_module(partname, searchpath, parent)

        except ImportError:
            self.msgout(3, "import_module ->", None)
            return None

        try:
            m = self._load_module(fqname, fp, pathname, stuff)

        finally:
            if fp is not None:
                fp.close()

        if parent:
            self.msg(4, "create reference", m, "->", parent)
            self._updateReference(
                m,
                parent,
                edge_data=DependencyInfo(
                    conditional=False, fromlist=False, function=False, tryexcept=False
                ),
            )
            parent[partname] = m

        self.msgout(3, "import_module ->", m)
        return m

    def _load_module(self, fqname, fp, pathname, info):
        suffix, mode, typ = info
        self.msgin(2, "load_module", fqname, fp and "fp", pathname)

        if typ == imp.PKG_DIRECTORY:
            if isinstance(mode, (list, tuple)):
                packagepath = mode
            else:
                packagepath = []

            m = self._load_package(fqname, pathname, packagepath)
            self.msgout(2, "load_module ->", m)
            return m

        if typ == imp.PY_SOURCE:
            contents = fp.read()
            if isinstance(contents, bytes):
                contents += b"\n"
            else:
                contents += "\n"

            try:
                co = compile(contents, pathname, "exec", ast.PyCF_ONLY_AST, True)
                if sys.version_info[:2] == (3, 5):
                    # In Python 3.5 some syntax problems with async
                    # functions are only reported when compiling to bytecode
                    compile(co, "-", "exec", 0, True)
            except SyntaxError:
                co = None
                cls = InvalidSourceModule

            else:
                cls = SourceModule

        elif typ == imp.PY_COMPILED:
            data = fp.read(4)
            magic = imp.get_magic()

            if data != magic:
                self.msg(2, "raise ImportError: Bad magic number", pathname)
                co = None
                cls = InvalidCompiledModule

            else:
                if sys.version_info[:2] >= (3, 7):
                    fp.read(12)
                elif sys.version_info[:2] >= (3, 4):
                    fp.read(8)
                else:
                    fp.read(4)

                try:
                    co = marshal.loads(fp.read())
                    cls = CompiledModule
                except Exception as exc:
                    raise
                    self.msgout(2, "raise ImportError: Cannot load code", pathname, exc)
                    co = None
                    cls = InvalidCompiledModule

        elif typ == imp.C_BUILTIN:
            cls = BuiltinModule
            co = None

        else:
            cls = Extension
            co = None

        m = self.createNode(cls, fqname)
        m.filename = pathname
        if co is not None:
            self._scan_code(co, m)

            if isinstance(co, ast.AST):
                try:
                    co = compile(co, pathname, "exec", 0, True)
                except RecursionError:
                    self.msg(
                        0, "Skip compiling %r due to recursion error" % (pathname,)
                    )
                    m.code = None
                    self.msgout(2, "load_module ->", m)
                    return m

            if self.replace_paths:
                co = self._replace_paths_in_code(co)
            m.code = co

        self.msgout(2, "load_module ->", m)
        return m

    def _safe_import_hook(
        self, name, caller, fromlist, level=DEFAULT_IMPORT_LEVEL, attr=None
    ):
        # wrapper for self.import_hook() that won't raise ImportError

        # List of graph nodes created for the modules imported by this call.
        mods = None

        # True if this is a Python 2-style implicit relative import of a
        # SWIG-generated C extension.
        is_swig_import = False

        # Attempt to import the module with Python's standard module importers.
        try:
            mods = self.import_hook(name, caller, level=level, attr=attr)
        # Failing that, defer to custom module importers handling non-standard
        # import schemes (e.g., SWIG, six).

        except InvalidRelativeImportError:
            self.msgout(2, "Invalid relative import", level, name, fromlist)
            result = []
            for sub in fromlist or "*":
                m = self.createNode(InvalidRelativeImport, "." * level + name, sub)
                self._updateReference(caller, m, edge_data=attr)
                result.append(m)

            return result

        except ImportError as msg:
            # If this is an absolute top-level import under Python 3 and if the
            # name to be imported is the caller's name prefixed by "_", this
            # could be a SWIG-generated Python 2-style implicit relative import.
            # SWIG-generated files contain functions named swig_import_helper()
            # importing dynamic libraries residing in the same directory. For
            # example, a SWIG-generated caller module "csr.py" might resemble:
            #
            #     # This file was automatically generated by SWIG (http://www.swig.org).
            #     ...
            #     def swig_import_helper():
            #         ...
            #         try:
            #             fp, pathname, description = imp.find_module('_csr',
            #                   [dirname(__file__)])
            #         except ImportError:
            #             import _csr
            #             return _csr
            #
            # While there exists no reasonable means for modulegraph to parse
            # the call to imp.find_module(), the subsequent implicit relative
            # import is trivially parsable. This import is prohibited under
            # Python 3, however, and thus parsed only if the caller's file is
            # parsable plaintext (as indicated by a filetype of ".py") and the
            # first line of this file is the above SWIG header comment.
            #
            # The constraint that this library's name be the caller's name
            # prefixed by '_' is explicitly mandated by SWIG and thus a
            # reliable indicator of "SWIG-ness". The SWIG documentation states:
            # "When linking the module, the name of the output file has to match
            #  the name of the module prefixed by an underscore."
            #
            # A MissingModule is not a SWIG import candidate.
            if (
                caller is not None
                and type(caller) is not MissingModule
                and fromlist is None
                and level == 0
                and caller.filename.endswith(".py")
                and name == "_" + caller.identifier.rpartition(".")[2]
                and sys.version_info[0] == 3
            ):
                self.msg(
                    4,
                    "SWIG import candidate (name=%r, caller=%r, level=%r)"
                    % (name, caller, level),
                )

                with open(caller.filename, "rb") as caller_file:
                    encoding = util.guess_encoding(caller_file)
                with open(
                    caller.filename, _READ_MODE, encoding=encoding
                ) as caller_file:
                    first_line = caller_file.readline()
                    self.msg(5, "SWIG import candidate shebang: %r" % (first_line))
                    if "automatically generated by SWIG" in first_line:
                        is_swig_import = True

                        # Convert this Python 2-style implicit relative import
                        # into a Python 3-style explicit relative import for
                        # the duration of this function call by overwriting
                        # the original parameters passed to this call.
                        fromlist = [name]
                        name = ""
                        level = 1
                        self.msg(
                            2,
                            "SWIG import (caller=%r, fromlist=%r, level=%r)"
                            % (caller, fromlist, level),
                        )

                        # Import the caller module importing this library.
                        try:
                            mods = self.import_hook(
                                name, caller, level=level, attr=attr
                            )
                        except ImportError as msg:
                            self.msg(2, "SWIG ImportError:", str(msg))

            # If this module remains unimportable, add a MissingModule node.
            if mods is None:
                self.msg(2, "ImportError:", str(msg))

                m = self.createNode(MissingModule, _path_from_importerror(msg, name))
                self._updateReference(caller, m, edge_data=attr)

        # If this module was successfully imported, get its graph node.
        if mods is not None:
            assert len(mods) == 1
            m = list(mods)[0]

        subs = [m]
        if isinstance(attr, DependencyInfo):
            attr = attr._replace(fromlist=True)
        for sub in fromlist or ():
            # If this name is in the module namespace already,
            # then add the entry to the list of substitutions
            if sub in m:
                sm = m[sub]
                if sm is not None:
                    if sm not in subs:
                        self._updateReference(caller, sm, edge_data=attr)
                        subs.append(sm)
                    continue

            elif sub in m.globalnames:
                # Global variable in the module, ignore
                continue

            # See if we can load it
            #    fullname = name + '.' + sub
            fullname = m.identifier + "." + sub
            sm = self.findNode(fullname)
            if sm is None:
                try:
                    sm = self.import_hook(
                        name, caller, fromlist=[sub], level=level, attr=attr
                    )
                except ImportError as msg:
                    self.msg(2, "ImportError:", str(msg))
                    sm = self.createNode(MissingModule, fullname)
                else:
                    sm = self.findNode(fullname)
                    if sm is None:
                        sm = self.createNode(MissingModule, fullname)

                    # If this is a SWIG C extension, instruct downstream
                    # freezers (py2app, PyInstaller) to freeze this extension
                    # under its unqualified rather than qualified name (e.g.,
                    # as "_csr" rather than "scipy.sparse.sparsetools._csr"),
                    # permitting the implicit relative import in its parent
                    # SWIG module to successfully find this extension.
                    if is_swig_import:
                        # If a graph node with that name already exists, avoid
                        # collisions by emitting an error instead.
                        if self.findNode(sub):
                            self.msg(
                                2,
                                "SWIG import error: %r basename %r "
                                "already exists" % (fullname, sub),
                            )
                        else:
                            self.msg(
                                4, "SWIG import renamed from %r to %r" % (fullname, sub)
                            )
                            sm.identifier = sub

            m[sub] = sm
            if sm is not None:
                self._updateReference(m, sm, edge_data=attr)
                self._updateReference(caller, sm, edge_data=attr)
                if sm not in subs:
                    subs.append(sm)
        return subs

    def _scan_code(self, co, m):
        if isinstance(co, ast.AST):
            self._scan_ast(co, m)
            try:
                self._scan_bytecode_stores(compile(co, "-", "exec", 0, True), m)
            except RecursionError:
                self.msg(
                    0, "RecursionError while compiling code, skip scanning bytecode"
                )

        else:
            self._scan_bytecode(co, m)

    def _scan_ast(self, co, m):
        visitor = _Visitor(m)
        visitor.visit(co)

        for name, have_star, args, kwds in visitor.imports:
            imported_module = self._safe_import_hook(*args, **kwds)[0]
            if have_star:
                m.globalnames.update(imported_module.globalnames)
                m.starimports.update(imported_module.starimports)
                if imported_module.code is None:
                    m.starimports.add(name)

    if sys.version_info[:2] >= (3, 4):
        # On Python 3.4 or later the dis module has a much nicer interface
        # for working with bytecode, use that instead of peeking into the
        # raw bytecode.
        # Note: This nicely sidesteps any issues caused by moving from bytecode
        # to wordcode in python 3.6.

        def _scan_bytecode_stores(self, co, m):
            constants = co.co_consts
            for inst in dis.get_instructions(co):
                if inst.opname in ("STORE_NAME", "STORE_GLOBAL"):
                    name = co.co_names[inst.arg]
                    m.globalnames.add(name)

            cotype = type(co)
            for c in constants:
                if isinstance(c, cotype):
                    self._scan_bytecode_stores(c, m)

        def _scan_bytecode(self, co, m):
            constants = co.co_consts

            level = None
            fromlist = None

            prev_insts = []

            for inst in dis.get_instructions(co):
                if inst.opname == "IMPORT_NAME":
                    assert prev_insts[-2].opname == "LOAD_CONST"
                    assert prev_insts[-1].opname == "LOAD_CONST"

                    level = co.co_consts[prev_insts[-2].arg]
                    fromlist = co.co_consts[prev_insts[-1].arg]

                    assert fromlist is None or type(fromlist) is tuple
                    name = co.co_names[inst.arg]
                    have_star = False
                    if fromlist is not None:
                        fromlist = set(fromlist)
                        if "*" in fromlist:
                            fromlist.remove("*")
                            have_star = True

                    imported_module = self._safe_import_hook(name, m, fromlist, level)[
                        0
                    ]

                    if have_star:
                        m.globalnames.update(imported_module.globalnames)
                        m.starimports.update(imported_module.starimports)
                        if imported_module.code is None:
                            m.starimports.add(name)

                elif inst.opname in ("STORE_NAME", "STORE_GLOBAL"):
                    # keep track of all global names that are assigned to
                    name = co.co_names[inst.arg]
                    m.globalnames.add(name)

                prev_insts.append(inst)
                del prev_insts[:-2]

            cotype = type(co)
            for c in constants:
                if isinstance(c, cotype):
                    self._scan_bytecode(c, m)

    else:
        # Before 3.4: Peek into raw bytecode.

        def _scan_bytecode_stores(
            self,
            co,
            m,
            STORE_NAME=_Bchr(dis.opname.index("STORE_NAME")),  # noqa: M511,B008
            STORE_GLOBAL=_Bchr(dis.opname.index("STORE_GLOBAL")),  # noqa: M511,B008
            HAVE_ARGUMENT=_Bchr(dis.HAVE_ARGUMENT),  # noqa: M511,B008
            unpack=struct.unpack,
        ):

            code = co.co_code
            constants = co.co_consts
            n = len(code)
            i = 0

            while i < n:
                c = code[i]
                i += 1
                if c >= HAVE_ARGUMENT:
                    i = i + 2

                if c == STORE_NAME or c == STORE_GLOBAL:
                    # keep track of all global names that are assigned to
                    oparg = unpack("<H", code[i - 2 : i])[0]  # noqa: E203
                    name = co.co_names[oparg]
                    m.globalnames.add(name)

            cotype = type(co)
            for c in constants:
                if isinstance(c, cotype):
                    self._scan_bytecode_stores(c, m)

        def _scan_bytecode(
            self,
            co,
            m,
            HAVE_ARGUMENT=_Bchr(dis.HAVE_ARGUMENT),  # noqa: M511,B008
            LOAD_CONST=_Bchr(dis.opname.index("LOAD_CONST")),  # noqa: M511,B008
            IMPORT_NAME=_Bchr(dis.opname.index("IMPORT_NAME")),  # noqa: M511,B008
            IMPORT_FROM=_Bchr(dis.opname.index("IMPORT_FROM")),  # noqa: M511,B008
            STORE_NAME=_Bchr(dis.opname.index("STORE_NAME")),  # noqa: M511,B008
            STORE_GLOBAL=_Bchr(dis.opname.index("STORE_GLOBAL")),  # noqa: M511,B008
            unpack=struct.unpack,
        ):

            # Python >=2.5: LOAD_CONST flags, LOAD_CONST names, IMPORT_NAME name
            # Python < 2.5: LOAD_CONST names, IMPORT_NAME name
            extended_import = bool(sys.version_info[:2] >= (2, 5))

            code = co.co_code
            constants = co.co_consts
            n = len(code)
            i = 0

            level = None
            fromlist = None

            while i < n:
                c = code[i]
                i += 1
                if c >= HAVE_ARGUMENT:
                    i = i + 2

                if c == IMPORT_NAME:
                    if extended_import:
                        assert code[i - 9] == LOAD_CONST
                        assert code[i - 6] == LOAD_CONST
                        arg1, arg2 = unpack("<xHxH", code[i - 9 : i - 3])  # noqa: E203
                        level = co.co_consts[arg1]
                        fromlist = co.co_consts[arg2]
                    else:
                        assert code[-6] == LOAD_CONST
                        (arg1,) = unpack("<xH", code[i - 6 : i - 3])  # noqa: E203
                        level = -1
                        fromlist = co.co_consts[arg1]

                    assert fromlist is None or type(fromlist) is tuple
                    (oparg,) = unpack("<H", code[i - 2 : i])  # noqa: E203
                    name = co.co_names[oparg]
                    have_star = False
                    if fromlist is not None:
                        fromlist = set(fromlist)
                        if "*" in fromlist:
                            fromlist.remove("*")
                            have_star = True

                    imported_module = self._safe_import_hook(name, m, fromlist, level)[
                        0
                    ]

                    if have_star:
                        m.globalnames.update(imported_module.globalnames)
                        m.starimports.update(imported_module.starimports)
                        if imported_module.code is None:
                            m.starimports.add(name)

                elif c == STORE_NAME or c == STORE_GLOBAL:
                    # keep track of all global names that are assigned to
                    oparg = unpack("<H", code[i - 2 : i])[0]  # noqa: E203
                    name = co.co_names[oparg]
                    m.globalnames.add(name)

            cotype = type(co)
            for c in constants:
                if isinstance(c, cotype):
                    self._scan_bytecode(c, m)

    def _load_package(self, fqname, pathname, pkgpath):
        """
        Called only when an imp.PACKAGE_DIRECTORY is found
        """
        self.msgin(2, "load_package", fqname, pathname, pkgpath)
        newname = _replacePackageMap.get(fqname)
        if newname:
            fqname = newname

        ns_pkgpath = _namespace_package_path(fqname, pkgpath or [], self.path)
        if ns_pkgpath or pkgpath:
            # this is a namespace package
            m = self.createNode(NamespacePackage, fqname)
            m.filename = "-"
            m.packagepath = ns_pkgpath
        else:
            m = self.createNode(Package, fqname)
            m.filename = pathname
            m.packagepath = [pathname] + ns_pkgpath

        # As per comment at top of file, simulate runtime packagepath additions.
        m.packagepath = m.packagepath + _packagePathMap.get(fqname, [])

        try:
            self.msg(2, "find __init__ for %s" % (m.packagepath,))
            fp, buf, stuff = self._find_module("__init__", m.packagepath, parent=m)
        except ImportError:
            pass

        else:
            try:
                self.msg(2, "load __init__ for %s" % (m.packagepath,))
                self._load_module(fqname, fp, buf, stuff)
            finally:
                if fp is not None:
                    fp.close()
        self.msgout(2, "load_package ->", m)
        return m

    def _find_module(self, name, path, parent=None):
        """
        Get a 3-tuple detailing the physical location of the Python module with
        the passed name if that module is found *or* raise `ImportError`
        otherwise.

        This high-level method wraps the low-level `modulegraph.find_module()`
        function with additional support for graph-based module caching.

        Parameters
        ----------
        name : str
            Fully-qualified name of the Python module to be found.
        path : list
            List of the absolute paths of all directories to search for this
            module *or* `None` if the default path list `self.path` is to be
            searched.
        parent : Node
            Optional parent module of this module if this module is a submodule
            of another module *or* `None` if this module is a top-level module.

        Returns
        ----------
        (file_handle, filename, metadata)
            See `modulegraph.find_module()` for details.
        """
        if parent is not None:
            # assert path is not None
            fullname = parent.identifier + "." + name
        else:
            fullname = name

        node = self.findNode(fullname)
        if node is not None:
            self.msg(3, "find_module -> already included?", node)
            raise ImportError(name)

        if path is None:
            if name in sys.builtin_module_names:
                return (None, None, ("", "", imp.C_BUILTIN))

            path = self.path

        fp, buf, stuff = find_module(name, path)
        try:
            if buf:
                buf = os.path.realpath(buf)

            return (fp, buf, stuff)
        except:  # noqa: E722, B001
            fp.close()
            raise

    def create_xref(self, out=None):
        global header, footer, entry, contpl, contpl_linked, imports
        if out is None:
            out = sys.stdout
        scripts = []
        mods = []
        for mod in self.flatten():
            name = os.path.basename(mod.identifier)
            if isinstance(mod, Script):
                scripts.append((name, mod))
            else:
                mods.append((name, mod))
        scripts.sort()
        mods.sort()
        scriptnames = [sn for sn, m in scripts]
        scripts.extend(mods)
        mods = scripts

        title = "modulegraph cross reference for " + ", ".join(scriptnames)
        print(header % {"TITLE": title}, file=out)

        def sorted_namelist(mods):
            lst = [os.path.basename(mod.identifier) for mod in mods if mod]
            lst.sort()
            return lst

        for name, m in mods:
            content = ""
            if isinstance(m, BuiltinModule):
                content = contpl % {"NAME": name, "TYPE": "<i>(builtin module)</i>"}
            elif isinstance(m, Extension):
                content = contpl % {"NAME": name, "TYPE": "<tt>%s</tt>" % m.filename}
            else:
                url = pathname2url(m.filename or "")
                content = contpl_linked % {"NAME": name, "URL": url}
            oute, ince = map(sorted_namelist, self.get_edges(m))
            if oute:
                links = ""
                for n in oute:
                    links += """  <a href="#%s">%s</a>\n""" % (n, n)
                content += imports % {"HEAD": "imports", "LINKS": links}
            if ince:
                links = ""
                for n in ince:
                    links += """  <a href="#%s">%s</a>\n""" % (n, n)
                content += imports % {"HEAD": "imported by", "LINKS": links}
            print(entry % {"NAME": name, "CONTENT": content}, file=out)
        print(footer, file=out)

    def itergraphreport(self, name="G", flatpackages=()):
        nodes = list(map(self.graph.describe_node, self.graph.iterdfs(self)))
        describe_edge = self.graph.describe_edge
        edges = deque()
        packagenodes = set()
        packageidents = {}
        nodetoident = {}
        inpackages = {}
        mainedges = set()

        flatpackages = dict(flatpackages)

        def nodevisitor(node, data, outgoing, incoming):
            if not isinstance(data, Node):
                return {"label": str(node)}
            s = "<f0> " + type(data).__name__
            for i, v in enumerate(data.infoTuple()[:1], 1):
                s += "| <f%d> %s" % (i, v)
            return {"label": s, "shape": "record"}

        def edgevisitor(edge, data, head, tail):
            if data == "orphan":
                return {"style": "dashed"}
            elif data == "pkgref":
                return {"style": "dotted"}
            return {}

        yield "digraph %s {\n" % (name,)
        attr = {"rankdir": "LR", "concentrate": "true"}
        cpatt = '%s="%s"'
        for item in attr.items():
            yield "\t%s;\n" % (cpatt % item,)

        # find all packages (subgraphs)
        for (node, data, _outgoing, _incoming) in nodes:
            nodetoident[node] = getattr(data, "identifier", None)
            if isinstance(data, Package):
                packageidents[data.identifier] = node
                inpackages[node] = {node}
                packagenodes.add(node)

        # create sets for subgraph, write out descriptions
        for (node, data, outgoing, incoming) in nodes:
            # update edges
            for edge in (describe_edge(e) for e in outgoing):
                edges.append(edge)

            # describe node
            yield '\t"%s" [%s];\n' % (
                node,
                ",".join(
                    [
                        (cpatt % item)
                        for item in nodevisitor(node, data, outgoing, incoming).items()
                    ]
                ),
            )

            inside = inpackages.get(node)
            if inside is None:
                inside = inpackages[node] = set()
            ident = nodetoident[node]
            if ident is None:
                continue
            pkgnode = packageidents.get(ident[: ident.rfind(".")])
            if pkgnode is not None:
                inside.add(pkgnode)

        graph = []
        subgraphs = {}
        for key in packagenodes:
            subgraphs[key] = []

        while edges:
            edge, data, head, tail = edges.popleft()
            if ((head, tail)) in mainedges:
                continue
            mainedges.add((head, tail))
            tailpkgs = inpackages[tail]
            common = inpackages[head] & tailpkgs
            if not common and tailpkgs:
                usepkgs = sorted(tailpkgs)
                if len(usepkgs) != 1 or usepkgs[0] != tail:
                    edges.append((edge, data, head, usepkgs[0]))
                    edges.append((edge, "pkgref", usepkgs[-1], tail))
                    continue
            if common:
                common = common.pop()
                if tail == common:
                    edges.append((edge, data, tail, head))
                elif head == common:
                    subgraphs[common].append((edge, "pkgref", head, tail))
                else:
                    edges.append((edge, data, common, head))
                    edges.append((edge, data, common, tail))

            else:
                graph.append((edge, data, head, tail))

        def do_graph(edges, tabs):
            edgestr = tabs + '"%s" -> "%s" [%s];\n'
            # describe edge
            for (edge, data, head, tail) in edges:
                attribs = edgevisitor(edge, data, head, tail)
                yield edgestr % (
                    head,
                    tail,
                    ",".join([(cpatt % item) for item in attribs.items()]),
                )

        for g, edges in subgraphs.items():
            yield '\tsubgraph "cluster_%s" {\n' % (g,)
            yield '\t\tlabel="%s";\n' % (nodetoident[g],)
            for s in do_graph(edges, "\t\t"):
                yield s
            yield "\t}\n"

        for s in do_graph(graph, "\t"):
            yield s

        yield "}\n"

    def graphreport(self, fileobj=None, flatpackages=()):
        if fileobj is None:
            fileobj = sys.stdout
        fileobj.writelines(self.itergraphreport(flatpackages=flatpackages))

    def report(self):
        """Print a report to stdout, listing the found modules with their
        paths, as well as modules that are missing, or seem to be missing.
        """
        print()
        print("%-15s %-25s %s" % ("Class", "Name", "File"))
        print("%-15s %-25s %s" % ("-----", "----", "----"))
        for m in sorted(self.flatten(), key=lambda n: n.identifier):
            print("%-15s %-25s %s" % (type(m).__name__, m.identifier, m.filename or ""))

    def _replace_paths_in_code(self, co):
        new_filename = original_filename = os.path.normpath(co.co_filename)
        for f, r in self.replace_paths:
            f = os.path.join(f, "")
            r = os.path.join(r, "")
            if original_filename.startswith(f):
                new_filename = r + original_filename[len(f) :]  # noqa: E203
                break

        else:
            return co

        consts = list(co.co_consts)
        for i in range(len(consts)):
            if isinstance(consts[i], type(co)):
                consts[i] = self._replace_paths_in_code(consts[i])

        code_func = type(co)

        if hasattr(co, "co_posonlyargcount"):
            return code_func(
                co.co_argcount,
                co.co_posonlyargcount,
                co.co_kwonlyargcount,
                co.co_nlocals,
                co.co_stacksize,
                co.co_flags,
                co.co_code,
                tuple(consts),
                co.co_names,
                co.co_varnames,
                new_filename,
                co.co_name,
                co.co_firstlineno,
                co.co_lnotab,
                co.co_freevars,
                co.co_cellvars,
            )
        elif hasattr(co, "co_kwonlyargcount"):
            return code_func(
                co.co_argcount,
                co.co_kwonlyargcount,
                co.co_nlocals,
                co.co_stacksize,
                co.co_flags,
                co.co_code,
                tuple(consts),
                co.co_names,
                co.co_varnames,
                new_filename,
                co.co_name,
                co.co_firstlineno,
                co.co_lnotab,
                co.co_freevars,
                co.co_cellvars,
            )
        else:
            return code_func(
                co.co_argcount,
                co.co_nlocals,
                co.co_stacksize,
                co.co_flags,
                co.co_code,
                tuple(consts),
                co.co_names,
                co.co_varnames,
                new_filename,
                co.co_name,
                co.co_firstlineno,
                co.co_lnotab,
                co.co_freevars,
                co.co_cellvars,
            )
