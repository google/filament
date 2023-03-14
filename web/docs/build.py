#!/usr/bin/env python3

# To run this script, I recommend using pipenv to create an isolated Python environment with the
# correct packages so that you do not interfere with other Python projects in your system.
# After installing pipenv, run the following commands from the current folder:
#
#  pipenv --python /usr/local/opt/python@3.9/bin/python3
#  pipenv install
#  pipenv shell
#  ./build.py

"""Converts markdown into HTML and extracts JavaScript code blocks.

For each markdown file, this script invokes mistletoe twice: once
to generate HTML, and once to extract JavaScript code blocks.

Literate code fragments are marked by "// TODO: <name>". These get
replaced according to extra properties on the code fences.

For example, the following snippet would replace "// TODO: create
wombat".

```js {fragment="create wombat"}
var wombat = new Wombat();
```

This script also generates reference documentation by extracting
doxygen style comments of the form:

    /// [name] ::tags:: brief description
    /// detailed description

Where "tags" consists of one or more of the following words:
   class, core, method, argument, retval, function

These docstrings are used to build a JSON hierarchy where the roots
are classes, free functions, and enums. The JSON is then traversed to
generate a markdown string, which then produces HTML.
"""

import glob
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
ROOT_DIR = SCRIPT_DIR + '../../'
OUTPUT_DIR = ROOT_DIR + 'docs/webgl/'
ENABLE_EMBEDDED_DEMO = True
BUILD_DIR = ROOT_DIR + 'out/cmake-webgl-release/'
TOOLS_DIR = ROOT_DIR + 'out/cmake-release/tools/'

TUTORIAL_PREAMBLE = """
## Literate programming

The markdown source for this tutorial is not only used to generate this
web page, it's also used to generate the JavaScript for the above demo.
We use a small Python script for weaving (generating HTML) and tangling
(generating JS). In the code samples, you'll often see
`// TODO: <some task>`. These are special markers that get replaced by
subsequent code blocks.
"""

REFERENCE_PREAMBLE = """
All type names in this reference belong to the Filament namespace.
For example, **[init](#init)** actually refers to **Filament.init**.
""".strip().replace("\n", " ")

import argparse
import jsbeautifier
import mistletoe
import pygments
import re
import shutil

from itertools import chain
from mistletoe import HTMLRenderer
from mistletoe.base_renderer import BaseRenderer
from mistletoe import span_token
from mistletoe.block_token import CodeFence as CF
from pygments import highlight
from pygments.formatters.html import HtmlFormatter
from pygments.lexers import get_lexer_by_name as get_lexer
from pygments.styles import get_style_by_name as get_style

class PygmentsRenderer(HTMLRenderer):
    "Extends HTMLRenderer by adding syntax highlighting"
    formatter = HtmlFormatter()
    formatter.noclasses = True
    def __init__(self, *extras, style='default'):
        super().__init__(*extras)
        self.formatter.style = get_style(style)
    def render_block_code(self, token):
        code = token.children[0].content
        lexer = get_lexer(token.language)
        return highlight(code, lexer, self.formatter)

class CodeFence(CF):
    "Extends the standard CodeFence with optional properties"
    ppattern = r' *(\{ *(.+) *\= *\"(.+)\" *\})?'
    pattern = re.compile(r'( {0,3})((?:`|~){3,}) *(\S*)' + ppattern)
    _open_info = None
    def __init__(self, match):
        lines, open_info = match
        self.language = span_token.EscapeSequence.strip(open_info[2])
        self.children = (span_token.RawText(''.join(lines)),)
        self.properties = open_info[3]

    @classmethod
    def start(cls, line):
        match_obj = cls.pattern.match(line)
        if not match_obj:
            return False
        prepend, leader, lang, props = match_obj.groups()[:4]
        if leader[0] in lang or leader[0] in line[match_obj.end():]:
            return False
        cls._open_info = len(prepend), leader, lang, props
        return True

