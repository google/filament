# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
"""CodeTags.py - Support code tags and auto-generated/manipulated regions

Example 1:
    /* <py>
    import System
    random = System.Random()
    </py> */
    // <py>ints=10</py>
    // <py::lines('GENERATED')>'int MyInts[] = { %s };' % [random.Next() for n in range(ints)]</py>
    // GENERATED:BEGIN
    // GENERATED:END

Example 2:  Sort lines (case-insensitive)
    // <py::lines('SORTED')>sorted(lines, key=str.lower)</py>
    // SORTED:BEGIN
    These
    are
    some
    lines
    to
    be
    sorted
    // SORTED:END

For .rst files, use this syntax:
    .. <py::lines('SORTED')>sorted(lines, key=str.lower)</py::lines>
    .. SORTED:BEGIN
    The
    text
    .. SORTED:END

TODO:
    - Handle doc/location thing better (not dependent on app.current_doc and selection)
        Would be easier with some exposed anchor point system
"""
import sys

is_py3 = sys.version_info[0] == 3
if is_py3:
    basestring = (str, bytes)

import re
import hctdb_instrhelp


def SelectTaggedLines(tag, start, doc=None):
    "Select lines between tag:BEGIN and tag:END"
    if doc is None:
        doc = app.current_doc
    text = doc.text
    begin = text.find(tag + ":BEGIN", doc.get_selection_range()[1])
    if begin == -1:
        raise StandardError("Could not find tag %s:BEGIN" % tag)
    begin = text.find(native_endl, begin)
    end = text.find(tag + ":END", begin)
    if end == -1:
        raise StandardError("Could not find tag %s:END" % tag)
    end = text.rfind(native_endl, begin, end) + len(native_endl)
    doc.select(begin, end)


rxWhitespace = re.compile(r"\s+")


def GetIndent(text, pos):
    "Get indent for line in text at pos"
    end = text.find(native_endl, pos)
    begin = text.rfind(native_endl, 0, end)
    if begin == -1:
        begin = 0
    else:
        begin += len(native_endl)
    m = rxWhitespace.match(text, begin, end)
    if m:
        return m.group(0)
    return ""


def ReplaceTaggedLines(text_or_lines, tag="GENERATED_CODE", doc=None, tagrange=None):
    "Replace lines between tag:BEGIN and tag:END with text_or_lines, start searching forward from selection or end of tagrange"
    if doc is None:
        doc = app.current_doc
    if tagrange is None:
        tagrange = doc.get_selection_range()
    SelectTaggedLines(tag, tagrange[1], doc)
    text = doc.text
    begin, end = doc.get_selection_range()
    indent = GetIndent(text, begin)
    if isinstance(text_or_lines, basestring):
        text_or_lines = text_or_lines.splitlines()
    text_or_lines = [indent + line for line in text_or_lines]
    delta = ReplaceSelection(text_or_lines)
    doc.select(begin, end + delta)  # reselect replacement so search may continue


def StripIndent(lines, indent):
    "Remove indent from lines of text"

    def strip_indent(line):
        if line.startswith(indent):
            return line[len(indent) :]
        return line

    if isinstance(lines, basestring):
        lines = lines.splitlines()
    return map(strip_indent, lines)


def UpdateTaggedLines(fn, tag="GENERATED_CODE", doc=None, tagrange=None):
    "Call supplied function with list of lines found between tag:BEGIN and tag:END.  Replace with result."
    if doc is None:
        doc = app.current_doc
    if tagrange is None:
        tagrange = doc.get_selection_range()
    SelectTaggedLines(tag, tagrange[1], doc)
    begin, end = doc.get_selection_range()
    indent = GetIndent(doc.text, begin)
    lines = NormalizeForPython(doc.selected_text).splitlines()[1:]
    lines = StripIndent(lines, indent)
    lines = WrapFunctionForExceptionHandling(fn)(lines)
    if isinstance(lines, basestring):
        lines = lines.splitlines()
    lines = [indent + line for line in lines]
    delta = ReplaceSelection(lines)
    doc.select(begin, end + delta)  # reselect replacement so search may continue


def UpdateEachTaggedLine(fn, tag="GENERATED_CODE", doc=None, begin=None):
    "Call fn for each line of text between tag:BEGIN and tag:END"
    UpdateTaggedLines(lambda lines: map(fn, lines), tag, doc, begin)


def _make_transform_lines(tag):
    """Makes an operation for tag processing
    The 'lines' operation will select lines with your tag and execute your code with a
    local variable 'lines', the result of which is joined back together into text
    to replace the current selection:
    <example_py::lines('COMMENT_THIS')>['// %s' % line for line in lines]</example_py>
    """

    def _transform(code, doc=app.current_doc):
        def _transformer(lines):
            return eval(code)

        return UpdateTaggedLines(_transformer, tag, doc=doc)

    return _transform


def _make_output(arg):
    def _print_fn(expression_text, doc=app.current_doc):
        print("%s = %s" % (expression_text, arg))

    return _print_fn


