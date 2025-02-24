window.allTestDone = false;
window.failedTests = [];

QUnit.testDone(function(details) {
  if (details.failed !== 0) {
    window.failedTests.push(details.module + details.name);
  }
});

QUnit.done(function(details) {
  window.total = details.total;
  window.allTestDone = true;
});

QUnit.test('windowDepth: no parent window', function(assert) {
  var serializer = new HTMLSerializer();
  assert.equal(serializer.windowDepth(window), 0);
});

QUnit.test('windowDepth: multiple parent windows', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame = document.createElement('iframe');
  fixture.appendChild(childFrame);
  var childFrameBody = childFrame.contentDocument.body;
  var grandChildFrame = document.createElement('iframe');
  childFrameBody.appendChild(grandChildFrame);
  assert.equal(serializer.windowDepth(childFrame.contentWindow), 1);
  assert.equal(serializer.windowDepth(grandChildFrame.contentWindow), 2);
});

QUnit.test('escapedCharacter: zero depth', function(assert) {
  var serializer = new HTMLSerializer();
  assert.equal(serializer.escapedCharacter('"', 0), '"');
  assert.equal(serializer.escapedCharacter("'", 0), "'");
  assert.equal(serializer.escapedCharacter('<', 0), '<');
  assert.equal(serializer.escapedCharacter('>', 0), '>');
  assert.equal(serializer.escapedCharacter('&', 0), '&');
});

QUnit.test('escapedCharacter: nonzero depth', function(assert) {
  var serializer = new HTMLSerializer();
  assert.equal(serializer.escapedCharacter('"', 1), '&quot;');
  assert.equal(serializer.escapedCharacter('"', 2), '&amp;quot;');
  assert.equal(serializer.escapedCharacter("'", 1), '&#39;');
  assert.equal(serializer.escapedCharacter("'", 2), '&amp;#39;');
  assert.equal(serializer.escapedCharacter('<', 1), '&lt;');
  assert.equal(serializer.escapedCharacter('<', 2), '&amp;lt;');
  assert.equal(serializer.escapedCharacter('>', 1), '&gt;');
  assert.equal(serializer.escapedCharacter('>', 2), '&amp;gt;');
  assert.equal(serializer.escapedCharacter('&', 1), '&amp;');
  assert.equal(serializer.escapedCharacter('&', 2), '&amp;amp;');
});

QUnit.test('iframeIndex: single layer', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame1 = document.createElement('iframe');
  var childFrame2 = document.createElement('iframe');
  fixture.appendChild(childFrame1);
  fixture.appendChild(childFrame2);
  assert.equal(serializer.iframeIndex(window), -1);
  assert.equal(serializer.iframeIndex(childFrame1.contentWindow), 0);
  assert.equal(serializer.iframeIndex(childFrame2.contentWindow), 1);
});

QUnit.test('iframeIndex: multiple layers', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame = document.createElement('iframe');
  var grandChildFrame1 = document.createElement('iframe');
  var grandChildFrame2 = document.createElement('iframe');
  fixture.appendChild(childFrame);
  var childFrameBody = childFrame.contentDocument.body;
  childFrameBody.appendChild(grandChildFrame1);
  childFrameBody.appendChild(grandChildFrame2);
  assert.equal(serializer.iframeIndex(grandChildFrame1.contentWindow), 0);
  assert.equal(serializer.iframeIndex(grandChildFrame2.contentWindow), 1);
});

QUnit.test('iframeQualifiedName: single layer', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame1 = document.createElement('iframe');
  var childFrame2 = document.createElement('iframe');
  fixture.appendChild(childFrame1);
  fixture.appendChild(childFrame2);
  assert.equal(serializer.iframeQualifiedName(window), '0');
  assert.equal(
      serializer.iframeQualifiedName(childFrame1.contentWindow),
      '0.0');
  assert.equal(
      serializer.iframeQualifiedName(childFrame2.contentWindow),
      '0.1');
});

QUnit.test('iframeQualifiedName: multiple layers', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame = document.createElement('iframe');
  var grandChildFrame1 = document.createElement('iframe');
  var grandChildFrame2 = document.createElement('iframe');
  fixture.appendChild(childFrame);
  var childFrameBody = childFrame.contentDocument.body;
  childFrameBody.appendChild(grandChildFrame1);
  childFrameBody.appendChild(grandChildFrame2);
  assert.equal(
      serializer.iframeQualifiedName(grandChildFrame1.contentWindow),
      '0.0.0');
  assert.equal(
      serializer.iframeQualifiedName(grandChildFrame2.contentWindow),
      '0.0.1');
});

QUnit.test('processTree: background-image with no local rewrite', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createElement('div');
  node.id = 'targetId';
  node.style.backgroundImage = "url('https://www.images.com/foo.png')";
  node.appendChild(document.createTextNode('test'));
  fixture.appendChild(node);
  var serializer = new HTMLSerializer();
  serializer.processTree(node);
  assert.deepEqual(serializer.externalImages, [['targetId',
      'https://www.images.com/foo.png']]);
  assert.equal(serializer.idToStyleMap['targetId']['background-image'],
      'url("https://www.images.com/foo.png")');
});

