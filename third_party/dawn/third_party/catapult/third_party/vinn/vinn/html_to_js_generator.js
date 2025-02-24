// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @fileoverview Provides tools for parsing HTML to generate javascript, needed
 * for d8 bootstrapping process.
 *
 * This file depends on Parse5.js. It must be loaded into global before
 * this file executes.
 */

(function(global) {

  var adapter = parse5.TreeAdapters.default;
  var parser = new parse5.Parser(adapter, {locationInfo: true});
  var serializer = new parse5.Serializer(adapter);


  function JsGenerator(html_text) {
    this.html_text_ = html_text;
  }

  JsGenerator.prototype = {
    __proto__: Object.prototype,

    /**
     * Create a javascript "chunk" that will be used later for final assembling
     * of all javascript contents generated from |html_text|.
     *
     * A javascript chunk contains:
     *  content: the text content (string) of the chunk.
     *  start_location: the original location of the html content that
     *    generates this chunk.
     *  expected_start_line_number: the line number that the text content
     *    generated from this chunk is supposed to be on.
     */
    createJsChunk(content, start_location, expected_start_line_number) {
      return {
        content: content,
        start_location: start_location,
        expected_start_line_number: expected_start_line_number
      };
    },

    getNumLinesOfString(s) {
      return (s.match(/\n/g) || []).length;
    },

    startLocation(node) {
      if (!node.__location)
        return undefined;
      return node.__location.start;
    },

    startLineNumber(node) {
      if (!node.__location)
        return undefined;
      return this.getNumLinesOfString(
          this.html_text_.substring(0, node.__location.start)) + 1;
    },

    startLocationOfContent(node) {
      if (!node.__location || !node.__location.startTag)
        return undefined;
      return node.__location.startTag.end;
    },

    startLineNumberOfContent(node) {
      if (!node.__location || !node.__location.startTag)
        return undefined;
      var start = node.__location.startTag.end;
      return this.getNumLinesOfString(this.html_text_.substring(0, start)) + 1;
    },

    generateJsFromJsChunks(chunks) {
      var results = [];
      chunks.sort(function(a, b) {
        return a.start_location - b.start_location;
      });
      var current_num_lines = 1;
      for (var i = 0; i < chunks.length; i++) {
        var num_blank_lines_to_insert = (
            chunks[i].expected_start_line_number - current_num_lines);
        if (num_blank_lines_to_insert < 0) {
          throw new Error('Cannot generate js content for ' +
                          chunks[i].content + '\n. Expected to add ' +
                          num_blank_lines_to_insert + ' new lines.');
        }
        var new_lines = new Array(num_blank_lines_to_insert + 1).join('\n');
        results.push(new_lines);
        results.push(chunks[i].content);
        current_num_lines += (
            num_blank_lines_to_insert +
            this.getNumLinesOfString(chunks[i].content));
      }
      return results.join('');
    },

    generateJsChunksForLinkNode: function(node) {
      var is_import_link = false;
      var href = '';
      for (var i = 0; i < node.attrs.length; i++) {
        if (node.attrs[i].name === 'rel' && node.attrs[i].value === 'import')
          is_import_link = true;
        if (node.attrs[i].name === 'href')
          href = node.attrs[i].value;
      }
      if (!is_import_link)
        return [];
      var chunk = this.createJsChunk(
          'global.HTMLImportsLoader.loadHTML(\'' + href + '\');',
          this.startLocation(node),
          this.startLineNumber(node));
      return [chunk];
    },

    generateJsChunksForScriptTag(node) {
      var src;
      for (var i = 0; i < node.attrs.length; i++) {
        if (node.attrs[i].name === 'src')
          src = node.attrs[i].value;
      }
      if (!src)
        return [];
      var chunk = this.createJsChunk(
          'global.HTMLImportsLoader.loadScript(\'' + src + '\');',
          this.startLocation(node),
          this.startLineNumber(node));
      return [chunk];
    },

    generateJsChunksForScriptText(node) {
      var script_content = serializer.serialize(node);
      var chunk = this.createJsChunk(
          script_content,
          this.startLocationOfContent(node),
          this.startLineNumberOfContent(node));
      return [chunk];
    },

    generateJsChunksForScriptNode: function(node) {
      var tagChunks = this.generateJsChunksForScriptTag(node);
      var textChunks = this.generateJsChunksForScriptText(node);
      return tagChunks.concat(textChunks);
    },

    generateJsChunksForNode: function(node) {
      if (node.nodeName === 'link')
        return this.generateJsChunksForLinkNode(node);
      if (node.nodeName === 'script')
        return this.generateJsChunksForScriptNode(node);
      return [];
    },

    generateJsFromHTML: function() {
      var document = parser.parse(this.html_text_);
      var nodes_list = document.childNodes;
      var generated_js_chunks = [];
      while (nodes_list.length) {
        var node = nodes_list.pop();
        var chunks = this.generateJsChunksForNode(node);
        generated_js_chunks.push.apply(generated_js_chunks, chunks);
        if (node.childNodes)
          nodes_list.push.apply(nodes_list, node.childNodes);
      }
      return this.generateJsFromJsChunks(generated_js_chunks);
    }
  };

  global.generateJsFromHTML = function(html_text) {
    return new JsGenerator(html_text).generateJsFromHTML();
  }
})(this);