class JsRenderer(BaseRenderer):
    ppattern = re.compile(CodeFence.ppattern)
    def __init__(self, *extras):
        self.root = ''
        self.fragments = {}
        super().__init__(*chain((CodeFence,), extras))
    def __exit__(self, *args): super().__exit__(*args)
    def render_strong(self, token): return ''
    def render_emphasis(self, token): return ''
    def render_inline_code(self, token): return ''
    def render_strikethrough(self, token): return ''
    def render_image(self, token): return ''
    def render_link(self, token): return ''
    def render_auto_link(self, token): return ''
    def render_escape_sequence(self, token): return ''
    def render_raw_text(self, token): return ''
    def render_heading(self, token): return ''
    def render_quote(self, token): return ''
    def render_paragraph(self, token): return ''
    def render_list(self, token): return ''
    def render_list_item(self, token): return ''
    def render_document(self, token):
        for child in token.children:
            self.render(child)
        label_pattern = re.compile(r'\s*// TODO:\ (.+)')
        result = ''
        for line in self.root.split('\n'):
            m = label_pattern.match(line)
            label = m.group(1) if m else None
            if label in self.fragments:
                result += self.fragments[label]
            else:
                result += line + '\n'
        opts = jsbeautifier.default_options()
        opts.indent_size = 2
        opts.end_with_newline = True
        opts.preserve_newlines = False
        opts.break_chained_methods = True
        opts.wrap_line_length = 100
        return jsbeautifier.beautify(result, opts)

    def render_code_fence(self, token):
        if token.language != 'js' or not token.properties:
            return ''
        match = JsRenderer.ppattern.match(token.properties)
        key = match.groups()[2]
        if key == 'root':
            self.root = token.children[0].content
            return
        fragments = self.fragments
        val = token.children[0].content
        fragments[key] = fragments.get(key, '') + val
        return ''

def weave(name):
    with open(SCRIPT_DIR + f'tutorial_{name}.md', 'r') as fin:
        markdown = fin.read()
        if ENABLE_EMBEDDED_DEMO:
            if name == 'triangle':
                markdown = TUTORIAL_PREAMBLE + markdown
            markdown = '<div class="demo_frame">' + \
                f'<iframe src="demo_{name}.html"></iframe>' + \
                f'<a href="demo_{name}.html">&#x1F517;</a>' + \
                '</div>\n' + markdown
        rendered = mistletoe.markdown(markdown, PygmentsRenderer)
    template = open(SCRIPT_DIR + 'tutorial_template.html').read()
    rendered = template.replace('$BODY', rendered)
    outfile = os.path.join(OUTPUT_DIR, f'tutorial_{name}.html')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

def generate_demo_html(name):
    template = open(SCRIPT_DIR + 'demo_template.html').read()
    rendered = template.replace('$SCRIPT', f'tutorial_{name}.js')
    outfile = os.path.join(OUTPUT_DIR, f'demo_{name}.html')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

def tangle(name):
    with open(SCRIPT_DIR + f'tutorial_{name}.md', 'r') as fin:
        rendered = mistletoe.markdown(fin, JsRenderer)
    outfile = os.path.join(OUTPUT_DIR, f'tutorial_{name}.js')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

def build_filamat(name):
    matsrc = SCRIPT_DIR + name + '.mat'
    matdst = os.path.join(OUTPUT_DIR, name + '.filamat')
    flags = '-a opengl -p mobile'
    matc_exec = os.path.join(TOOLS_DIR, 'matc/matc')
    retval = os.system(f"{matc_exec} {flags} -o {matdst} {matsrc}")
    if retval != 0:
        exit(retval)

def copy_built_file(pattern, destfolder=None):
    outdir = OUTPUT_DIR
    if destfolder:
        outdir = os.path.join(outdir, destfolder)
    if not os.path.exists(outdir):
         os.mkdir(outdir)
    pattern = os.path.join(BUILD_DIR, pattern)
    for src in glob.glob(pattern):
        dst = os.path.join(outdir, os.path.basename(src))
        shutil.copyfile(src, dst)

def copy_src_file(src):
    src = os.path.join(ROOT_DIR, src)
    dst = os.path.join(OUTPUT_DIR, os.path.basename(src))
    shutil.copyfile(src, dst)

def spawn_local_server():
    import http.server
    import socketserver
    Handler = http.server.SimpleHTTPRequestHandler
    Handler.extensions_map.update({ '.wasm': 'application/wasm' })
    Handler.directory = OUTPUT_DIR
    os.chdir(OUTPUT_DIR)
    socketserver.TCPServer.allow_reuse_address = True
    port = 8000
    print(f"serving docs at http://localhost:{port}")
    with socketserver.TCPServer(("", port), Handler) as httpd:
        httpd.allow_reuse_address = True
        httpd.serve_forever()

def expand_refs(comment_line):
    """Adds hrefs to markdown links that do not already have them; e.g.
    expands [Foo] to [Foo](#Foo) but leaves [Foo](https://foo) alone.
    """
    result = comment_line
    result = re.sub(r"\[(\S+)\]([^(])", r"[\1](#\1)\2", result)
    result = re.sub(r"\[(\S+)\]$", r"[\1](#\1)", result)
    return result