QUnit.test('processTree: background-image with local rewrite', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createElement('div');
  node.id = 'targetId';
  node.style.backgroundImage = "url('https://www.images.com/foo.png')";
  node.appendChild(document.createTextNode('test'));
  fixture.appendChild(node);
  var serializer = new HTMLSerializer();
  serializer.setLocalImagePath('local/');
  serializer.processTree(node);
  assert.deepEqual(serializer.externalImages, [['targetId',
      'https://www.images.com/foo.png']]);
  assert.equal(serializer.idToStyleMap['targetId']['background-image'],
      'url("local/targetId.png")');
});

QUnit.test('processTree: background-image with data url and local rewrite', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createElement('div');
  node.id = 'targetId';
  node.style.backgroundImage = "url('data:image/gif;base64,R0lGODlhAQABAIAAAA" +
      "UEBAAAACwAAAAAAQABAAACAkQBADs=')";
  node.appendChild(document.createTextNode('test'));
  fixture.appendChild(node);
  var serializer = new HTMLSerializer();
  serializer.setLocalImagePath('local/');
  serializer.processTree(node);
  assert.equal(0, serializer.externalImages.length);
  assert.equal(serializer.idToStyleMap['targetId']['background-image'],
      'url("data:image/gif;base64,R0lGODlhAQABAIAAAA' +
      'UEBAAAACwAAAAAAQABAAACAkQBADs=")');
});

QUnit.test('processTree: background-image with multiple images and local rewrite',
  function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createElement('div');
  node.id = 'targetId';
  node.style.backgroundImage = 'url("https://www.images.com/foo.png"),' +
      'url("https://www.images.com/bar.png")';
  node.appendChild(document.createTextNode('test'));
  fixture.appendChild(node);
  var serializer = new HTMLSerializer();
  serializer.setLocalImagePath('local/');
  serializer.processTree(node);
  // TODO(wkorman): Currently this produces not-useful output (two
  // entries with the same rewritten file name) as we don't support
  // having multiple external images associated with a single element
  // id. Look at tacking an optional incrementing per-element image
  // number onto the name (which will need to be passed along in
  // externalImages as well somehow).
  assert.deepEqual(serializer.externalImages, [['targetId',
      'https://www.images.com/foo.png'],
      ['targetId', 'https://www.images.com/bar.png']]);
  assert.equal(serializer.idToStyleMap['targetId']['background-image'],
      'url("local/targetId.png"),url("local/targetId.png")');
});

QUnit.test('processTree: background-image with complex multiple images and local rewrite',
  function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createElement('div');
  node.id = 'targetId';
  node.style.backgroundImage = 'url("https://www.images.com/foo.png"),' +
      'url("data:image/gif;base64,R0lGODlhAQABAIAAAA"),' +
      'url("https://www.images.com/bar.png"),' +
      'linear-gradient(to right top,red,rgb(240, 109, 6)),' +
      'url("https://www.images.com/baz.png")';
  node.appendChild(document.createTextNode('test'));
  fixture.appendChild(node);
  var serializer = new HTMLSerializer();
  serializer.setLocalImagePath('local/');
  serializer.processTree(node);
  // TODO(wkorman): The actual external images produce duplicate
  // filenames for same reason as above unit test. However, we should
  // successfully pass through the other urls.
  assert.deepEqual(serializer.externalImages, [
      ['targetId', 'https://www.images.com/foo.png'],
      ['targetId', 'https://www.images.com/bar.png'],
      ['targetId', 'https://www.images.com/baz.png']]);
  assert.equal(serializer.idToStyleMap['targetId']['background-image'],
               'url("local/targetId.png"),' +
               'url("data:image/gif;base64,R0lGODlhAQABAIAAAA"),' +
               'url("local/targetId.png"),' +
               'linear-gradient(to right top,rgb(255,0,0),rgb(240,109,6)),' +
               'url("local/targetId.png")');
});

QUnit.test('processSimpleAttribute: top window', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var win = fixture.ownerDocument.defaultView;
  serializer.processSimpleAttribute(win, 'height', '5');
  assert.equal(serializer.html[0], 'height="5" ');
});

QUnit.test('processSimpleAttribute: nested window', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame = document.createElement('iframe');
  var grandChildFrame = document.createElement('iframe');
  fixture.appendChild(childFrame);
  var childFrameBody = childFrame.contentDocument.body;
  childFrameBody.appendChild(grandChildFrame);
  var childFrameWindow = childFrame.contentDocument.defaultView;
  var grandChildFrameWindow = grandChildFrame.contentDocument.defaultView;
  serializer.processSimpleAttribute(childFrameWindow, 'height', '5');
  serializer.processSimpleAttribute(grandChildFrameWindow, 'width', '2');
  assert.equal(serializer.html[0], 'height=&quot;5&quot; ');
  assert.equal(serializer.html[1], 'width=&amp;quot;2&amp;quot; ');
});

QUnit.test('processHoleAttribute: top window', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var win = fixture.ownerDocument.defaultView;
  var valueIndex = serializer.processHoleAttribute(win, 'height');
  assert.equal(valueIndex, 1);
  assert.equal(serializer.html[0], 'height="');
  assert.equal(serializer.html[2], '" ');
});

