document.addEventListener('DOMContentLoaded', function() {
  if (document.getElementById('button')) {
    document.getElementById('button').addEventListener('click', click);
  }
});

function click() {
  var messages = []
  var results;
  chrome.runtime.onMessage.addListener(function(message, sender, sendResponse) {
    messages.push(message);
    if (results && messages.length == results.length) {
      completeProcess(messages);
    }
  });

  var serializer = {file: 'HTMLSerializer.js', allFrames: true};
  chrome.tabs.executeScript(null, serializer, function() {
    var contentScript = {file: 'content_script.js', allFrames: true};
    chrome.tabs.executeScript(null, contentScript, function(response) {
      results = response;
      if (messages.length == results.length)
        completeProcess(messages);
    });
  });

  var a = document.getElementById('button');
  a.className = 'serialize';
  a.innerHTML = 'Serializing...';
  a.removeEventListener('click', click);
}

/**
 * Takes all the responses from the injected content scripts and creates the
 * HTML file for download.
 * 
 * @param {Array<Object>} messages The response from all of the injected content
 *     scripts.
 */
function completeProcess(messages) {
  var html = outputHTMLString(messages);
  var file = new Blob([html], {type: 'text/html'});
  var url = URL.createObjectURL(file);

  var a = document.getElementById('button');
  a.className = 'download';
  a.innerHTML = 'Download';
  a.href = url;
  a.download = "snap-it.html";
}

/**
 * Converts the responses from the injected content scripts into a string
 * representing the HTML.
 * 
 * @param {Array<Object>} messages The response from all of the injected content
 *     scripts.
 * @return {string} The resulting HTML.
 */
function outputHTMLString(messages) {
  var rootIndex = 0;
  for (var i = 1; i < messages.length; i++) {
    rootIndex = messages[i].frameIndex === '0' ? i : rootIndex;
  }
  fillRemainingHolesAndMinimizeStyles(messages, rootIndex);
  return messages[rootIndex].html.join('');
}

/**
 * Fills all of the gaps in |messages[i].html|.
 *
 * @param {Array<Object>} messages The response from all of the injected content
 *     scripts.
 * @param {number} i The index of messages to use.
 */
function fillRemainingHolesAndMinimizeStyles(messages, i) {
  var html = messages[i].html;
  var frameHoles = messages[i].frameHoles;
  for (var index in frameHoles) {
    if (frameHoles.hasOwnProperty(index)) {
      var frameIndex = frameHoles[index];
      for (var j = 0; j < messages.length; j++) {
        if (messages[j].frameIndex == frameIndex) {
          fillRemainingHolesAndMinimizeStyles(messages, j);
          html[index] = messages[j].html.join('');
        }
      }
    }
  }
  minimizeStyles(messages[i]);
}

/**
 * Removes all style attribute properties that are unneeded.
 *
 * @param {Object} message The message Object whose style attributes should be
 *     minimized.
 */
function minimizeStyles(message) {
  var nestingDepth = message.frameIndex.split('.').length - 1;
  var iframe = document.createElement('iframe');
  document.body.appendChild(iframe);
  iframe.setAttribute(
      'style',
      `height: ${message.windowHeight}px;` + 
      `width: ${message.windowWidth}px;`);

  var html = message.html.join('');
  html = unescapeHTML(html, nestingDepth);
  iframe.contentDocument.documentElement.innerHTML = html;
  var doc = iframe.contentDocument;

  // Remove entry in |message.html| where extra style element was specified.
  message.html[message.pseudoElementTestingStyleIndex] = '';
  var finalPseudoElements = [];
  for (var selector in message.pseudoElementSelectorToCSSMap) {
    minimizePseudoElementStyle(message, doc, selector, finalPseudoElements);
  }

  if (finalPseudoElements.length > 0) {
    message.html[message.pseudoElementPlaceHolderIndex] =
        `<style>${finalPseudoElements.join(' ')}</style>`;
  }

  if (message.rootStyleIndex) {
    minimizeStyle(
        message,
        doc,
        doc.documentElement,
        message.rootId,
        message.rootStyleIndex);
  }

  for (var id in message.idToStyleIndex) {
    var index = message.idToStyleIndex[id];
    var element = doc.getElementById(id);
    if (element) {
      minimizeStyle(message, doc, element, id, index);
    }
  }
  iframe.remove();
}

