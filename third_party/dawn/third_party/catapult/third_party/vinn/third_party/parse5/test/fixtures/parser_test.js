var assert = require('assert'),
    path = require('path'),
    HTML = require('../../lib/common/html'),
    parse5 = require('../../index'),
    Parser = parse5.Parser,
    Serializer = parse5.Serializer,
    TestUtils = require('../test_utils');


TestUtils.generateTestsForEachTreeAdapter(module.exports, function (_test, treeAdapter) {
    function getFullTestName(test) {
        return ['Parser(', test.dirName, ') - ', test.idx, '.', test.setName, ' - ', test.input].join('');
    }

    function getLocationFullTestName(test) {
        return ['Parser(Location info) - ', test.name].join('');
    }

    function walkTree(document, handler) {
        for (var stack = treeAdapter.getChildNodes(document).slice(); stack.length;) {
            var node = stack.shift(),
                children = treeAdapter.getChildNodes(node);

            handler(node);

            if (children && children.length)
                stack = children.concat(stack);
        }
    }

    //Here we go..
    TestUtils.loadTreeConstructionTestData([
        path.join(__dirname, '../data/tree_construction'),
        path.join(__dirname, '../data/tree_construction_regression'),
        path.join(__dirname, '../data/tree_construction_options')
    ], treeAdapter).forEach(function (test) {
        _test[getFullTestName(test)] = function () {
            var parser = new Parser(treeAdapter, {
                    decodeHtmlEntities: !test.disableEntitiesDecoding
                }),
                result = test.fragmentContext ?
                         parser.parseFragment(test.input, test.fragmentContext) :
                         parser.parse(test.input),
                actual = TestUtils.serializeToTestDataFormat(result, treeAdapter),
                msg = TestUtils.prettyPrintParserAssertionArgs(actual, test.expected);

            assert.strictEqual(actual, test.expected, msg);
        };
    });


    //Location info tests
    TestUtils.loadSerializationTestData(path.join(__dirname, '../data/serialization')).forEach(function (test) {
        //NOTE: the idea of this test is the following: we parse document with the location info.
        //Then for each node in the tree we run serializer and compare results with the substring
        //obtained via location info from the expected serialization results.
        _test[getLocationFullTestName(test)] = function () {
            var parser = new Parser(treeAdapter, {
                    locationInfo: true,
                    decodeHtmlEntities: false
                }),
                serializer = new Serializer(treeAdapter, {
                    encodeHtmlEntities: false
                }),
                html = test.expected,
                document = parser.parse(html);

            walkTree(document, function (node) {
                if (node.__location !== null) {
                    var fragment = treeAdapter.createDocumentFragment();

                    treeAdapter.appendChild(fragment, node);

                    var expected = serializer.serialize(fragment),
                        actual = html.substring(node.__location.start, node.__location.end);

                    expected = TestUtils.removeNewLines(expected);
                    actual = TestUtils.removeNewLines(actual);

                    //NOTE: use ok assertion, so output will not be polluted by the whole content of the strings
                    assert.ok(actual === expected, TestUtils.getStringDiffMsg(actual, expected));

                    if (node.__location.startTag) {
                        //NOTE: Based on the idea that the serialized fragment starts with the startTag
                        var length = node.__location.startTag.end - node.__location.startTag.start,
                            expectedStartTag = serializer.serialize(fragment).substring(0, length),
                            actualStartTag = html.substring(node.__location.startTag.start, node.__location.startTag.end);

                        expectedStartTag = TestUtils.removeNewLines(expectedStartTag);
                        actualStartTag = TestUtils.removeNewLines(actualStartTag);

                        assert.ok(expectedStartTag === actualStartTag, TestUtils.getStringDiffMsg(actualStartTag, expectedStartTag));
                    }

                    if (node.__location.endTag) {
                        //NOTE: Based on the idea that the serialized fragment ends with the endTag
                        var length = node.__location.endTag.end - node.__location.endTag.start,
                            expectedEndTag = serializer.serialize(fragment).slice(-length),
                            actualEndTag = html.substring(node.__location.endTag.start, node.__location.endTag.end);

                        expectedEndTag = TestUtils.removeNewLines(expectedEndTag);
                        actualEndTag = TestUtils.removeNewLines(actualEndTag);

                        assert.ok(expectedEndTag === actualEndTag, TestUtils.getStringDiffMsg(actualEndTag, expectedEndTag));
                    }
                }
            });
        };
    });

    exports['Regression - location info for the implicitly generated <body>, <html> and <head> (GH-44)'] = function () {
        var html = '</head><div class="test"></div></body></html>',
            parser = new Parser(treeAdapter, {
                locationInfo: true,
                decodeHtmlEntities: false
            }),
            document = parser.parse(html);

        //NOTE: location info for all implicitly generated elements should be null
        walkTree(document, function (node) {
            if (treeAdapter.getTagName(node) !== HTML.TAG_NAMES.DIV)
                assert.strictEqual(node.__location, null);
        });
    };
});


exports['Regression - HTML5 Legacy Doctype Misparsed with htmlparser2 tree adapter (GH-45)'] = function () {
    var html = '<!DOCTYPE html SYSTEM "about:legacy-compat"><html><head></head><body>Hi there!</body></html>',
        parser = new Parser(parse5.TreeAdapters.htmlparser2),
        document = parser.parse(html);

    assert.strictEqual(document.childNodes[0].data, '!DOCTYPE html SYSTEM "about:legacy-compat"');
};