QUnit.test('processHoleAttribute: nested window', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame = document.createElement('iframe');
  var grandChildFrame = document.createElement('iframe');
  fixture.appendChild(childFrame);
  var childFrameBody = childFrame.contentDocument.body;
  childFrameBody.appendChild(grandChildFrame);
  var childFrameWindow = childFrame.contentDocument.defaultView;
  var grandChildFrameWindow = grandChildFrame.contentDocument.defaultView;
  var childValueIndex = serializer.processHoleAttribute(
      childFrameWindow,
      'height');
  var grandChildValueIndex = serializer.processHoleAttribute(
      grandChildFrameWindow,
      'width');
  assert.equal(childValueIndex, 1);
  assert.equal(serializer.html[0], 'height=&quot;');
  assert.equal(serializer.html[2], '&quot; ');
  assert.equal(grandChildValueIndex, 4);
  assert.equal(serializer.html[3], 'width=&amp;quot;');
  assert.equal(serializer.html[5], '&amp;quot; ');
});

QUnit.test('qualifiedUrlForElement', function(assert) {
  var serializer = new HTMLSerializer();
  var iframe = document.createElement('iframe');
  iframe.setAttribute('src', 'page.html');
  var url = serializer.qualifiedUrlForElement(iframe);
  var href = window.location.href;
  var path = href.slice(0, href.lastIndexOf('/'));
  assert.equal(path + '/page.html', url.href);
});

QUnit.test('qualifiedUrl', function(assert) {
  var serializer = new HTMLSerializer();
  var url = serializer.qualifiedUrl('page.html');
  var href = window.location.href;
  var path = href.slice(0, href.lastIndexOf('/'));
  assert.equal(path + '/page.html', url.href);
});

QUnit.test('processSrcHole: top window', function(assert) {
  var serializer = new HTMLSerializer();
  var iframe = document.createElement('iframe');
  iframe.setAttribute('src', 'tests.html');
  serializer.processSrcHole(iframe);
  assert.equal(serializer.html[0], 'src="');
  assert.equal(serializer.html[1], '');
  assert.equal(serializer.html[2], '" ');
  assert.equal(serializer.srcHoles[1], window.location.href);
  assert.equal(Object.keys(serializer.srcHoles).length, 1);
});

QUnit.test('processSrcAttribute: iframe', function(assert) {
  var serializer = new HTMLSerializer();
  var iframe = document.createElement('iframe');
  iframe.setAttribute('src', 'tests.html');
  serializer.processSrcAttribute(iframe, 'dummyId');
  assert.equal(serializer.html.length, 0);
  assert.equal(Object.keys(serializer.srcHoles).length, 0);
});

QUnit.test('processSrcAttribute: audio', function(assert) {
  var serializer = new HTMLSerializer();
  var audio = document.createElement('audio');
  audio.setAttribute('src', 'tests.html');
  serializer.processSrcAttribute(audio, 'dummyId');
  assert.equal(serializer.html[0], `src="${window.location.href}" `);
  assert.equal(serializer.html.length, 1);
  assert.equal(Object.keys(serializer.srcHoles).length, 0);
});

QUnit.test('processSrcAttribute: img', function(assert) {
  var serializer = new HTMLSerializer();
  var img = document.createElement('img');
  img.setAttribute('src', 'tests.html');
  serializer.processSrcAttribute(img, 'dummyId');
  assert.equal(serializer.html[0], 'src="');
  assert.equal(serializer.html[1], '');
  assert.equal(serializer.html[2], '" ');
  assert.equal(serializer.srcHoles[1], window.location.href);
  assert.equal(Object.keys(serializer.srcHoles).length, 1);
});

QUnit.test('processSrcAttribute: img (external)', function(assert) {
  var serializer = new HTMLSerializer();
  var img = document.createElement('img');
  img.setAttribute('src', 'https://www.images.com/foo.png');
  serializer.processSrcAttribute(img, 'targetId');
  assert.deepEqual(serializer.externalImages, [['targetId',
      'https://www.images.com/foo.png']]);
});

QUnit.test('processSrcAttribute: img (external) with local rewrite',
    function(assert) {
  var serializer = new HTMLSerializer();
  serializer.setLocalImagePath('local/');
  var img = document.createElement('img');
  img.setAttribute('src', 'https://www.images.com/foo.png');
  serializer.processSrcAttribute(img, 'targetId');
  assert.deepEqual(serializer.externalImages, [['targetId',
      'https://www.images.com/foo.png']]);
  assert.equal(serializer.html[0], 'src="local/targetId.png" ');
});

QUnit.test('processTree: single node', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createElement('div');
  node.appendChild(document.createTextNode('test'));
  fixture.appendChild(node);
  var serializer = new HTMLSerializer();
  serializer.processTree(node);
  assert.equal(Object.keys(serializer.srcHoles).length, 0);
  assert.equal(Object.keys(serializer.frameHoles).length, 0);
});

QUnit.test('HTMLSerializer class loaded twice', function(assert) {
  assert.expect(0);
  var done = assert.async();
  var fixture = document.getElementById('qunit-fixture');
  var script1 = document.createElement('script');
  var script2 = document.createElement('script');
  script1.setAttribute('src', '../HTMLSerializer.js');
  script2.setAttribute('src', '../HTMLSerializer.js');
  fixture.appendChild(script1);
  fixture.appendChild(script2);
  setTimeout(function() {
    done();
  }, 0);
});