/**
 * Removes all style attribute properties that are unneeded for a single
 *     pseudo element.
 *
 * @param {Object} message The message Object that contains the pseudo element
 *     whose style attributes should be minimized.
 * @param {Document} doc The Document that contains the rendered HTML.
 * @param {string} selector The CSS selector for the pseudo element.
 * @param {Array<string>} finalPseudoElements An array to contain the final
 *     declaration of pseudo elements.  It will be updated to reflect the pseudo
 *     element that is being processed.
 */
function minimizePseudoElementStyle(
    message,
    doc,
    selector,
    finalPseudoElements) {
  var maxNumberOfIterations = 5;
  var match = selector.match(/^#(.*):(:.*)$/);
  var id = match[1];
  var type = match[2];
  var element = doc.getElementById(id);
  if (element) {
    var originalStyleMap = message.pseudoElementSelectorToCSSMap[selector];
    var requiredStyleMap = {};
    // We compare the computed style before and after removing the pseudo
    // element and accumulate the differences in |requiredStyleMap|. The pseudo
    // element is removed by changing the element id. Because some properties
    // affect other properties, such as border-style: solid causing a change in
    // border-width, we do this iteratively until a fixed-point is reached (or
    // |maxNumberOfIterations| is hit).
    // TODO(sfine): Unify this logic with minimizeStyles.
    for (var i = 0; i < maxNumberOfIterations; i++) {
      var currentPseudoElement = ['#' + message.unusedId + ':' + type + '{'];
      currentPseudoElement.push(buildStyleAttribute(requiredStyleMap));
      currentPseudoElement.push('}');
      element.setAttribute('id', message.unusedId);
      var style = doc.getElementById(message.pseudoElementTestingStyleId);
      style.innerHTML = currentPseudoElement.join(' ');
      var foundNewRequiredStyle = updateMinimizedStyleMap(
          doc,
          element,
          originalStyleMap,
          requiredStyleMap,
          type);
      if (!foundNewRequiredStyle) {
        break;
      }
    }
    element.setAttribute('id', id);
    finalPseudoElements.push('#' + id + ':' + type + '{');
    var finalPseudoElement = buildStyleAttribute(requiredStyleMap);
    var nestingDepth = message.frameIndex.split('.').length - 1;
    finalPseudoElement = finalPseudoElement.replace(
        /"/g,
        escapedQuote(nestingDepth));
    finalPseudoElements.push(finalPseudoElement);
    finalPseudoElements.push('}');
  }
}


/**
 * Removes all style attribute properties that are unneeded for a single
 *     element.
 *
 * @param {Object} message The message Object that contains the element whose
 *     style attributes should be minimized.
 * @param {Document} doc The Document that contains the rendered HTML.
 * @param {Element} element The Element whose style attributes should be
 *     minimized.
 * @param {string} id The id of the Element in the final page.
 * @param {number} index The index in |message.html| where the Element's style
 *     attribute is specified.
 */
function minimizeStyle(message, doc, element, id, index) {
  var originalStyleAttribute = element.getAttribute('style');
  var originalStyleMap = message.idToStyleMap[id];
  var requiredStyleMap = {};
  var maxNumberOfIterations = 5;

  // We compare the computed style before and after removing the style attribute
  // and accumulate the differences in |requiredStyleMap|. Because some
  // properties affect other properties, such as boder-style: solid causing a
  // change in border-width, we do this iteratively until a fixed-point is
  // reached (or |maxNumberOfIterations| is hit).
  for (var i = 0; i < maxNumberOfIterations; i++) {
    element.setAttribute('style', buildStyleAttribute(requiredStyleMap));
    var foundNewRequiredStyle = updateMinimizedStyleMap(
        doc,
        element,
        originalStyleMap,
        requiredStyleMap,
        null);
    element.setAttribute('style', buildStyleAttribute(originalStyleMap));
    if (!foundNewRequiredStyle) {
      break;
    }
  }

  var finalStyleAttribute = buildStyleAttribute(requiredStyleMap);
  if (finalStyleAttribute) {
    var nestingDepth = message.frameIndex.split('.').length - 1;
    finalStyleAttribute = finalStyleAttribute.replace(
        /"/g,
        escapedQuote(nestingDepth + 1));
    var quote = escapedQuote(nestingDepth);
    message.html[index] = `style=${quote}${finalStyleAttribute}${quote} `;
  } else {
    message.html[index] = '';
  }
}

/**
 * We compare the original computed style with the minimized computed style
 * and update |minimizedStyleMap| based on any differences.
 *
 * @param {Document} doc The Document that contains the rendered HTML.
 * @param {Element} element The Element whose style attributes should be
 *     minimized.
 * @param {Object<string, string>} originalStyleMap A map representing the
 *     original computed style values. The keys are style attribute property
 *     names. The values are the corresponding property values.
 * @param {Object<string, string>} minimizedStyleMap A map representing the
 *     minimized style values. The keys are style attribute property names. The
 *     values are the corresponding property values.
 * @param {string} pseudo If the style describes an ordinary
 *     Element, then |pseudo| will be set to null.  If the style describes a
 *     pseudo element, then |pseudo| will be the string that represents that
 *     pseudo element.
 * @return {boolean} Returns true if minimizedStyleMap was changed. Returns false
 *     otherwise.
 */
function updateMinimizedStyleMap(
    doc,
    element,
    originalStyleMap,
    minimizedStyleMap,
    pseudo) {
  var currentComputedStyle = doc.defaultView.getComputedStyle(element, pseudo);
  var foundNewRequiredStyle = false;
  for (var property in originalStyleMap) {
    var originalValue = originalStyleMap[property];
    if (originalValue != currentComputedStyle.getPropertyValue(property)) {
      minimizedStyleMap[property] = originalValue;
      foundNewRequiredStyle = true;
    }
  }
  return foundNewRequiredStyle;
}

/**
 * Build a style attribute from a map of property names to property values.
 *
 * @param {Object<string, string} styleMap The keys are style attribute property
 *     names. The values are the corresponding property values.
 * @return {string} The correct style attribute.
 */
function buildStyleAttribute(styleMap) {
  var styleAttribute = [];
  for (var property in styleMap) {
    styleAttribute.push(property + ': ' + styleMap[property] + ';');
  }
  return styleAttribute.join(' ');
}

/**
 * Take a string that represents valid HTML and unescape it so that it can be
 * rendered.
 *
 * @param {string} html The HTML to unescape.
 * @param {number} nestingDepth The number of times the HTML must be unescaped.
 * @return {string} The unescaped HTML.
 */
function unescapeHTML(html, nestingDepth) {
  var div = document.createElement('div');
  for (var i = 0; i < nestingDepth; i++) {
    div.innerHTML = `<iframe srcdoc="${html}"></iframe>`;
    html = div.childNodes[0].attributes.srcdoc.value;
  }
  return html;
}

/**
 * Calculate the correct encoding of a quotation mark that should be used given
 * the nesting depth of the window in the frame tree.
 *
 * @param {number} depth The nesting depth of the appropriate window in the
 *     frame tree.
 * @return {string} The correctly escaped string. 
 */
function escapedQuote(depth) {
  if (depth == 0) {
    return '"';
  } else {
    var arr = 'amp;'.repeat(depth-1);
    return '&' + arr + 'quot;';
  }
}
