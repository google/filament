# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import codecs
import os
import logging
import json
import re
import shutil
import sys
from io import BytesIO

import six.moves.urllib.request # pylint: disable=import-error
from six.moves import input # pylint: disable=redefined-builtin

from telemetry.core import util
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import browser_options


HTML_SUFFIX = '.html'
STRIP_QUERY_PARAM_REGEX = re.compile(r'\?.*$')
EXPENSIVE_JS_TIMEOUT_SECONDS = 240

def _TransmitLargeJSONToTab(tab, json_obj, js_holder_name):
  tab.ExecuteJavaScript(
      'var {{ @js_holder_name }} = "";', js_holder_name=js_holder_name)

  # To avoid crashing devtool connection (details in crbug.com/763119#c16),
  # we break down the json string to chunks which each chunk has a maximum
  # size of 100000 characters (100000 seems to not break the connection and
  # makes sending data reasonably fast).
  k = 0
  step_size = 100000
  json_obj_string = json.dumps(json_obj)
  while k < len(json_obj_string):
    sub_string_chunk = json_obj_string[k: k + step_size]
    k += step_size
    tab.ExecuteJavaScript(
        '{{ @js_holder_name }} += {{ sub_string_chunk }};',
        js_holder_name=js_holder_name, sub_string_chunk=sub_string_chunk)

  tab.ExecuteJavaScript(
      '{{ @js_holder_name }} = JSON.parse({{ @js_holder_name }});',
      js_holder_name=js_holder_name)


def _CreateBrowser(finder_options, enable_browser_log):
  if enable_browser_log:
    # Enable NON_VERBOSE_LOGGING which also contains devtool's console logs.
    finder_options.browser_options.logging_verbosity = (
        browser_options.BrowserOptions.NON_VERBOSE_LOGGING)
    # Do not upload the log to cloud storage.
    finder_options.browser_options.logs_cloud_bucket = None

  possible_browser = browser_finder.FindBrowser(finder_options)
  return possible_browser.BrowserSession(finder_options.browser_options)


def _ReadSnapItSource(path):
  """ Returns the contents of the snap-it source file at the given path
  relative to the snap-it repository.
  """
  full_path = os.path.join(util.GetCatapultThirdPartyDir(), 'snap-it', path)
  with open(full_path) as f:
    return f.read()


def _FetchImages(image_dir, frame_number, external_images):
  if len(external_images) == 0:
    return

  image_count = len(external_images)
  print('Fetching external images [local_dir=%s, frame_number=%d, '
        'image_count=%d].' % (image_dir, frame_number, image_count))

  for i in range(image_count):
    [element_id, image_url] = external_images[i]
    _, image_file_extension = os.path.splitext(image_url)
    # Strip any query param and all subsequent characters. Note that
    # we also do this JavaScript-side (see HTMLSerializer.fileSuffix),
    # but the stripped file name isn't currently passed back in the
    # interest of shipping less data around.
    image_file_extension = STRIP_QUERY_PARAM_REGEX.sub('', image_file_extension)
    image_file = os.path.join(image_dir, '%d-%s%s' % (
        frame_number, element_id, image_file_extension))
    sys.stdout.write('Fetching image #%i / %i\r' % (i, image_count))
    sys.stdout.flush()
    logging.info('Fetching image [frame_number=%d, %d/%d, local_file=%s, '
                 'url=%s].' % (frame_number, i, image_count, image_file,
                               image_url))
    try:
      image_request = six.moves.urllib.request.urlopen(image_url)
    except IOError as e:
      print('Error fetching image [local_file=%s, url=%s, message=%s].' % (
          image_file, image_url, e))
      continue

    try:
      with open(image_file, 'wb') as image_file_handle:
        shutil.copyfileobj(BytesIO(image_request.read()), image_file_handle)
    except IOError as e:
      print('Error copying image [local_file=%s, url=%s, message=%s].' % (
          image_file, image_url, e))

def _GetLocalImageDirectory(snapshot_path):
  return os.path.splitext(snapshot_path)[0]