QUnit.test('processTree: no closing tag', function(assert) {
  var serializer = new HTMLSerializer();
  var img = document.createElement('img');
  serializer.processTree(img);
  assert.equal(serializer.html[0], '<img ');
  assert.equal(
      serializer.html[1],
      `style="${window.getComputedStyle(img, null).cssText}" `);
  assert.equal(serializer.html[3], '>')
  assert.equal(serializer.html.length, 4);
});

QUnit.test('processTree: closing tag', function(assert) {
  var serializer = new HTMLSerializer();
  var p = document.createElement('p');
  serializer.processTree(p);
  assert.equal(serializer.html[0], '<p ');
  assert.equal(
      serializer.html[1],
      `style="${window.getComputedStyle(p, null).cssText}" `);
  assert.equal(serializer.html[3], '>');
  assert.equal(serializer.html[4], '</p>');
  assert.equal(serializer.html.length, 5);
});

// TODO(crbug.com/1315424): Fix and re-enable test to pass on all trybots.
QUnit.skip(
  'processAttributes: img with height and width attributes',
  function(assert) {
    var serializer = new HTMLSerializer();
    var fixture = document.getElementById('qunit-fixture');
    var img = document.createElement('img');
    img.setAttribute('height', 5);
    img.setAttribute('width', 5);
    fixture.appendChild(img);
    serializer.processAttributes(img, 'id');
    var styleText = serializer.html[0];
    assert.ok(styleText.includes(' height: 5px;'));
    assert.ok(styleText.includes(' width: 5px;'));
  }
);

// TODO(crbug.com/1315424): Fix and re-enable test to pass on all trybots.
QUnit.skip(
  'processAttributes: img without height and width attributes',
  function(assert) {
    var serializer = new HTMLSerializer();
    var fixture = document.getElementById('qunit-fixture');
    var img = document.createElement('img');
    fixture.appendChild(img);
    var style = window.getComputedStyle(img, null);
    serializer.processAttributes(img, 'id');
    var styleText = serializer.html[0];
    assert.ok(styleText.includes(` height: ${style.height};`));
    assert.ok(styleText.includes(` width: ${style.width};`));
  }
);

// TODO(crbug.com/1315424): Fix and re-enable test to pass on all trybots.
QUnit.skip(
  'processAttributes: img with height and width attributes and inline style',
  function(assert) {
    var serializer = new HTMLSerializer();
    var fixture = document.getElementById('qunit-fixture');
    var img = document.createElement('img');
    img.setAttribute('height', 5);
    img.setAttribute('width', 5);
    img.setAttribute('style', 'height: 10px; width: 10px;');
    fixture.appendChild(img);
    serializer.processAttributes(img, 'id');
    var styleText = serializer.html[0];
    assert.ok(styleText.includes(' height: 10px;'));
    assert.ok(styleText.includes(' width: 10px;'));
    assert.notOk(styleText.includes(' height: 5px;'));
    assert.notOk(styleText.includes(' width: 5px;'));
  }
);

QUnit.test('processAttributes: img with data url', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var img = document.createElement('img');
  img.setAttribute('height', 1);
  img.setAttribute('width', 1);
  var dataUrl = 'data:image/gif;base64,R0lGODlhAQABAIAAAA' +
      'UEBAAAACwAAAAAAQABAAACAkQBADs=';
  img.setAttribute('src', dataUrl);
  fixture.appendChild(img);
  serializer.processAttributes(img, 'id');
  assert.ok(serializer.html[4].includes(dataUrl));
});

QUnit.test('isImageDataUrl', function(assert) {
  var serializer = new HTMLSerializer();
  assert.ok(serializer.isImageDataUrl('data:image/png'));
  assert.ok(serializer.isImageDataUrl('data:image/png;base64,'));
  assert.ok(serializer.isImageDataUrl('data:image/png;base64,F00='));
  assert.ok(serializer.isImageDataUrl('data:image/jpg'));
  assert.ok(serializer.isImageDataUrl('data:image/j'));
  assert.notOk(serializer.isImageDataUrl('data:image'));
  assert.notOk(serializer.isImageDataUrl('data:image/'));
  assert.notOk(serializer.isImageDataUrl(''));
  assert.notOk(serializer.isImageDataUrl('data:'));
  assert.notOk(serializer.isImageDataUrl('foo'));
});

QUnit.test('processText: simple text node', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createTextNode('Simple text');
  fixture.appendChild(node);
  serializer.processText(node);
  assert.equal(serializer.html[0], 'Simple text');
});

QUnit.test('processText: escaped characters', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var node = document.createTextNode(`<div> with '&"`);
  fixture.appendChild(node);
  serializer.processText(node);
  assert.equal(
      serializer.html[0],
      '&lt;div&gt; with &#39;&amp;&quot;');
});

QUnit.test('processText: nested escaped characters', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var childFrame = document.createElement('iframe');
  var grandChildFrame = document.createElement('iframe');
  fixture.appendChild(childFrame);
  var childFrameBody = childFrame.contentDocument.body;
  childFrameBody.appendChild(grandChildFrame);
  var grandChildFrameBody = grandChildFrame.contentDocument.body;
  var node1 = document.createTextNode(`<div> with '&"`);
  var node2 = document.createTextNode(`<div> with '&"`);
  childFrameBody.appendChild(node1);
  grandChildFrameBody.appendChild(node2);
  serializer.processText(node1);
  serializer.processText(node2);
  assert.equal(
      serializer.html[0],
      '&amp;lt;div&amp;gt; with &amp;#39;&amp;amp;&amp;quot;');
  assert.equal(
      serializer.html[1],
      '&amp;amp;lt;div&amp;amp;gt; with &amp;amp;#39;&amp;amp;amp;&amp;amp;quot;');
});

