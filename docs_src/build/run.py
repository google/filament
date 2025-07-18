# Copyright (C) 2025 The Android Open Source Project
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

import json
import os
import re
from utils import execute, ArgParseImpl
import shutil

CUR_DIR = os.path.dirname(os.path.abspath(__file__))
DOCS_SRC_DIR = os.path.join(CUR_DIR, '../')
ROOT_DIR = os.path.join(CUR_DIR, '../../')

MDBOOK_DIR = os.path.join(CUR_DIR, '../src_mdbook')
BOOK_OUPUT_DIR = os.path.join(MDBOOK_DIR, 'book')
SRC_DIR = os.path.join(MDBOOK_DIR, 'src')

MARKDEEP_DIR = os.path.join(CUR_DIR, '../src_markdeep')
RAW_COPIES_DIR = os.path.join(CUR_DIR, '../src_raw')
DUP_DIR = os.path.join(SRC_DIR, 'dup')
MAIN_DIR = os.path.join(SRC_DIR, 'main')

FILAMENT_MD = 'Filament.md.html'
MATERIALS_MD = 'Materials.md.html'

def transform_dup_file_link(line, transforms):
  URL_CONTENT = '[-a-zA-Z0-9()@:%_\+.~#?&//=]+'
  res = re.findall(f'\[(.+)\]\(({URL_CONTENT})\)', line)
  for text, url in  res:
    word = f'[{text}]({url})'
    for tkey in transforms.keys():
      if url.startswith(tkey):
        nurl = url.replace(tkey, transforms[tkey])
        line = line.replace(word, f'[{text}]({nurl})')
        break
  return line

def pull_duplicates():
  if not os.path.exists(DUP_DIR):
    os.mkdir(DUP_DIR)

  config = {}
  with open(f'{CUR_DIR}/duplicates.json') as config_txt:
    config = json.loads(config_txt.read())

  for fin in config.keys():
    new_name = config[fin]['dest']
    link_transforms = config[fin].get('link_transforms', {})
    fpath = os.path.join(ROOT_DIR, fin)
    new_fpath = os.path.join(SRC_DIR, new_name)

    with open(fpath, 'r') as in_file:
      with open(new_fpath, 'w') as out_file:
        for line in in_file.readlines():
          out_file.write(transform_dup_file_link(line, link_transforms))

def pull_markdeep_docs():
  import http.server
  import socketserver
  import threading
  from selenium import webdriver
  from selenium.webdriver.chrome.options import Options
  from selenium.webdriver.common.by import By
  import time

  class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    """Handle requests in a separate thread."""

  class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=MARKDEEP_DIR, **kwargs)

  def start_server(port):
    """Starts the web server in a separate thread."""
    httpd = Server(("", port), Handler)
    server_thread = threading.Thread(target=httpd.serve_forever)
    server_thread.daemon = True  # Allow main thread to exit
    server_thread.start()
    print(f"Server started on port {port}...")
    return httpd

  PORT = 12345
  httpd = start_server(PORT)

  # Set up Chrome options for headless mode
  chrome_options = Options()
  chrome_options.add_argument("--headless")

  # This option is necessary for running on some VMs
  chrome_options.add_argument("--no-sandbox")

  # Create a new Chrome instance in headless mode
  driver = webdriver.Chrome(options=chrome_options)

  for doc in ['Filament', 'Materials']:
    # Open the URL with ?export, which markdeep will export the resulting html.
    driver.get(f"http://localhost:{PORT}/{doc}.md.html?export")

    time.sleep(3)
    # We extract the html from the resulting "page" (an html output itself).
    text = driver.find_elements(By.TAG_NAME, "pre")[0].text

    # 1. Remove the double empty lines.  These make the following text seem like markdown text as oppose to embedded html.
    # 2. Remove the max-width styling from the body tag.
    # 3. Remove the font-family styling from the body tag.
    text = text.replace("\n\n","\n")\
               .replace("max-width:680px;", "")\
               .replace("font-family:Palatino", "--font-family:Palatino")\
               .replace("\"./images", "\"../images")\
               .replace("\"images/", "\"../images/")

    # Save the page source as .md with embedded html
    with open(f'{MAIN_DIR}/{doc.lower()}.md', "w", encoding="utf-8") as f:
      f.write(text)

  # Close the browser
  driver.quit()
  # Shutdown the server
  httpd.shutdown()

if __name__ == "__main__":
  pull_duplicates()
  pull_markdeep_docs()

  res, err = execute('mdbook build', cwd=MDBOOK_DIR)
  assert res == 0, f"failed to execute `mdbook`. return-code={res} err=\"{err}\""

  shutil.copytree(RAW_COPIES_DIR, BOOK_OUPUT_DIR, dirs_exist_ok=True)
  shutil.copy(os.path.join(MARKDEEP_DIR, FILAMENT_MD), BOOK_OUPUT_DIR)
  shutil.copy(os.path.join(MARKDEEP_DIR, MATERIALS_MD), BOOK_OUPUT_DIR)
