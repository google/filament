/**
 * HTML Serializer that takes a document and synchronously stores it as an array
 * of strings, then asynchronously retrieves data URLs for same-origin images.
 * It stores enough state to later be converted to an html text file.
 */
var HTMLSerializer = class {
  constructor() {

    /**
     * @private {Set<string>} Contains the tag names that should be
     *     ignored while serializing a document.
     * @const
     */
    this.FILTERED_TAGS = new Set(['SCRIPT', 'NOSCRIPT', 'STYLE', 'LINK']);

    /**
     * @private {Set<string>} Contains the tag names for elements
     *     that have no closing tags.  List of tags taken from:
     *     https://html.spec.whatwg.org/multipage/syntax.html#void-elements.
     * @const
     */
    this.NO_CLOSING_TAGS = new Set([
      'AREA',
      'BASE',
      'BR',
      'COL',
      'EMBED',
      'HR',
      'IMG',
      'INPUT',
      'KEYGEN',
      'LINK',
      'META',
      'PARAM',
      'SOURCE',
      'TRACK',
      'WBR'
    ]);

    /**
     * @private {Array<string>} A list of the pseudo elements that will be
     *     processed.
     * @const
     */
    this.PSEUDO_ELEMENTS = [':before', ':after'];

    /**
     * @private {Object<string, string>} The keys are all characters that need
     *     to be properly escaped when in a text node.  The value is the
     *     properly escaped string.
     * @const
     */
    this.CHARACTER_ESCAPING_MAP = {
      '&' : '&amp;',
      '<' : '&lt;',
      '>' : '&gt;',
      '"' : '&quot;',
      "'" : '&#39;'
    };

    /**
     * @public {Object<string, number>} An enum representing different types of
     *     text.
     * @const
     */
    this.INPUT_TEXT_TYPE = {
      HTML : 0,
      CSS : 1
    };

    /**
     * @public {Array<string>} This array represents the serialized html that
     *     makes up a node or document.
     */
    this.html = [];

    /**
     * @public {Object<number, string>} The keys represent an index in
     *     |this.html|. The value is a url at which the resource that belongs at
     *     that index can be retrieved. The resource will eventually be
     *     converted to a data url.
     */
    this.srcHoles = {};

    /**
     * @public {Object<number, string>} The keys represent an index in
     *     |this.html|. The value is a string that uniquely identifies an
     *     iframe, the serialized contents of which should be placed at that
     *     index of |this.html|.
     */
    this.frameHoles = {};

    /**
     * @public {Array<Array<string, string>>} An array of tuples as
     *     |(element id, fully qualified url)| detailing any
     *     external image urls found in the webpage.
     */
    this.externalImages = [];

    /**
     * @private {Array<string>} Each element of |this.crossOriginStyleSheets|
     *     contains a url to a stylesheet that does not have the same origin
     *     as the webpage being serialized.
     */
    this.crossOriginStyleSheets = [];

    /**
     * @private {Array<string>} Each element of |this.fontCSS| will contain
     *     a CSS declaration of a different externally loaded font.
     */
    this.fontCSS = [];

    /**
     * @private {number} The index in |this.html| where the style element
     *     containing the fonts will go.
     */
    this.fontPlaceHolderIndex;

    /**
     * @private {number} The index in |this.html| where the style element
     *     containing the pseudo elements will go.
     */
    this.pseudoElementPlaceHolderIndex;

    /**
     * @private {number} The id of a style element that can be used to test
     *     minimized pseudo element declarations in popup.js.
     */
    this.pseudoElementTestingStyleId;

    /**
     * @private {number} The index in |this.html| where a style element will be
     *     placed to test minimized pseudo element declarations in popup.js.
     */
    this.pseudoElementTestingStyleIndex;

    /**
     * @private {Array<string>} Each element of this array is a string
     *     representing CSS that defines a single pseudo element.
     */
    this.pseudoElementCSS = [];

    /**
     * @private {Object<string, Object<string, string>>} The keys represent a
     *     pseudo element selector. The value is a map of that pseudo element's
     *     style property names to property values.
     */
    this.pseudoElementSelectorToCSSMap = {};

    /**
     * @private {Function} A funtion that generates a unique string each time it
     * is called, which can be used as an element id.
     */
    this.generateId = this.generateIdGenerator();

    /**
     * @private {number} The window height of the Document being serialized.
     */
    this.windowHeight;

    /**
     * @private {number} The window width of the Document being serialized.
     */
    this.windowWidth;

    /**
     * @private {Object<string, number>} The keys represent the id of an
     *     Element. The value is the index in |this.html| where the
     *     value of the style attribute for that Element is specified.
     */
    this.idToStyleIndex = {};

    /**
     * @private {Object<string, Object<string, string>>} The keys represent the
     *     id of an Element.  The value is a map of that Element's style
     *     attribute property names to property values.
     */
    this.idToStyleMap = {};

    /**
     * @private {number} The index in |this.html| at which the html element's
     *     style attribute is specified.
     */
    this.rootStyleIndex;

    /**
     * @private {string} The assigned id of the html element.
     */
    this.rootId;

    /**
     * @private {?string} An optional string representing the path
     *     under which any external images fetched and stored locally
     *     will reside.
     */
    this.localImagePath = null;
  }

  /**
   * Takes an html document, and populates this objects fields such that it can
   * eventually be converted into an html file.
   *
   * @param {Document} doc The Document to serialize.
   */
  processDocument(doc) {
    this.windowHeight = doc.defaultView.innerHeight;
    this.windowWidth = doc.defaultView.innerWidth;

    if (doc.doctype) {
      this.html.push('<!DOCTYPE html>\n');
    }

    if (this.iframeQualifiedName(doc.defaultView) == '0') {
      this.html.push(
          `<!-- Original window height: ${this.windowHeight}. -->\n`);
      this.html.push(`<!-- Original window width: ${this.windowWidth}. -->\n`);
    }

    this.loadFonts(doc);
    this.pseudoElementPlaceHolderIndex = this.html.length;
    this.html.push(''); // Entry where pseudo element style tag will go.
    this.pseudoElementTestingStyleIndex = this.html.length;
    this.html.push(''); // Entry where minimized pseudo elements can be tested.

    var nodes = doc.childNodes;
    for (var i = 0, node; node = nodes[i]; i++) {
      if (node.nodeType != Node.DOCUMENT_TYPE_NODE) {
        this.processTree(node);
      }
    }
    if (this.pseudoElementCSS.length > 0) {
      var pseudoElements = `<style>${this.pseudoElementCSS.join('')}</style>`;
      this.html[this.pseudoElementPlaceHolderIndex] = pseudoElements;
    }

    this.pseudoElementTestingStyleId = this.generateId(doc);
    var style = `<style id="${this.pseudoElementTestingStyleId}"></style>`;
    var nestingDepth = this.windowDepth(doc.defaultView);
    var escapedQuote = this.escapedCharacter('"', nestingDepth);
    style = style.replace(/"/g, escapedQuote);
    this.html[this.pseudoElementTestingStyleIndex] = style;
  }

  /**
   * Takes an html node, and populates this object's fields such that it can
   * eventually be converted into an html text file.
   *
   * @param {Node} node The Node to serialize.
   * @private
   */
  processTree(node) {
    var tagName = node.tagName;
    if (!tagName && node.nodeType != Node.TEXT_NODE) {
      // Ignore nodes that don't have tags and are not text.
    } else if (tagName && this.FILTERED_TAGS.has(tagName)) {
      // Filter out nodes that are in filteredTags.
    } else if (node.nodeType == Node.TEXT_NODE) {
      this.processText(node);
    } else {
      this.html.push(`<${tagName.toLowerCase()} `);
      var id;
      if (node.attributes.id) {
        id = node.attributes.id.value;
      } else {
        id = this.generateId(node.ownerDocument);
      }
      this.processAttributes(node, id);
      this.processPseudoElements(node, id);
      this.processBackgroundImageStyle(node, id);
      this.html.push('>');

      if (tagName == 'HEAD') {
        this.fontPlaceHolderIndex = this.html.length;
        this.html.push('');
        this.pseudoElementPlaceHolderIndex = this.html.length;
        this.html.push('');
        this.pseudoElementTestingStyleIndex = this.html.length;
        this.html.push('');
      }

      var children = node.childNodes;
      if (children) {
        for (var i = 0, child; child = children[i]; i++) {
          this.processTree(child);
        }
      }

      if (!this.NO_CLOSING_TAGS.has(tagName)) {
        this.html.push(`</${tagName.toLowerCase()}>`);
      }
    }
  }

  /**
   * Takes an HTML element, and if it has pseudo elements listed in
   * |this.PSEUDO_ELEMENTS| they will be added to |this.pseudoElementCSS| and
   * |this.pseudoElementSelectorToCSSMap|.
   *
   * @param {Element} element The Element whose pseudo elements will be
   *     processed.
   * @param {string} id The id of the Element whose pseudo elements will be
   *     processed.
   * @private
   */
  processPseudoElements(element, id) {
    var win = element.ownerDocument.defaultView;
    for (var i = 0, pseudo; pseudo = this.PSEUDO_ELEMENTS[i]; i++) {
      var style = win.getComputedStyle(element, pseudo);
      if (style.content) {
        var nestingDepth = this.windowDepth(win);
        var escapedQuote = this.escapedCharacter('"', nestingDepth);
        var styleText = style.cssText.replace(/"/g, escapedQuote);
        styleText = this.escapedUnicodeString(
            styleText,
            this.INPUT_TEXT_TYPE.CSS);
        this.pseudoElementCSS.push(
            '#' + id + ':' + pseudo + '{' + styleText + '} ');

        var styleMap = {};
        for (var i = 0; i < style.length; i++) {
          var propertyName = style.item(i);
          var propertyValue = style.getPropertyValue(propertyName);
          propertyValue = this.escapedUnicodeString(
              propertyValue,
              this.INPUT_TEXT_TYPE.CSS);
          styleMap[propertyName] = propertyValue;
        }
        this.pseudoElementSelectorToCSSMap['#' + id + ':' + pseudo] = styleMap;
      }
    }
  }

  /**
   * Takes an html node of type Node.TEXT_NODE, and add its text content with
   *     all characters properly escaped to |this.html|.
   * @param {Node} node The text node.
   */
   // TODO(sfine): Take care of attribute value normalization:
   // https://developers.whatwg.org/the-iframe-element.html#the-iframe-element
  processText(node) {
    var win = node.ownerDocument.defaultView;
    var nestingDepth = this.windowDepth(win);
    var text = node.textContent;
    text = this.escapedCharacterString(text, nestingDepth+1);
    text = this.escapedUnicodeString(text, this.INPUT_TEXT_TYPE.HTML);
    this.html.push(text);
  }

  /**
   * Processes the 'background-image' style for the given element,
   * noting any external images in |this.externalImages| and
   * potentially mutating their url to allow for local storage as
   * appropriate.
   *
   * @param {Element} element The Element to serialize.
   * @param {string} id The id of the Element being serialized.
   * @private
   */
  processBackgroundImageStyle(element, id) {
    var imageStyle = this.idToStyleMap[id]['background-image'];
    if (!imageStyle || imageStyle == 'none')
      return;

    // TODO(wkorman): Create helper method to reduce win code redundancy.
    var imageUrls = imageStyle.split(',');
    var outImageUrls = [];
    var urlRegex = /\s*url\("([^)]*)"\)/;
    // TODO(wkorman): Finish support for multiple images in a single
    // element's background-image style. See also the unit test for
    // background-image with multiple images.
    for (var i = 0; i < imageUrls.length; i++) {
      var rawImageUrl = imageUrls[i].trim();
      var matches = rawImageUrl.match(urlRegex);
      // If it doesn't look like a url, leave it alone.
      if (!matches) {
        outImageUrls.push(rawImageUrl);
        continue;
      }

      var parsedUrl = matches[1];
      var url = this.qualifiedUrl(parsedUrl);

      if (this.isImageDataUrl(url.href)) {
        // Just pass the url through as it will render directly.
        outImageUrls.push(wrapUrl(url.href));
      } else {
        // TODO(wkorman): For same-origin images, consider creating a
        // "hole" and processing it somehow. For now we just punt and
        // treat same-origin images as external images, which is
        // inefficient, but should generally work.
        this.externalImages.push([id, url.href]);
        var wrappedUrl = this.wrapUrl(this.getExternalImageUrl(id, url.href));
        outImageUrls.push(wrappedUrl);
      }
    }

    var processedImageStyle = outImageUrls.join(',');
    this.idToStyleMap[id]['background-image'] = processedImageStyle;
  }

  /**
   * Takes an html element, and populates this object's fields with the
   * appropriate attribute names and values.
   *
   * @param {Element} element The Element to serialize.
   * @param {string} id The id of the Element being serialized.
   * @private
   */
  processAttributes(element, id) {
    var win = element.ownerDocument.defaultView;
    var style = win.getComputedStyle(element, null);
    var styleMap = {};
    for (var i = 0; i < style.length; i++) {
      var propertyName = style.item(i);
      styleMap[propertyName] = style.getPropertyValue(propertyName);
    }
    this.idToStyleMap[id] = styleMap;
    this.idToStyleIndex[id] = this.html.length;
    if (element.tagName == 'HTML') {
      this.rootStyleIndex = this.html.length;
      this.rootId = id;
    }
    this.processSimpleAttribute(win, 'style', style.cssText);
    this.processSimpleAttribute(win, 'id', id);

    var attributes = element.attributes;
    if (attributes) {
      for (var i = 0, attribute; attribute = attributes[i]; i++) {
        switch (attribute.name.toLowerCase())  {
          case 'src':
            this.processSrcAttribute(element, id);
            break;
          case 'style':
          case 'id':
            break;
          default:
            var name = attribute.name;
            var value = attribute.value;
            this.processSimpleAttribute(win, name, value);
        }
      }
      // TODO(sfine): Ensure this is working by making sure that an iframe
      //              will always have attributes.
      if (element.tagName == 'IFRAME' && element.attributes.src) {
        var valueIndex = this.processHoleAttribute(win, 'srcdoc');
        var iframeName = this.iframeQualifiedName(element.contentWindow);
        this.frameHoles[valueIndex] = iframeName;
      }
    }
  }

  /**
   * @return {boolean} whether the given url is a data url with an image media
   *     type.
   * @private
   */
  isImageDataUrl(url) {
    // We could parse the mime type and scrutinize components to
    // ensure validity, but we live with simple string checks for now.
    return url.startsWith('data:image/') &&
        url.indexOf('/') < (url.length - 1);
  }

  /**
   * @return {string} the url for the external image with the given
   *     element id
   * @param {string} id The element id
   * @param {string} url The image element source url
   * @private
   */
  getExternalImageUrl(id, url) {
    // Pass through external image urls unmodified if we've not
    // got a local image path set, thus connoting that we won't
    // be fetching external images for local storage.
    if (!this.localImagePath)
      return url;
    var localUrl = this.localImagePath + id;
    var suffix = this.fileSuffix(url);
    if (suffix.length > 0)
        localUrl += '.' + suffix;
    return localUrl;
  }

  /**
   * @return {string} the suffix string (sans the '.' prefix) for the
   *     given file name or the empty string if no suffix was identified
   * @param {string} name The file name
   * @private
   */
  fileSuffix(name) {
    var slashParts = name.split('/');
    var baseName = slashParts.pop();
    var parts = baseName.split('.');
    if (parts.length == 1)
        return '';
    var suffix = parts.pop();
    var questionIndex = suffix.indexOf('?');
    return (questionIndex == -1) ? suffix : suffix.slice(0, questionIndex);
  }

  /**
   * Process the src attribute of a given element.
   *
   * @param {Element} element The Element being processed, which has the src
   *     attribute.
   * @param {string} id The id of the Element being serialized.
   * @private
   */
  processSrcAttribute(element, id) {
    var win = element.ownerDocument.defaultView;
    var url = this.qualifiedUrlForElement(element);
    var sameOrigin = window.location.host == url.host;
    switch (element.tagName) {
      case 'IFRAME':
        break; // Do nothing.
      case 'SOURCE':
        var parent = element.parent;
        if (parent && parent.tagName == 'PICTURE' && sameOrigin) {
          this.processSrcHole(element);
        } else {
          this.processSimpleAttribute(win, 'src', url.href);
        }
        break;
      case 'INPUT':
        var type = element.attributes.type;
        if (type && type.value.toLowerCase() == 'image') {
          this.processSrcHole(element);
        }
        break;
      case 'IMG':
        // Our method for calculating same-origin can be incorrect in
        // the presence of data urls loaded from (for example)
        // localhost, so check for and handle them explicitly. This
        // also avoids unnecessarily processing them asynchronously.
        if (this.isImageDataUrl(url.href)) {
          // Just pass the url through as it will render directly.
          this.processSimpleAttribute(win, 'src', url.href);
        } else if (sameOrigin) {
          this.processSrcHole(element);
        } else {
          this.externalImages.push([id, url.href]);
          this.processSimpleAttribute(win, 'src',
              this.getExternalImageUrl(id, url.href));
        }
        break;
      default:
        this.processSimpleAttribute(win, 'src', url.href);
    }
  }

  /**
   * Get a URL object with a fully qualified url for the value of the
   * |element|'s src attribute.
   *
   * @param {Element} element The element for which to retrieve the URL.
   * @return {URL} The URL object.
   */
  qualifiedUrlForElement(element) {
    return this.qualifiedUrl(element.attributes.src.value);
  }

  /**
   * Get a URL object with a fully qualified url for the given raw url
   * string.
   *
   * @param {Element} element The element for which to retrieve the URL.
   * @return {URL} The URL object.
   */
  qualifiedUrl(rawUrl) {
    var anchor = document.createElement('a');
    anchor.href = rawUrl;
    // Retrieve fully qualified URL string from the anchor.
    var anchorUrl = anchor.href;
    return new URL(anchorUrl);
  }

  /**
   * Add an entry to |this.srcHoles| so it can be processed asynchronously.
   *
   * @param {Element} element The element being processed, which has the src
   *     attribute.
   * @private
   */
  processSrcHole(element) {
    var win = element.ownerDocument.defaultView;
    var valueIndex = this.processHoleAttribute(win, 'src');
    this.srcHoles[valueIndex] = this.qualifiedUrlForElement(element).href;
  }

  /**
   * Add an attribute with name |name| to |this.html| with an empty index for
   * its value that can later be filled in.
   *
   * @param {Window} win The window of the Element that is being processed.
   * @param {string} name The name of the attribute.
   * @return {number} The index in |this.html| where the value will be placed.
   */
  processHoleAttribute(win, name) {
    var quote = this.escapedCharacter('"', this.windowDepth(win));
    this.html.push(`${name}=${quote}`);
    var valueIndex = this.html.length;
    this.html.push(''); // Entry where value will go.
    this.html.push(quote + ' '); // Add a space before the next attribute.
    return valueIndex;
  }

  /**
   * Add a name and value pair to the list of attributes in |this.html|.
   *
   * @param {Window} win The window of the Element that is being processed.
   * @param {string} name The name of the attribute.
   * @param {string} value The value of the attribute.
   */
  processSimpleAttribute(win, name, value) {
    var nestingDepth = this.windowDepth(win);
    var quote = this.escapedCharacter('"', nestingDepth);
    value = this.escapedCharacterString(value, nestingDepth+1);
    this.html.push(`${name}=${quote}${value}${quote} `);
  }

  /**
   * Load all external fonts, and add an entry to |this.html| at index
   * |this.fontPlaceHolderIndex|.
   *
   * @param {Document} doc The Document being serialized.
   */
  loadFonts(doc) {
    this.fontPlaceHolderIndex = this.html.length;
    this.html.push(''); // Entry where the font style tag will go.
    for (var i = 0, styleSheet; styleSheet = doc.styleSheets[i]; i++) {
      if (styleSheet.cssRules) {
        for (var j = 0, rule; rule = styleSheet.cssRules[j]; j++) {
          this.processCSSFonts(doc.defaultView, styleSheet.href, rule.cssText);
        }
      } else {
        this.crossOriginStyleSheets.push(styleSheet.href);
      }
    }
  }

  /**
   * Simple convenience method to wrap a raw url string with the
   * required CSS uri syntax.
   *
   * @param {string} url The raw url string.
   * @return {string} the wrapped url string.
   * @private
   */
  wrapUrl(url) {
    return 'url("' + url + '")';
  }

  /**
   * Takes a string representing CSS and parses it to find any fonts that are
   * declared.  If any fonts are declared, it processes them so that they
   * can be used in the serialized document and adds them to |this.fontCSS|.
   *
   * @param {Window} win The Window of the Document being serialized.
   * @param {string} href The url at which the CSS stylesheet is located.
   * @param {string} css The CSS text.
   */
  processCSSFonts(win, href, css) {
    var serializer = this;
    var fonts = css.match(/@font-face *?{[\s\S]*?}/g);
    if (fonts) {
      var nestingDepth = this.windowDepth(win);
      var escapedQuote = this.escapedCharacter('"', nestingDepth);
      for (var i = 0; i < fonts.length; i++) {
        // Convert url specified in font to fully qualified url.
        var font = fonts[i].replace(/url\("(.*?)"\)/g, function(match, url) {
          // If href is null the url must be a fully qualified url.
          url = href ? serializer.qualifiedFontUrl(href, url) : url;
          return serializer.wrapUrl(url);
        }).
        replace(/"/g, escapedQuote);
        this.fontCSS.push(font);
      }
    }
  }

  /**
   * Computes the fully qualified url at which a font can be loaded.
   * TODO(sfine): Make this method sufficiently robust, so that it can replace
   *              the current implementation of qualifiedUrl.
   *
   * @param {string} href The url at which the CSS stylesheet containing the
   *     font is located.
   * @param {string} url The url listed in the font declaration.
   */
  qualifiedFontUrl(href, url) {
    if (href.charAt(href.length-1) == '/') {
      href = href.slice(0, href.length-1);
    }
    var hrefUrl = new URL(href);
    if (url.includes('://')) {
      return url;
    } else if (url.startsWith('//')) {
      return hrefUrl.protocol + url;
    } else if (url.startsWith('/')) {
      return hrefUrl.origin + url;
    } else {
      href = href.slice(0, href.lastIndexOf('/'));
      return href + '/' + url;
    }
  }

  /**
   * Computes the index of the window in its parent's array of frames.
   *
   * @param {Window} childWindow The window to use in the calculation.
   * @return {number} the frames index.
   */
  iframeIndex(childWindow) {
    if (childWindow.parent != childWindow) {
      for (var i = 0; i < childWindow.parent.frames.length; i++) {
        if (childWindow.parent.frames[i] == childWindow) {
          return i;
        }
      }
    } else {
      return -1;
    }
  }

  /**
   * Computes the fully qualified path of the frame in the root
   * document. Nested layers are separated by '.'.
   *
   * @param {Window} win The window to use in the calculation.
   * @return {string} The full path.
   */
  iframeQualifiedName(win) {
    if (this.iframeIndex(win) < 0) {
      return '0';
    } else {
      var qualifiedName = this.iframeQualifiedName(win.parent);
      var index = this.iframeIndex(win);
      return qualifiedName + '.' + index;
    }
  }

  /**
   * Calculate the correct encoding of a character that should be used given the
   * nesting depth of the window in the frame tree.
   *
   * @param {string} char The character that should be escaped.
   * @param {number} depth The nesting depth of the appropriate window in the
   *     frame tree.
   * @return {string} The correctly escaped string.
   */
  escapedCharacter(char, depth) {
    if (depth == 0) {
      return char;
    } else {
      var arr = 'amp;'.repeat(depth-1);
      return '&' + arr + this.CHARACTER_ESCAPING_MAP[char].slice(1);
    }
  }

  /**
   * Returns the string that is passed as an argument with all characters in
   * |this.ESCAPED_CHARACTER_MAP| replaced with the correct character encoding
   * that should be used, given the nesting depth of the window in the frame
   * tree.
   *
   * @param {string} str The string that should have its characters escaped.
   * @param {number} depth The nesting depth of the appropriate window in the
   *     frame tree.
   * @return {string} The correctly escaped string.
   */
  escapedCharacterString(str, depth) {
    // Some escaping introduces '&' characters so we escape '&' first to prevent
    // escaping the '&' added by other escape substitutions.
    str = str.replace(/&/g, this.escapedCharacter('&', depth));
    for (var char in this.CHARACTER_ESCAPING_MAP) {
      if (char != '&') {
        var regExp = new RegExp(char, 'g');
        str = str.replace(regExp, this.escapedCharacter(char, depth));
      }
    }
    return str;
  }

  /**
   * Returns the string that is passed as an argument with all non ascii unicode
   * characters escaped.
   *
   * @param {string} str The string that should have its characters escaped.
   * @param {number} textType A possible value of |this.INPUT_TEXT_TYPE| which
   *     represents the type of text being escaped.
   * @return {string} The correctly escaped string.
   */
  escapedUnicodeString(str, textType) {
    var serializer = this;
    return str.replace(/[\s\S]/g, function(char) {
      var unicode = char.codePointAt();
      if (unicode < 128) {
        return char;
      } else if (textType == serializer.INPUT_TEXT_TYPE.HTML) {
        return '&#' + unicode + ';';
      } else {
        return '\\' + unicode.toString(16);
      }
    });
  }

  /**
   * Calculate the nesting depth of a window in the frame tree.
   *
   * @param {Window} win The window to use in the calculation.
   * @return {number} The nesting depth of the window in the frame trees.
   */
  windowDepth(win) {
    return this.iframeQualifiedName(win).split('.').length - 1;
  }

  /**
   * Return a JSONizable dictionary with instance data for export.
   */
  asDict() {
    var result = {
      'frameIndex': htmlSerializer.iframeQualifiedName(window),
      'unusedId': htmlSerializer.generateId(document)
    };
    var copyFields = [
      'externalImages',
      'frameHoles',
      'html',
      'idToStyleIndex',
      'idToStyleMap',
      'pseudoElementPlaceHolderIndex',
      'pseudoElementSelectorToCSSMap',
      'pseudoElementTestingStyleId',
      'pseudoElementTestingStyleIndex',
      'rootId',
      'rootStyleIndex',
      'windowHeight',
      'windowWidth',
    ];
    for (var i = 0; i  < copyFields.length; i++) {
      var fieldName = copyFields[i];
      result[fieldName] = htmlSerializer[fieldName];
    }
    return result;
  }

  /**
   * Create a function that will generate strings which can be used as
   * ids.
   *
   * @return {Function<Document>} A funtion that generates a valid id each time
   *     it is called.
   */
  generateIdGenerator() {
    var counter = 0;
    function idGenerator(doc) {
      var id;
      do {
        id = 'snap-it' + counter++;
      } while (doc.getElementById(id));
      return id;
    }
    return idGenerator;
  }

  /**
   * Asynchronously fill in any holes in |this.html|.
   *
   * @param {Document} doc The Document being serialized.
   * @param {Function} callback The callback function, which will be called when
   *     all asynchronous processing is finished.
   */
  fillHolesAsync(doc, callback) {
    var serializer = this;
    this.fillFontHoles(doc, function() {
      serializer.fillSrcHoles(callback);
    });
  }

  /**
   * Takes all of the cross origin stylesheets, processes their font
   * declarations, and adds them to |this.html|. Calls the callback when
   *     complete.
   *
   * @param {Document} doc The Document being serialized.
   * @param {Function} callback The callback function.
   */
  fillFontHoles(doc, callback) {
    if (this.crossOriginStyleSheets.length == 0) {
      if (this.fontCSS.length > 0) {
        var fonts = `<style>${this.fontCSS.join('')}</style>`;
        this.html[this.fontPlaceHolderIndex] = fonts;
      }
      callback();
    } else {
      var styleSheetSrc = this.crossOriginStyleSheets.shift();
      var serializer = this;
      fetch(styleSheetSrc).then(function(response) {
        return response.text();
      }).then(function(css) {
          serializer.processCSSFonts(doc.defaultView, styleSheetSrc, css);
          serializer.fillFontHoles(doc, callback);
      }).catch(function(error) {
        console.log(error);
        serializer.fillFontHoles(doc, callback);
      });
    }
  }

  /**
   * Take all of the srcHoles and create data urls for the resources, placing
   * them in |this.html|. Calls the callback when complete.
   *
   * @param {Function} callback The callback function.
   */
  fillSrcHoles(callback) {
    if (Object.keys(this.srcHoles).length == 0) {
      callback(this);
    } else {
      var index = Object.keys(this.srcHoles)[0];
      var src = this.srcHoles[index];
      delete this.srcHoles[index];
      var serializer = this;
      fetch(src).then(function(response) {
        return response.blob();
      }).then(function(blob) {
        var reader = new FileReader();
        reader.onload = function(e) {
          serializer.html[index] = e.target.result;
          serializer.fillSrcHoles(callback);
        }
        reader.readAsDataURL(blob);
      }).catch(function(error) {
        console.log(error);
        serializer.fillSrcHoles(callback);
      });
    }
  }

  /**
   * Sets the path to the directory under which any external images
   * found in the page will be fetched and stored locally. If
   * non-null, the urls for any such images will be rewritten to be
   * relative to this path.
   *
   * @param {string=} opt_path The directory path.
   */
  setLocalImagePath(opt_path) {
    this.localImagePath = opt_path;
  }
}