QUnit.test('processPseudoElements', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var element = document.createElement('div');
  var style = document.createElement('style');
  style.appendChild(document.createTextNode('div::before{content:"test";}'));
  fixture.appendChild(style);
  fixture.appendChild(element);
  serializer.processPseudoElements(element, 'myId');
  var styleText = window.getComputedStyle(element, ':before').cssText;
  assert.equal(serializer.pseudoElementCSS.length, 1);
  assert.equal(serializer.pseudoElementCSS[0], `#myId::before{${styleText}} `);
});

QUnit.test('processTree: element without id', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var element = document.createElement('div');
  fixture.appendChild(element);
  serializer.processTree(element);
  assert.equal(serializer.html[0], '<div ');
  assert.equal(serializer.html[2], 'id="snap-it0" ');
  assert.equal(serializer.html[3], '>');
});

QUnit.test('processTree: element with id', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var element = document.createElement('div');
  element.setAttribute('id', 'myId');
  fixture.appendChild(element);
  serializer.processTree(element);
  assert.equal(serializer.html[0], '<div ');
  assert.equal(serializer.html[2], 'id="myId" ');
  assert.equal(serializer.html[3], '>');
});

QUnit.test('processTree: generated id exists', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var span = document.createElement('span');
  span.setAttribute('id', 'snap-it0');
  fixture.appendChild(span);
  var element = document.createElement('div');
  fixture.appendChild(element);
  serializer.processTree(element);
  assert.equal(serializer.html[0], '<div ');
  assert.equal(serializer.html[2], 'id="snap-it1" ');
  assert.equal(serializer.html[3], '>');
});

QUnit.test('generateIdGenerator', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  fixture.appendChild(div);
  var generateId = serializer.generateIdGenerator();
  assert.equal(generateId(document), 'snap-it0');
  assert.equal(generateId(document), 'snap-it1');
  div.setAttribute('id', 'snap-it2');
  assert.equal(generateId(document), 'snap-it3');
});

QUnit.test('escapedUnicodeString: html', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.appendChild(document.createTextNode('i \u2665 \u0073f'));
  var string = div.childNodes[0].textContent;
  assert.equal(
      serializer.escapedUnicodeString(string, serializer.INPUT_TEXT_TYPE.HTML),
      'i &#9829; sf');
});

QUnit.test('escapedUnicodeString: css', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.appendChild(document.createTextNode('i \u2665 \u0073f'));
  var string = div.childNodes[0].textContent;
  assert.equal(
      serializer.escapedUnicodeString(string, serializer.INPUT_TEXT_TYPE.CSS),
      'i \\2665 sf');
});

QUnit.test('qualifiedFontUrl', function(assert) {
  var serializer = new HTMLSerializer();
  var href = 'http://www.example.com/path/page/';
  var url1 = '/hello/world/';
  assert.equal(
      serializer.qualifiedFontUrl(href, url1),
      'http://www.example.com/hello/world/');
  var url2 = './hello/world/';
  assert.equal(
      serializer.qualifiedFontUrl(href, url2),
      'http://www.example.com/path/./hello/world/');
  var url3 = '../hello/world/';
  assert.equal(
      serializer.qualifiedFontUrl(href, url3),
      'http://www.example.com/path/../hello/world/');
  var url4 = 'http://www.google.com/';
  assert.equal(
      serializer.qualifiedFontUrl(href, url4),
      'http://www.google.com/');
  var url5 = 'hello/world/';
  assert.equal(
      serializer.qualifiedFontUrl(href, url5),
      'http://www.example.com/path/hello/world/');
});

QUnit.test('processCSSFonts: no line breaks in declaration', function(assert) {
  var serializer = new HTMLSerializer();
  var cssText = 'body{color:red;}' +
      '@font-face{font-family:Font;src:url("/hello/")}';
  var href = 'http://www.example.com/';
  serializer.processCSSFonts(window, href, cssText);
  assert.equal(
      serializer.fontCSS[0],
      '@font-face{font-family:Font;src:url("http://www.example.com/hello/")}');
});

QUnit.test('processCSSFonts: line breaks in declaration', function(assert) {
  var serializer = new HTMLSerializer();
  var cssText = 'body{color:red;}' +
      '@font-face { font-family:Font;\nsrc:url("/goodbye/")}';
  var href = 'http://www.url.com/';
  serializer.processCSSFonts(window, href, cssText);
  assert.equal(
      serializer.fontCSS[0],
      '@font-face { font-family:Font;\nsrc:url("http://www.url.com/goodbye/")}');
});

// https://github.com/catapult-project/catapult/issues/4233
QUnit.skip('loadFonts', function(assert) {
  var serializer = new HTMLSerializer();
  serializer.loadFonts(document);
  assert.equal(serializer.html[0], '');
  assert.equal(serializer.fontPlaceHolderIndex, 0);
  assert.equal(
      serializer.crossOriginStyleSheets[0],
      'https://code.jquery.com/qunit/qunit-2.0.0.css');
});