def _SnapPageToFile(finder_options, url, interactive, snapshot_path,
                    snapshot_file, enable_browser_log):
  """ Save the HTML snapshot of the page whose address is |url| to
  |snapshot_file|.
  """
  with _CreateBrowser(finder_options, enable_browser_log) as browser:
    tab = browser.tabs[0]
    tab.Navigate(url)
    if interactive:
      input(
          'Activating interactive mode. Press enter after you finish '
          "interacting with the page to snapshot the page's DOM content.")

    print('Snapshotting content of %s. This could take a while...' % url)
    tab.WaitForDocumentReadyStateToBeComplete()
    tab.action_runner.WaitForNetworkQuiescence(
        timeout_in_seconds=EXPENSIVE_JS_TIMEOUT_SECONDS)

    snapit_script = _ReadSnapItSource('HTMLSerializer.js')
    dom_combining_script = _ReadSnapItSource('popup.js')
    image_dir = _GetLocalImageDirectory(snapshot_path)
    if not os.path.exists(image_dir):
      os.mkdir(image_dir)
    serialized_doms = []
    # |external_images| holds, for each frame, a list of tuples as
    # (element id), (image src url) with the url as it was in the
    # original unmodified page html. We use the element id to construct
    # a page-unique local image filename. We use the url to fetch the
    # image from the external server.
    external_images = []

    # Serialize the dom in each frame.
    frame_number = 0
    for context_id in tab.EnableAllContexts():
      # Build a distinct local image path for each frame by including
      # the frame number as the prefix string for the eventual file.
      local_image_path = os.path.join(os.path.basename(image_dir),
                                      '%d-' % frame_number)
      tab.ExecuteJavaScript(snapit_script, context_id=context_id)
      tab.ExecuteJavaScript(
          '''
          var serializedDom;
          var htmlSerializer = new HTMLSerializer();
          htmlSerializer.setLocalImagePath('%s');
          htmlSerializer.processDocument(document);
          htmlSerializer.fillHolesAsync(document, function(s) {
            serializedDom = s.asDict();
          });
          ''' % local_image_path, context_id=context_id,
          timeout=EXPENSIVE_JS_TIMEOUT_SECONDS)
      tab.WaitForJavaScriptCondition(
          'serializedDom !== undefined', context_id=context_id)
      serialized_doms.append(tab.EvaluateJavaScript(
          'serializedDom', context_id=context_id))
      external_images.append(tab.EvaluateJavaScript(
          'htmlSerializer.externalImages', context_id=context_id))
      frame_number += 1

    # Execute doms combining code in blank page to minimize the chance of V8
    # OOM.
    tab.Navigate('about:blank')
    tab.WaitForDocumentReadyStateToBeComplete()

    # Sending all the serialized doms back to tab execution context.
    tab.ExecuteJavaScript('var serializedDoms = [];')
    for i, dom in enumerate(serialized_doms):
      sys.stdout.write('Processing dom of frame #%i / %i\r' %
                       (i, len(serialized_doms)))
      sys.stdout.flush()
      _TransmitLargeJSONToTab(tab, dom, 'sub_dom')
      tab.ExecuteJavaScript('serializedDoms.push(sub_dom);')

    # Combine all the doms to one HTML string.
    tab.EvaluateJavaScript(dom_combining_script,
                           timeout=EXPENSIVE_JS_TIMEOUT_SECONDS)
    page_snapshot = tab.EvaluateJavaScript('outputHTMLString(serializedDoms);',
                                           timeout=EXPENSIVE_JS_TIMEOUT_SECONDS)

    print('Writing page snapshot [path=%s].' % snapshot_path)
    snapshot_file.write(page_snapshot)
    for i, image in enumerate(external_images):
      _FetchImages(image_dir, i, image)


def SnapPage(finder_options, url, interactive, snapshot_path,
             enable_browser_log):
  """ Save the HTML snapshot of the page whose address is |url| to
  the file located at the relative path |snapshot_path|.
  """
  if not snapshot_path.endswith(HTML_SUFFIX):
    raise ValueError('Snapshot path should end with \'%s\' [value=\'%s\'].' % (
        HTML_SUFFIX, snapshot_path))

  snapshot_path = os.path.abspath(snapshot_path)
  with codecs.open(snapshot_path, 'w', 'utf-8') as f:
    _SnapPageToFile(finder_options, url, interactive, snapshot_path, f,
                    enable_browser_log)
  print('Successfully saved snapshot to file://%s' % snapshot_path)