def gather_docstrings(paths):
    """Given a list of paths to JS and CPP files, builds a JSON tree of
    type descriptions."""
    result = []
    stack = [{"tags": ["root"]}]
    previous = stack[0]
    docline = re.compile(r' */// (.+)')
    enumline = re.compile(r' *enum_.*\"(.*)\"')
    enumvalue = re.compile(r' *\.value\("(.*)\"')
    tagged = re.compile(r'(\S+)? *::(.+):: *(.*)')
    lines = []
    enumerating = False
    current_enumeration = None
    for path in paths:
        lines += open(path).readlines()
    for line in lines:
        match_obj = docline.match(line)
        if not match_obj:
            match_obj = enumline.match(line)
            if match_obj:
                result.append({
                    "name": match_obj.groups()[0],
                    "tags": "enum",
                    "brief": "",
                    "detail": None,
                    "children": [],
                })
                current_enumeration = result[-1]["children"]
                enumerating = True
                continue
            match_obj = enumvalue.match(line)
            if match_obj:
                val = match_obj.groups()[0]
                current_enumeration.append(val)
            continue
        ln = match_obj.groups()[0]
        match_obj = tagged.match(ln)
        if match_obj:
            name = match_obj.groups()[0]
            tags = match_obj.groups()[1].split()
            brief = match_obj.groups()[2]
            entity = {
                "name": name,
                "tags": tags,
                "brief": brief,
                "detail": None,
                "children": []
            }

            # Check if this is continuation of a previous type.
            if brief == '':
              for existing_type in result:
                if existing_type['name'] == name:
                  entity = existing_type
                  result.remove(existing_type)
                  break

            top = stack[-1]["tags"]
            if 'root' in top:
                result.append(entity)
                stack.append(entity)
            elif 'class' in tags or 'function' in tags:
                result.append(entity)
                stack[-1] = entity
            elif 'method' in tags and 'class' in top:
                stack[-1]["children"].append(entity)
                stack.append(entity)
            elif 'method' in tags:
                stack[-2]["children"].append(entity)
                stack[-1] = entity
            elif 'retval' in tags or 'argument' in tags:
                stack[-1]["children"].append(entity)
            previous = entity
        else:
            brief = previous["brief"]
            detail = previous["detail"]
            if brief.endswith("\\"):
                previous["brief"] = brief[:-1] + ln
            elif not detail:
                previous["detail"] = ln
            else:
                previous["detail"] += "\n" + ln
    return result

def generate_class_reference(entity):
    name = entity["name"]
    brief, detail = entity["brief"], entity["detail"]
    brief = expand_refs(brief)
    result = f"\n## class <a id='{name}' href='#{name}'>{name}</a>\n\n"
    result += brief + "\n\n"
    entity["children"].sort(key = lambda t: t["name"])
    for method in entity["children"]:
        result += "- **"
        if "static" in method["tags"]:
            # Write the class name before the method name.
            result += name + "."
        else:
            # Instances are lowercase by convention.
            result += name[0].lower() + name[1:] + "."
        mname = method.get("name")
        assert mname, f"Missing method name on {name}"
        args = []
        for child in method["children"]:
            if "argument" in child["tags"]:
                cname = child.get("name")
                assert cname, f"Missing arg name on {mname}"
                args.append(cname)
        result += f"{mname}(" + ", ".join(args) + ")**\n"
        if method["brief"] != "":
            result += "  - " + method["brief"] + "\n"
        for child in method["children"]:
            argname = child["name"]
            argbrief = expand_refs(child["brief"])
            if "argument" in child["tags"]:
                result += f"  - *{argname}* {argbrief}\n"
            elif "retval" in child["tags"]:
                result += f"  - *returns* {argbrief}\n"
    result += "\n"
    if detail:
        result += expand_refs(detail) + "\n"
    return result

def generate_function_reference(entity):
    name = entity["name"]
    brief, detail = entity["brief"], entity["detail"]
    brief = expand_refs(brief)
    result = f"\n## function <a id='{name}' href='#{name}'>{name}</a>("
    args = []
    for child in entity["children"]:
        if "argument" in child["tags"]:
            args.append(child["name"])
    result += ", ".join(args)
    result += ")\n\n"
    result += brief + "\n\n"
    for child in entity["children"]:
        argname = child["name"]
        argbrief = expand_refs(child["brief"])
        if "argument" in child["tags"]:
            result += f"- **{argname}**\n  - {argbrief}\n"
        else:
            result += f"- **returns**\n  - {argbrief}\n"
    result += "\n"
    if detail:
        result += expand_refs(detail) + "\n"
    return result

def generate_enum_reference(entity):
    name = entity["name"]
    result = f"\n## enum <a id='{name}' href='#{name}'>{name}</a>\n\n"
    for valname in entity["children"]:
        result += f"- {valname}\n"
    result += "\n"
    return result

