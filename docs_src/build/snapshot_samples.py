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
"""
Automated WebGL Snapshot Generator

This script starts a local HTTP server pointing to the built mdbook output directory 
and uses a headless Chrome browser (via Selenium) to load the generated WebGL examples.

For each sample and tutorial:
1. It waits for the scene and its assets (textures, models, materials) to fully render.
2. It explicitly targets the `<canvas>` DOM element to take an isolated screenshot.
3. It crops the raw screenshot into a perfect square based on its aspect ratio.
4. It uses Lanczos resampling to scale the image down to a 100x100 pixel thumbnail.
5. It saves the final thumbnail to `docs_src/src_mdbook/src/images/` for embedding in the Markdown.

These thumbnails are used directly by the "Web Tutorials" and "Web Samples" index pages.
"""

import os
import time
from PIL import Image
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
import http.server
import socketserver
import threading

CUR_DIR = os.path.dirname(os.path.abspath(__file__))
BOOK_DIR = os.path.join(CUR_DIR, '../src_mdbook/book')
IMAGES_DIR = os.path.join(CUR_DIR, '../src_mdbook/src/images')

os.makedirs(IMAGES_DIR, exist_ok=True)

class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    pass

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=BOOK_DIR, **kwargs)

def start_server(port):
    httpd = Server(("", port), Handler)
    server_thread = threading.Thread(target=httpd.serve_forever)
    server_thread.daemon = True
    server_thread.start()
    print(f"Server started on port {port}...")
    return httpd

def snapshot_samples():
    port = 8081
    httpd = start_server(port)
    time.sleep(2)

    chrome_options = Options()
    chrome_options.add_argument("--headless")
    chrome_options.add_argument("--window-size=800,600")
    
    driver = webdriver.Chrome(options=chrome_options)
    
    samples = ['animation', 'cube_fl0', 'helmet', 'morphing', 'parquet', 'skinning', 'sky', 'triangle', 'redball', 'suzanne']
    
    for sample in samples:
        print(f"Taking snapshot of {sample}...")
        driver.get(f"http://localhost:{port}/samples/web/{sample}.html")
        time.sleep(4) # Wait for models to load and render
        
        canvas = driver.find_element(By.TAG_NAME, "canvas")
        if not canvas:
            print(f"Failed to find canvas for {sample}")
            continue
            
        # Get raw screenshot of the canvas element
        png = canvas.screenshot_as_png
        import io
        img = Image.open(io.BytesIO(png))
        
        # Crop to square
        width, height = img.size
        size = min(width, height)
        left = (width - size) / 2
        top = (height - size) / 2
        right = (width + size) / 2
        bottom = (height + size) / 2
        
        img = img.crop((left, top, right, bottom))
        
        # Resize to 100x100
        img = img.resize((100, 100), Image.Resampling.LANCZOS)
        
        img.save(os.path.join(IMAGES_DIR, f"web_sample_{sample}.png"))
        print(f"Saved snapshot for {sample}")

    driver.quit()
    httpd.shutdown()

if __name__ == "__main__":
    snapshot_samples()