QUnit.test('escapedCharacterString', function(assert) {
  var serializer =  new HTMLSerializer();
  var str = serializer.escapedCharacterString(`hello &>'<& "`, 2);
  assert.equal(
      str,
      'hello &amp;amp;&amp;gt;&amp;#39;&amp;lt;&amp;amp; &amp;quot;');
});

QUnit.test('unescapeHTML', function(assert) {
  var html = '&amp;lt;div&amp;gt;&amp;lt;/div&amp;gt;';
  var unescapedHTML = unescapeHTML(html, 2);
  assert.equal(unescapedHTML, '<div></div>');
});

QUnit.test('minimizeStyles', function(assert) {
  var message = {
    'html': [
        '<div id="myId"',
        'style="animation-delay: 0s; width: 5px;" ',
        '></div>'
    ],
    'frameHoles': null,
    'idToStyleIndex': {"myId": 1},
    'idToStyleMap': {
      'myId': {
        'animation-delay': '0s',
        'width': '5px'
      }
    },
    'windowHeight': 5,
    'windowWidth': 5,
    'frameIndex': '0'
  };
  minimizeStyles(message);
  assert.equal(message.html[1], 'style="width: 5px;" ');
});

QUnit.test('minimizeStyle', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.setAttribute('id', 'myId');
  div.setAttribute('style', 'animation-delay: 0s; width: 5px;');
  fixture.appendChild(div);
  var message = {
    'html': [
        '<div id="myId"',
        'style="animation-delay: 0s; width: 5px;" ',
        '></div>'
    ],
    'frameHoles': null,
    'idToStyleIndex': {"myId": 1},
    'idToStyleMap': {
      'myId': {
        'animation-delay': '0s',
        'width': '5px'
      }
    },
    'windowHeight': 5,
    'windowWidth': 5,
    'frameIndex': '0'
  };
  minimizeStyle(message, document, div, 'myId', 1);
  assert.equal(message.html[1], 'style="width: 5px;" ');
});

QUnit.test('serialize tree: end-to-end, no style', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var iframe = document.createElement('iframe');
  fixture.appendChild(iframe);
  var div = document.createElement('div');
  div.appendChild(document.createTextNode('hello world'));
  iframe.contentDocument.body.appendChild(div);
  serializer.processTree(div);
  var win = div.ownerDocument.defaultView;
  var message = {
    'html': serializer.html,
    'frameHoles': serializer.frameHoles,
    'idToStyleIndex': serializer.idToStyleIndex,
    'idToStyleMap': {
      'snap-it0': {
        'animation-delay': '0s'
      }
    },
    'windowHeight': serializer.windowHeight,
    'windowWidth': serializer.windowWidth,
    'frameIndex': serializer.iframeQualifiedName(win)
  };
  var html = unescapeHTML(outputHTMLString([message]), 1);
  assert.equal(html, '<div id="snap-it0" >hello world</div>');
});

QUnit.test('serialize tree: end-to-end, style', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var iframe = document.createElement('iframe');
  fixture.appendChild(iframe);
  var div = document.createElement('div');
  div.appendChild(document.createTextNode('hello world'));
  iframe.contentDocument.body.appendChild(div);
  var style = document.createElement('style');
  style.innerHTML = 'div { border: 1px solid blue; }';
  iframe.contentDocument.body.appendChild(style);
  serializer.processTree(div);
  var win = div.ownerDocument.defaultView;
  var message = {
    'html': serializer.html,
    'frameHoles': serializer.frameHoles,
    'idToStyleIndex': serializer.idToStyleIndex,
    'idToStyleMap': {
      'snap-it0': {
        'animation-delay': '0s',
        'border-bottom-color': 'rgb(0, 0, 255)',
        'border-bottom-style': 'solid',
        'border-bottom-width': '4px',
        'border-left-color': 'rgb(0, 0, 255)',
        'border-left-style': 'solid',
        'border-left-width': '4px',
        'border-right-color': 'rgb(0, 0, 255)',
        'border-right-style': 'solid',
        'border-right-width': '4px',
        'border-top-color': 'rgb(0, 0, 255)',
        'border-top-style': 'solid',
        'border-top-width': '4px',
        'width': '276px',
        'perspective-origin': '142px 24px',
        'transform-origin': '142px 24px'
      }
    },
    'windowHeight': serializer.windowHeight,
    'windowWidth': serializer.windowWidth,
    'frameIndex': serializer.iframeQualifiedName(win)
  };
  var html = unescapeHTML(outputHTMLString([message]), 1);
  assert.equal(
      html,
      '<div style="border-bottom-color: rgb(0, 0, 255); border-bottom-style: ' +
      'solid; border-bottom-width: 4px; border-left-color: rgb(0, 0, 255); ' +
      'border-left-style: solid; border-left-width: 4px; border-right-color: ' +
      'rgb(0, 0, 255); border-right-style: solid; border-right-width: 4px; ' +
      'border-top-color: rgb(0, 0, 255); border-top-style: solid; ' +
      'border-top-width: 4px; width: 276px; perspective-origin: 142px 24px; ' +
      'transform-origin: 142px 24px;" id="snap-it0" >hello world</div>');
});