def _make_eval(arg):
    def _eval(expression_text, doc=app.current_doc):
        print(arg)

    return _eval


PyOperations = {
    "lines": _make_transform_lines,
    "output": _make_output,
    "eval": _make_eval,
}
rxPyTag = re.compile(r"\<py(::(\w+)(\((.*?)\))?(/)?)?\>")


def FindPyTag(doc, begin=0):
    text = doc.text
    _operation_function_ = None
    tag_code = ""
    end = -1
    while begin != -1:
        begin = text.find("<py", begin)
        if begin != -1:
            m = rxPyTag.match(text, begin)
            if m:
                end = m.end()
                tag_code = tag = m.group(2)
                if tag:
                    try:
                        _operation_function_ = PyOperations[tag]
                    except KeyError:
                        print('Unknown tag "%s"' % tag)
                        raise
                    if m.group(3):
                        tag_code = "(_operation_function_)%s" % m.group(3)
                        try:
                            _operation_function_ = eval(
                                tag_code,
                                globals(),
                                {"_operation_function_": _operation_function_},
                            )
                        except:
                            print(
                                '%s(%d): Exception during eval("%s")'
                                % (
                                    doc.name,
                                    text.count(native_endl, 0, begin) + 1,
                                    tag_code,
                                )
                            )
                            raise
                        tag_code = tag_code.replace("(_operation_function_)", tag)
                else:
                    tag = ""
                if m.group(
                    5
                ):  # we have form: <example_py[...]/>          # whole tag selected currently
                    if m.group(4):  # we have <example_py::something(...)/>
                        begin, end = m.span(4)  # Select the arguments to 'something'
                    break
                begin = end
                end = text.find("</py>", end)
                if end == -1:
                    print(
                        "%s(%d): Error: Could not find matching </py>"
                        % (doc.name, text.count(native_endl, 0, begin) + 1)
                    )
                    begin = -1
                break
            else:
                # "<py" was the beginning of something else, keep searching:
                begin += 3
    return begin, end, _operation_function_, tag_code


def ExecuteNextPyTag(doc, begin=0):
    begin, end, op, tag_code = FindPyTag(doc, begin)
    if begin != -1 and end != -1:
        indent = GetIndent(doc.text, begin)
        code = doc.text[begin:end]
        code = NormalizeForPython(code)
        code = "\n".join(StripIndent(code, indent))
        doc.select(begin, end)
        if op:
            result = None
            try:
                result = op(code, doc=doc)
            except:
                print(
                    '%s(%d): Exception during eval("%s")'
                    % (doc.name, doc.text.count(native_endl, 0, begin) + 1, code)
                )
                raise
            if result is not None:
                print(result)
        else:
            try:
                exec(code, globals())
            except:
                print(
                    "%s(%d): Exception executing code"
                    % (doc.name, doc.text.count(native_endl, 0, begin) + 1)
                )
                raise
    return end


def ExecutePyTags(doc=None):
    if doc is None:
        doc = app.current_doc
    begin = 0
    while begin != -1:
        begin = ExecuteNextPyTag(doc, begin)


################################################
################################################
# Testing Section


class Test(object):
    "Provides testing functionality, emulating dependencies"

    class TestApp(object):
        pass

    class TestDoc(object):
        def __repr__(self):
            return "<TestDoc: %s=%s, %s>" % (
                repr(self._selection),
                repr(self.text[self._selection[0] : self._selection[1]]),
                repr(self.text),
            )

        def __init__(self, text, selection=None):
            self.name = "TestDoc"
            self.text = text
            self._selection = (0, 0)

        def get_selected_text(self):
            return self.text[self._selection[0] : self._selection[1]]

        def select(self, start, end):
            self._selection = start, end

        def get_selection_range(self):
            return self._selection

        def set_selected_text(self, text):
            begin, end = self.get_selection_range()
            self.text = self.text[:begin] + text + self.text[end:]
            # self._selection = (0,0)

        selected_text = property(get_selected_text, set_selected_text)

    def __init__(self, before, expected):
        self.before, self.expected = before, expected
        self.after = ""

    def test(self, selection=None):
        doc = Test.TestDoc(self.before, selection)
        app.current_doc = doc
        try:
            ExecutePyTags(doc)
            self.after = doc.text
            return 0
        except:
            import sys

            _stderr = sys.stderr
            sys.stderr = self
            sys.excepthook(*sys.exc_info())
            sys.stderr = _stderr
            return 1

    def write(self, text):
        self.after += text

    def verify(self):
        "Returns nothing if passed, or failure message if failed."
        if self.after == self.expected:
            return
        return (
            "==================== Verify failed!\n--- Before:%s+++ After:%s=== Expected:%s====================\n"
            % (self.before, self.after, self.expected)
        )

    @staticmethod
    def NormalizeForVSText(text_or_lines):
        if isinstance(text_or_lines, basestring):
            text = str(text_or_lines).replace(native_endl, "\n")
        else:
            text = "\n".join(
                [str(item).replace(native_endl, "\n") for item in text_or_lines]
            )
        return text.replace("\n", native_endl)

    @staticmethod
    def NormalizeForPython(text):
        return str(text).replace(native_endl, "\n").replace("\r", "\n")

    @staticmethod
    def TerminateTextForVS(text):
        if text.endswith(native_endl):
            return text
        return text + native_endl

    @staticmethod
    def WrapFunctionForExceptionHandling(fn):
        import sys

        def wrapperfn(*args, **kwargs):
            try:
                return fn(*args, **kwargs)
            except:
                sys.excepthook(*sys.exc_info())
                sys.stderr.write("\n")
                raise

        return wrapperfn

    @staticmethod
    def ReplaceSelection(text_or_lines, doc=None):
        if doc is None:
            doc = app.current_doc
        original = doc.selected_text
        result = TerminateTextForVS(NormalizeForVSText(text_or_lines))
        if original.startswith(native_endl) and not result.startswith(native_endl):
            result = native_endl + result
        doc.selected_text = result
        return len(result) - len(original)

    @staticmethod
    def SetDependencies():
        global app
        global NormalizeForVSText, NormalizeForPython, TerminateTextForVS, WrapFunctionForExceptionHandling, ReplaceSelection
        if not hasattr(globals(), "app"):
            (
                NormalizeForVSText,
                NormalizeForPython,
                TerminateTextForVS,
                WrapFunctionForExceptionHandling,
                ReplaceSelection,
            ) = (
                Test.NormalizeForVSText,
                Test.NormalizeForPython,
                Test.TerminateTextForVS,
                Test.WrapFunctionForExceptionHandling,
                Test.ReplaceSelection,
            )
            app = Test.TestApp()


