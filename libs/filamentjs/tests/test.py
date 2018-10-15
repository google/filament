#!/usr/bin/env python3

import http.server
import socketserver
import os
import glob
import shutil

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.realpath(SCRIPT_DIR + "/../../../")
BUILD_DIR = ROOT_DIR + "/out/cmake-webgl-release"
SERVE_DIR = BUILD_DIR + "/libs/filamentjs"
PORT = 8000

# Copy assets used for testing into the server folder.

src_assets = (
  glob.glob(BUILD_DIR + "/samples/web/public/material/*") +
  glob.glob(BUILD_DIR + "/samples/web/public/monkey/*") +
  glob.glob(SCRIPT_DIR + "/testwasm.*"))

for src in src_assets:
  dst = SERVE_DIR + "/" + os.path.basename(src)
  shutil.copyfile(src, dst)

# Associate wasm files with the correct MIME type.

Handler = http.server.SimpleHTTPRequestHandler
Handler.extensions_map.update({
    '.wasm': 'application/wasm',
})

# Serve all files in the server folder.

print(f"http://localhost:{PORT}/testwasm.html")
Handler.directory = SERVE_DIR
os.chdir(SERVE_DIR)
socketserver.TCPServer.allow_reuse_address = True
with socketserver.TCPServer(("", PORT), Handler) as httpd:
    httpd.allow_reuse_address = True
    httpd.serve_forever()