QUnit.test('processTree: head tag', function(assert) {
  var serializer = new HTMLSerializer();
  var head = document.createElement('head');
  serializer.processTree(head);
  assert.equal(serializer.fontPlaceHolderIndex, 4);
  assert.equal(serializer.pseudoElementPlaceHolderIndex, 5);
});

// https://github.com/catapult-project/catapult/issues/4318
QUnit.skip('minimizeStyles: root html tag', function(assert) {
  var message = {
    'html': [
        '<html id="myId" ',
        'style="animation-delay: 0s; width: 5px;" ',
        '></html>'
    ],
    'frameHoles': null,
    'idToStyleIndex': {},
    'idToStyleMap': {
      'myId': {
        'animation-delay': '0s',
        'width': '5px'
      }
    },
    'windowHeight': 5,
    'windowWidth': 5,
    'rootId': 'myId',
    'rootStyleIndex': 1,
    'frameIndex': '0'
  };
  minimizeStyles(message);
  assert.equal(message.html[1], '');
});

QUnit.test('processAttributes: escaping characters', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.setAttribute('name', '<">');
  fixture.appendChild(div);
  serializer.processAttributes(div, 'myId');
  assert.equal(serializer.html[2], 'name="&lt;&quot;&gt;" ');
});

// https://github.com/catapult-project/catapult/issues/4233
QUnit.skip('window size comment', function(assert) {
  var serializer = new HTMLSerializer();
  serializer.processDocument(document);
  assert.equal(
      serializer.html[1],
      `<!-- Original window height: ${window.innerHeight}. -->\n`);
  assert.equal(
      serializer.html[2],
      `<!-- Original window width: ${window.innerWidth}. -->\n`);
});

// https://github.com/catapult-project/catapult/issues/4233
QUnit.skip('processDocument: doctype tag', function(assert) {
  var serializer = new HTMLSerializer();
  serializer.processDocument(document);
  assert.equal(serializer.html[0], '<!DOCTYPE html>\n');
});

QUnit.test('processDocument: no doctype tag', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var iframe = document.createElement('iframe');
  fixture.appendChild(iframe);
  serializer.processDocument(iframe.contentDocument);
  assert.notEqual(serializer.html[0], '<!DOCTYPE html>\n');
});

QUnit.test('processDocument: no empty styles', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var iframe = document.createElement('iframe');
  fixture.appendChild(iframe);
  serializer.processDocument(iframe.contentDocument);
  for (var i = 0; i < serializer.html.length; i++)
    assert.ok(serializer.html[i].search(/<style>\s*<\/style>/) == -1);
});

QUnit.test('processDocument: no empty styles for css fonts', function(assert) {
  var serializer = new HTMLSerializer();
  var fixture = document.getElementById('qunit-fixture');
  var iframe = document.createElement('iframe');
  serializer.html = ['']
  serializer.fontPlaceHolderIndex = 1;
  serializer.fillFontHoles(iframe.contentDocument, () => {});
  for (var i = 0; i < serializer.html.length; i++)
    assert.ok(serializer.html[i].search(/<style>\s*<\/style>/) == -1);
});

QUnit.test('minimizeStyles: no empty styles', function(assert) {
  var message = {
    'html': [
        '<div></div>'
    ],
    'pseudoElementTestingStyleIndex': 1,
    'pseudoElementPlaceHolderIndex': 2,
    'frameHoles': null,
    'idToStyleIndex': {},
    'idToStyleMap': {},
    'windowHeight': 5,
    'windowWidth': 5,
// https://github.com/catapult-project/catapult/issues/4233
    'frameIndex': '0'
  };
  minimizeStyles(message);
  for (var i = 0; i < message.html.length; i++)
    assert.ok(message.html[i].search(/<style>\s*<\/style>/) == -1);
});

QUnit.test('escapedQuote', function(assert) {
  assert.equal(escapedQuote(0), '"');
  assert.equal(escapedQuote(1), '&quot;');
  assert.equal(escapedQuote(3), '&amp;amp;quot;');
});

QUnit.test('buildStyleAttribute', function(assert) {
  var styleMap = {
    'color': 'blue',
    'background-color': 'red'
  }
  assert.equal(
      buildStyleAttribute(styleMap),
      'color: blue; background-color: red;');
});

QUnit.test('updateMinimizedStyleMap: no update', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.setAttribute('id', 'myId');
  div.setAttribute('style', 'width: 5px;');
  fixture.appendChild(div);
  var originalStyleMap = {
    'animation-delay': '0s',
    'width': '5px'
  };
  var requiredStyleMap = {
    'width': '5px'
  };
  var updated = updateMinimizedStyleMap(
      document,
      div,
      originalStyleMap,
      requiredStyleMap,
      null);
  assert.notOk(updated);
  assert.equal(Object.keys(requiredStyleMap).length, 1);
  assert.equal(requiredStyleMap.width, '5px');
});

QUnit.test('updateMinimizedStyleMap: update', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.setAttribute('id', 'myId');
  fixture.appendChild(div);
  var originalStyleMap = {
    'animation-delay': '0s',
    'width': '5px'
  };
  var requiredStyleMap = {};
  var updated = updateMinimizedStyleMap(
      document,
      div,
      originalStyleMap,
      requiredStyleMap,
      null);
  assert.ok(updated);
  assert.equal(Object.keys(requiredStyleMap).length, 1);
  assert.equal(requiredStyleMap.width, '5px');
});

