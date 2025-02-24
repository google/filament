/**
 * Copyright 2018 The ANGLE Project Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * This script is meant to be executed agaisnt the vulkan spec 31.3.3 tables.
 * Instructions: Copy all the tables from the HTML source to a plain document
 * and add a textarea at the bottom with ID='result'. Execute this JS on load
 * of the document and you should have a JSON string in the textarea with the
 * mandatory specs included. If the spec changes format / CSS in any way, the
 * selectors will need to be modified.
 **/
var indexToFeatureMap = [];
var index = 12;

var outputJson = {};

// Map all features to indexes of squares.
$("#features-formats-mandatory-features-subbyte td").each(function() {
  $this = $(this);
  $this.find("code").each(function() {
    if ($(this).text().startsWith("VK_FORMAT_FEATURE")) {
        indexToFeatureMap[index--] = $(this).text();
    }
  });
});

var allTableIds =
  ["features-formats-mandatory-features-subbyte",
  "features-formats-mandatory-features-2byte",
  "features-formats-mandatory-features-4byte",
  "features-formats-mandatory-features-10bit",
  "features-formats-mandatory-features-16bit",
  "features-formats-mandatory-features-32bit",
  "features-formats-mandatory-features-64bit",
  "features-formats-mandatory-features-depth-stencil",
  "features-formats-mandatory-features-features-bcn",
  "features-formats-mandatory-features-features-etc",
  "features-formats-mandatory-features-features-astc"];

for (var i = 0; i < allTableIds.length; i++) {
  $("#" + allTableIds[i] + " td").each(function() {
    $this = $(this);

    $this.find("code").each(function() {
      if (!$(this).text().startsWith("VK_FORMAT_FEATURE") &&
          $(this).text().startsWith("VK_FORMAT")) {
        // Found one vkFormat to features line.
        var vkFormat = $(this).text();
        var squareIndex = 0;
        var features = [];
        var skipEntry = false;

        $(this).closest("tr").find("td.halign-center").each(function() {
          // Find all squares with features.
          if ($(this).text() === "✓") {
            features.push(indexToFeatureMap[squareIndex]);
          }
          if ($(this).text() === "†") {
            skipEntry = true;
            return false; // Break;
          }
          squareIndex++;
        });
        if (!skipEntry &&
              (features.length > 0)) {
          Object.defineProperty(outputJson, vkFormat, {
            value: features,
            enumerable: true,
            readable: true,
            writable: false
          });
        }
      }
    });
  });
}
$("#result").text(JSON.stringify(outputJson));
