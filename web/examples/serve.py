#!/usr/bin/env python3
#
# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
import http.server
import socketserver
from pathlib import Path

try:
    import mistletoe
    from mistletoe.html_renderer import HTMLRenderer
except ImportError:
    print("mistletoe is not installed. Please install it using `pip install mistletoe`.", file=sys.stderr)
    sys.exit(1)

SCRIPT_DIR = Path(__file__).parent.resolve()
ROOT_DIR = SCRIPT_DIR.parent.parent

def find_examples_dir():
    # search for filament.js in out/
    out_dir = ROOT_DIR / 'out'
    if out_dir.exists():
        for root, dirs, files in os.walk(out_dir):
            if 'filament.js' in files:
                return root
    return None

examples_dir = find_examples_dir()

if not examples_dir:
    print("Could not find the built examples directory in out/. Please build the webgl target first.", file=sys.stderr)
    # fallback to web/examples for testing, though it won't have the built assets
    examples_dir = str(SCRIPT_DIR)

# Read template
HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="en"><head>
<link href="https://google.github.io/filament/favicon.png" rel="icon" type="image/x-icon" />
<link href="https://fonts.googleapis.com/css?family=Open+Sans:300,400,600,700|Tangerine:700|Inconsolata" rel="stylesheet">
<link href="../main.css" rel="stylesheet" type="text/css">
</head>
<body class="verbiage">
$BODY
</body>
</html>"""

class CustomHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=examples_dir, **kwargs)

    def do_GET(self):
        # Serve main index if root
        if self.path == '/' or self.path == '/index.html':
            self.serve_index()
            return
            
        # If it's a markdown file, render it
        if self.path.endswith('.md'):
            self.serve_markdown()
            return

        # Otherwise serve normally
        super().do_GET()

    def serve_index(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

        # Generate a list of all samples and tutorials
        html = ["<!DOCTYPE html><html><head><title>Filament Examples</title>"]
        html.append("<style>body { font-family: sans-serif; max-width: 800px; margin: 40px auto; padding: 0 20px; line-height: 1.6; } ")
        html.append("h1 { border-bottom: 1px solid #eee; padding-bottom: 10px; } </style>")
        html.append("</head><body><h1>Filament Examples and Tutorials</h1><ul>")
        
        # Look for subdirectories in examples_dir
        for item in sorted(os.listdir(examples_dir)):
            item_path = os.path.join(examples_dir, item)
            if os.path.isdir(item_path):
                # check for .html or .md
                entry_point = None
                for file in os.listdir(item_path):
                    if file.endswith('.html') or file.endswith('.md'):
                        # Prefer markdown if multiple exist (tutorials have both usually)
                        if file.endswith('.md'):
                            entry_point = f"{item}/{file}"
                            break
                        entry_point = f"{item}/{file}"
                
                if entry_point:
                    html.append(f"<li><a href='{entry_point}'>{item}</a></li>")
        
        html.append("</ul></body></html>")
        self.wfile.write("".join(html).encode('utf-8'))

    def serve_markdown(self):
        # path looks like /triangle/triangle.md
        file_path = os.path.join(examples_dir, self.path.lstrip('/'))
        if not os.path.exists(file_path):
            self.send_error(404, "File not found")
            return
            
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        
        with open(file_path, 'r') as f:
            md_content = f.read()
            
        import re
        blocks = []
        def repl(match):
            blocks.append(match.group(0))
            return f'<!-- BLOCK_{len(blocks)-1} -->'
            
        md_content = re.sub(r'<script.*?</script>', repl, md_content, flags=re.DOTALL)
        md_content = re.sub(r'<div class="demo_frame".*?</div>', repl, md_content, flags=re.DOTALL)
            
        rendered_html = mistletoe.markdown(md_content, HTMLRenderer)
        
        for i, block in enumerate(blocks):
            rendered_html = rendered_html.replace(f'<!-- BLOCK_{i} -->', block)
        
        # fix relative links, since we are serving at /dir/file.md but resources are at /dir/
        # base url should just be the directory of the markdown
        dir_name = os.path.dirname(self.path.lstrip('/'))
        base_tag = f'<base href="/{dir_name}/" />'
        
        # insert base tag into HEAD
        if '<head>' in HTML_TEMPLATE:
            final_html = HTML_TEMPLATE.replace('<head>', f'<head>\n{base_tag}')
        else:
            final_html = HTML_TEMPLATE
            
        final_html = final_html.replace('$BODY', rendered_html)
        
        self.wfile.write(final_html.encode('utf-8'))

if __name__ == '__main__':
    port = 8000
    print(f"Serving at http://localhost:{port}")
    print(f"Serving directory: {examples_dir}")
    with socketserver.TCPServer(("", port), CustomHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
