#!/usr/bin/env pipenv run python

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

"""

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CURRENT_DIR = os.getcwd()
ROOT_DIR = '../../../'
OUTPUT_DIR = ROOT_DIR + 'docs/webgl/'
SERVER_DIR = OUTPUT_DIR + '..'
BUILD_DIR = ROOT_DIR + 'out/cmake-webgl-release/'
EXEC_NAME = os.path.basename(sys.argv[0])
SCRIPT_NAME = os.path.basename(__file__)
PREAMBLE = """
## Literate programming

The markdown source for this tutorial is not only used to generate this
website, it's also used to generate the JavaScript for the above demo.
We use a small Python script for weaving (generating HTML) and tangling
(generating JS). In the code samples, you'll often see
`// TODO: <some task>`. These are special markers that get replaced by
subsequent code blocks.
"""

# The pipenv command in the shebang needs a certain working directory.
if EXEC_NAME == SCRIPT_NAME and SCRIPT_DIR != CURRENT_DIR:
    relative_script_path = os.path.dirname(__file__)
    quit(f"Please run script from {relative_script_path}")

import mistletoe
import re
import shutil
import jsbeautifier

from itertools import chain
from mistletoe import HTMLRenderer, BaseRenderer
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
    with open(f'tutorial_{name}.md', 'r') as fin:
        markdown = fin.read()
        style = 'style="width:100%;height:200px;border:none"'
        if name == 'triangle':
            markdown = PREAMBLE + markdown
        iframe = f"<iframe {style} src='demo_{name}.html'></iframe>\n\n"
        markdown = iframe + markdown
        rendered = mistletoe.markdown(markdown, PygmentsRenderer)
    template = open('tutorial_template.html').read()
    rendered = template.replace('$BODY', rendered)
    outfile = os.path.join(OUTPUT_DIR, f'tutorial_{name}.html')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

def generate_demo_html(name):
    template = open('demo_template.html').read()
    rendered = template.replace('$SCRIPT', f'tutorial_{name}.js')
    outfile = os.path.join(OUTPUT_DIR, f'demo_{name}.html')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

def tangle(name):
    with open(f'tutorial_{name}.md', 'r') as fin:
        rendered = mistletoe.markdown(fin, JsRenderer)
    outfile = os.path.join(OUTPUT_DIR, f'tutorial_{name}.js')
    with open(outfile, 'w') as fout:
        fout.write(rendered)

def copy_demo_filamat(srcname, dstname):
    matsrc = 'samples/web/public/material'
    matsrc = os.path.join(BUILD_DIR, matsrc, srcname + '.filamat')
    matdst = os.path.join(OUTPUT_DIR, dstname + '.filamat')
    shutil.copyfile(matsrc, matdst)

def copy_built_file(src):
    src = os.path.join(BUILD_DIR, src)
    dst = os.path.join(OUTPUT_DIR, os.path.basename(src))
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
    Handler.directory = SERVER_DIR
    os.chdir(SERVER_DIR)
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", 8000), Handler) as httpd:
        httpd.allow_reuse_address = True
        httpd.serve_forever()

if __name__ == "__main__":
    for name in ["triangle", "redball"]:
        weave(name)
        tangle(name)
        generate_demo_html(name)
    copy_src_file('libs/filamentjs/docs/main.css')
    copy_src_file('third_party/gl-matrix/gl-matrix-min.js')
    copy_built_file('libs/filamentjs/filament.js')
    copy_built_file('libs/filamentjs/filament.wasm')
    copy_built_file('samples/web/public/pillars_2k/pillars_2k_skybox.ktx')
    copy_built_file('samples/web/public/pillars_2k/pillars_2k_ibl.ktx')
    copy_demo_filamat('bakedColor', 'triangle')
    copy_demo_filamat('sandboxLit', 'plastic')
    if len(sys.argv) > 1 and sys.argv[1] == 'serve':
        spawn_local_server()