_tests = r"""
<py>app.current_doc.text = '\nfoo\n'</py>
--------------------
foo
====================
not indented
    // <py::lines('COMMENT_THIS')>['// %s' % line for line in lines]</py>
    before section
    // COMMENT_THIS:BEGIN
    lines to comment
    // COMMENT_THIS:END
after comment
--------------------
not indented
    // <py::lines('COMMENT_THIS')>['// %s' % line for line in lines]</py>
    before section
    // COMMENT_THIS:BEGIN
    // lines to comment
    // COMMENT_THIS:END
after comment
====================

--------------------

====================

--------------------

""".split(
    "===================="
)

_tests = [text.split("--------------------") for text in _tests]
if not is_py3:
    del text


def test_module(tests=_tests):
    global native_endl
    Test.SetDependencies()
    native_endl = "\n"
    failed = 0
    print("Testing CodeTags.py")
    for before, expected in tests:
        test = Test(before, expected)
        test.test()
        result = test.verify()
        if result:
            print(result)
            failed += 1
    if failed:
        print("%d Test(s) Failed!" % failed)
    else:
        print("All Tests Passed!")
    return failed


def _TraceFunctions():
    global _tablevel
    _tablevel = 0

    def _trace(fn):
        def _wrapper(*args, **kwargs):
            global _tablevel
            _tablevel += 1
            _args = map(repr, args)
            _args += ["%s=%s" % (key, repr(value)) for key, value in kwargs.items()]
            print("%s%s(%s)" % (_tablevel * "  ", fn.__name__, ", ".join(_args)))
            result = fn(*args, **kwargs)
            if result:
                print("%s= %s" % (_tablevel * "  ", repr(result)))
            _tablevel -= 1
            return result

        return _wrapper

    import types

    for key, value in globals().items():
        if key[:1] != "_":
            if type(value) == types.FunctionType:
                globals()[key] = _trace(value)
            elif type(value) == types.ClassType:
                for key in dir(value):
                    member = getattr(value, key)
                    if type(member) == types.MethodType:
                        setattr(value, key, _trace(member))


################################################
################################################
# For running from the command-line


def usage():
    print(
        """CodeTags.py - Templatize and generate code with python tags.

Usage:
CodeTags.py <input file> [<output file>]
    <input file>        - file with tags to expand
    <output file>       - optional output file (expand in place if unspecified)

"""
    )
    return 1


def main(argv, newline=None):
    trace = False
    test = False
    args = []
    for arg in argv:
        if arg[0] in "/-":
            opt = arg[1:].lower()
            if opt == "trace":
                trace = True
                continue
            elif opt == "test":
                test = True
                continue
        args.append(arg)

    if trace:
        _TraceFunctions()

    if test:
        return test_module()

    if len(args) == 1:
        # do in-place expansion of code tags
        args.append(args[0])

    result = 0
    if len(args) == 2:
        global native_endl
        Test.SetDependencies()
        native_endl = "\n"
        with open(args[0], "rt") as f:
            # Use test harness to do work, but we don't care about the comparison
            # with expected, since we're not actually in testing mode:
            test = Test(f.read(), "expected not used.")
            result = test.test()
        if result == 0:
            with open(args[1], "wt", newline=newline) as f:
                f.write(test.after)
        else:
            sys.stderr.write(test.after)
    else:
        result = usage()

    return result


if __name__ == "__main__":
    import sys

    exit(main(sys.argv[1:]))