QUnit.test('minimizePseudoElementStyle', function(assert) {
  var fixture = document.getElementById('qunit-fixture');
  var div = document.createElement('div');
  div.setAttribute('id', 'divId');
  var pseudoElementStyle = document.createElement('style');
  pseudoElementStyle.innerHTML =
      '#divId::before{animation-delay: 0s; content:"hello"}';
  var testingStyle = document.createElement('style');
  testingStyle.setAttribute('id', 'styleId');
  fixture.appendChild(div);
  fixture.appendChild(pseudoElementStyle);
  fixture.appendChild(testingStyle);
  var message = {
    'pseudoElementSelectorToCSSMap': {
      '#divId::before': {
        'animation-delay': '0s',
        'content': 'hello'
      }
    },
    'pseudoElementTestingStyleId': 'styleId',
    'unusedId': 'unusedId',
    'frameIndex': '0'
  }
  var finalPseudoElementCSS = [];
  minimizePseudoElementStyle(
      message,
      document,
      '#divId::before',
      finalPseudoElementCSS);
  assert.equal(
      finalPseudoElementCSS.join(' '),
      '#divId::before{ content: hello; }');
});

QUnit.test('minimizeStyles: pseudo elements', function(assert) {
  var message = {
    'html': [
        '<div id="divId"',
        'style="animation-delay: 0s; width: 5px;" ',
        '></div>',
        '<style>#divId::before{animation-delay: 0s; content:"hello"}</style>',
        '<style id="styleId"></style>'
    ],
    'pseudoElementSelectorToCSSMap': {
      '#divId::before': {
        'animation-delay': '0s',
        'content': 'hello'
      }
    },
    'pseudoElementPlaceHolderIndex': 3,
    'pseudoElementTestingStyleIndex': 4,
    'pseudoElementTestingStyleId': 'styleId',
    'unusedId': 'unusedId',
    'frameHoles': null,
    'idToStyleIndex': {"divId": 1},
    'idToStyleMap': {
      'divId': {
        'animation-delay': '0s',
        'width': '5px'
      }
    },
    'windowHeight': 5,
    'windowWidth': 5,
    'frameIndex': '0'
  };
  minimizeStyles(message);
  assert.equal(
      message.html[3],
      '<style>#divId::before{ content: hello; }</style>');
});

QUnit.test('getExternalImageUrl: no local image path', function(assert) {
  var serializer = new HTMLSerializer();
  var urlWithSuffix = 'http://foo.com/img.png';
  assert.equal(serializer.getExternalImageUrl('id', urlWithSuffix),
      urlWithSuffix);
  assert.equal(serializer.getExternalImageUrl('id', ''), '');
  var urlNoSuffix = 'http://foo.com/img';
  assert.equal(serializer.getExternalImageUrl('id', urlNoSuffix), urlNoSuffix);
});

QUnit.test('getExternalImageUrl: local image path', function(assert) {
  var serializer = new HTMLSerializer();
  serializer.setLocalImagePath('local/');
  var urlWithSuffix = 'http://foo.com/img.png';
  assert.equal(serializer.getExternalImageUrl('id', urlWithSuffix),
      'local/id.png');
  var urlNoSuffix = 'http://foo.com/img';
  assert.equal(serializer.getExternalImageUrl('id', urlNoSuffix), 'local/id');
});

QUnit.test('getExternalImageUrl: fileSuffix', function(assert) {
  var serializer = new HTMLSerializer();
  assert.equal(serializer.fileSuffix(''), '');
  assert.equal(serializer.fileSuffix('/'), '');
  assert.equal(serializer.fileSuffix('foo'), '');
  assert.equal(serializer.fileSuffix('.'), '');
  assert.equal(serializer.fileSuffix('.png'), 'png');
  assert.equal(serializer.fileSuffix('foo.png'), 'png');
  assert.equal(serializer.fileSuffix('foo..png'), 'png');
  assert.equal(serializer.fileSuffix('/foo.png'), 'png');
  assert.equal(serializer.fileSuffix('a/foo.png'), 'png');
  assert.equal(serializer.fileSuffix('http://foo.com/foo'), '');
  assert.equal(serializer.fileSuffix('http://foo.com/foo.png'), 'png');
  assert.equal(serializer.fileSuffix('http://foo.com/foo..png'), 'png');
  assert.equal(serializer.fileSuffix('http://foo.com/foo.png?'), 'png');
  assert.equal(serializer.fileSuffix('http://foo.com/foo.png?v=w'), 'png');
});

QUnit.test('wrapUrl', function(assert) {
  var serializer = new HTMLSerializer();
  assert.equal(serializer.wrapUrl(''), 'url("")');
  assert.equal(serializer.wrapUrl('http://www.foo.com'),
      'url("http://www.foo.com")');
  var dataUrl = 'data:image/gif;base64,R0lGODlhAQABAIAAAA' +
      'UEBAAAACwAAAAAAQABAAACAkQBADs=';
  assert.equal(serializer.wrapUrl(dataUrl),
      'url("data:image/gif;base64,R0lGODlhAQABAIAAAAUEBAAAACwAAAAAAQABAAACAk' +
      'QBADs=")');
});