def build_reference_markdown(doctree):
    result = REFERENCE_PREAMBLE
    # Generate table of contents
    result += """
### <a id="classes" href="#classes">Classes</a>
|     |     |
| --- | --- |
"""
    doctree.sort(key = lambda t: t['name'])
    for entity in doctree:
        name = entity["name"]
        brief = expand_refs(entity["brief"])
        if "class" in entity["tags"]:
            result += f"| [{name}](#{name}) | {brief} |\n"

    result += """
### <a id="functions" href="#functions">Free Functions</a>
|     |     |
| --- | --- |
"""
    for entity in doctree:
        name = entity["name"]
        brief = expand_refs(entity["brief"])
        if "function" in entity["tags"]:
            result += f"| [{name}](#{name}) | {brief} |\n"
    result += """
### <a id="enums" href="#enums">Enumerations</a>
|     |     |
| --- | --- |
"""
    for entity in doctree:
        name = entity["name"]
        brief = expand_refs(entity["brief"])
        if "enum" in entity["tags"]:
            result += f"| [{name}](#{name}) | {brief} |\n"
    result += "\n<br>\n"
    # Generate actual reference
    for entity in doctree:
        if "class" in entity["tags"]:
            result += "\n<div class='classdoc'>\n"
            result += generate_class_reference(entity)
            result += "\n</div>\n"
    for entity in doctree:
        if "function" in entity["tags"]:
            result += "\n<div class='funcdoc'>\n"
            result += generate_function_reference(entity)
            result += "\n</div>\n"
    for entity in doctree:
        if "enum" in entity["tags"]:
            result += "\n<div class='enumdoc'>\n"
            result += generate_enum_reference(entity)
            result += "\n</div>\n"
    return result

def build_api_reference():
    doctree = gather_docstrings([
        ROOT_DIR + 'web/filament-js/jsbindings.cpp',
        ROOT_DIR + 'web/filament-js/jsenums.cpp',
        ROOT_DIR + 'web/filament-js/utilities.js',
        ROOT_DIR + 'web/filament-js/wasmloader.js',
        ROOT_DIR + 'web/filament-js/extensions.js',
    ])
    markdown = build_reference_markdown(doctree)
    rendered = mistletoe.markdown(markdown, PygmentsRenderer)
    template = open(SCRIPT_DIR + 'ref_template.html').read()
    rendered = template.replace('$BODY', rendered)
    outfile = os.path.join(OUTPUT_DIR, f'reference.html')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__,
            formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-d", "--disable-demo",
            help="omit the embedded WebGL demo",
            action="store_true")
    parser.add_argument("-s", "--server",
            help="start small server in output folder",
            action="store_true")
    parser.add_argument("-b", "--build-folder", type=str,
            default=BUILD_DIR,
            help="set the cmake webgl build folder")
    parser.add_argument("-t", "--tools-folder", type=str,
            default=TOOLS_DIR,
            help="set the cmake host build folder for tools")
    parser.add_argument("-o", "--output-folder", type=str,
            default=OUTPUT_DIR,
            help="set the output folder")
    args = parser.parse_args()

    BUILD_DIR = args.build_folder
    TOOLS_DIR = args.tools_folder
    OUTPUT_DIR = args.output_folder
    ENABLE_EMBEDDED_DEMO = not args.disable_demo
    os.makedirs(os.path.realpath(OUTPUT_DIR), exist_ok=True)

    for name in open(SCRIPT_DIR + 'tutorials.txt').read().split():
        weave(name)
        tangle(name)
        generate_demo_html(name)

    copy_src_file(ROOT_DIR + 'web/docs/main.css')
    copy_src_file(ROOT_DIR + 'third_party/gl-matrix/gl-matrix-min.js')

    copy_built_file('web/filament-js/filament.js')
    copy_built_file('web/filament-js/filament.wasm')

    build_filamat('triangle')
    build_filamat('plastic')
    build_filamat('textured')

    build_api_reference()

    # Copy resources from "samples" to "docs"
    copy_built_file('web/samples/helmet.html')

    copy_built_file('web/samples/parquet.html')
    copy_built_file('web/samples/parquet.filamat')

    copy_built_file('web/samples/suzanne.html')
    copy_built_file('web/samples/suzanne.filamesh')

    copy_built_file('web/samples/metallic*.ktx')
    copy_built_file('web/samples/normal*.ktx')
    copy_built_file('web/samples/roughness*.ktx')
    copy_built_file('web/samples/ao*.ktx')
    copy_built_file('web/samples/albedo*.ktx')

    copy_built_file('web/samples/metallic*.ktx2')
    copy_built_file('web/samples/normal*.ktx2')
    copy_built_file('web/samples/roughness*.ktx2')
    copy_built_file('web/samples/ao*.ktx2')
    copy_built_file('web/samples/albedo*.ktx2')

    copy_built_file('web/samples/default_env/default_env*.ktx', 'default_env')

    # TODO: We do not normally build the following env maps so we should update the tutorials.
    # copy_built_file('web/samples/pillars_2k/pillars_2k_*.ktx', 'pillars_2k')
    # copy_built_file('web/samples/venetian_crossroads_2k/venetian_crossroads*.ktx', 'venetian_crossroads_2k')

    if args.server:
        spawn_local_server()
