var assert = require('assert'),
    path = require('path'),
    HTML = require('../../lib/common/html'),
    JsDomParser = require('../../index').JsDomParser,
    Serializer = require('../../index').TreeSerializer,
    TestUtils = require('../test_utils');

exports['State guard'] = function () {
    var docHtml = '<script>Yoyo</script>',
        getParsingUnit = function () {
            var parsing = JsDomParser.parseDocument(docHtml);

            parsing.done(function noop() {
                //NOTE: do nothing =)
            });

            return parsing;
        };

    assert.throws(function () {
        var parsing = getParsingUnit();

        parsing.suspend();
        parsing.suspend();
    });

    assert.throws(function () {
        var parsing = getParsingUnit();

        parsing.resume();
    });
};

exports['Reentrancy'] = function (done) {
    var serializer = new Serializer(),
        asyncAssertionCount = 0,
        docHtml1 = '<!DOCTYPE html><html><head><script>Yoyo</script></head><body></body></html>',
        docHtml2 = '<!DOCTYPE html><html><head></head><body>Beep boop</body></html>',
        fragments = [
            '<div>Hey ya!</div>',
            '<p><a href="#"></a></p>',
            '<template><td>yo</td></template>',
            '<select><template><option></option></template></select>'
        ];

    var parsing1 = JsDomParser.parseDocument(docHtml1)

        .done(function (document1) {
            var actual = serializer.serialize(document1);

            assert.strictEqual(asyncAssertionCount, 5);
            assert.ok(actual === docHtml1, TestUtils.getStringDiffMsg(actual, docHtml1));
            done();
        })

        .handleScripts(function () {
            parsing1.suspend();

            setTimeout(function () {
                fragments.forEach(function (fragment) {
                    var actual = serializer.serialize(JsDomParser.parseInnerHtml(fragment));
                    assert.ok(actual === fragment, TestUtils.getStringDiffMsg(actual, fragment));
                    asyncAssertionCount++;
                });

                var parsing2 = JsDomParser.parseDocument(docHtml2);

                parsing2.done(function (document2) {
                    var actual = serializer.serialize(document2);

                    assert.ok(actual === docHtml2, TestUtils.getStringDiffMsg(actual, docHtml2));
                    asyncAssertionCount++;
                    parsing1.resume();
                });
            });
        });
};


TestUtils.generateTestsForEachTreeAdapter(module.exports, function (_test, treeAdapter) {
    function getFullTestName(test) {
        return ['JsDomParser - ', test.idx, '.', test.setName, ' - ', test.input].join('');
    }

    function parseDocument(input, callback) {
        var parsing = JsDomParser.parseDocument(input, treeAdapter)

            .done(callback)

            .handleScripts(function (document, scriptElement) {
                parsing.suspend();

                //NOTE: test parser suspension in different modes (sync and async).
                //If we have script then execute it and resume parser asynchronously.
                //Otherwise - resume synchronously.
                var scriptTextNode = treeAdapter.getChildNodes(scriptElement)[0],
                    script = scriptTextNode && treeAdapter.getTextNodeContent(scriptTextNode);

                //NOTE: don't pollute test runner output by console.log() calls from test scripts
                if (script && script.trim() && script.indexOf('console.log') === -1) {
                    setTimeout(function () {
                        //NOTE: mock document for script evaluation
                        document.write = function (html) {
                            parsing.documentWrite(html);
                        };

                        try {
                            eval(script);
                        } catch (err) {
                            //NOTE: ignore broken scripts from test data
                        }

                        parsing.resume();
                    });
                }

                else
                    parsing.resume();
            });
    }

    //Here we go..
    TestUtils.loadTreeConstructionTestData([
        path.join(__dirname, '../data/tree_construction'),
        path.join(__dirname, '../data/tree_construction_jsdom')
    ], treeAdapter).forEach(function (test) {
        _test[getFullTestName(test)] = function (done) {
            function assertResult(result) {
                var actual = TestUtils.serializeToTestDataFormat(result, treeAdapter),
                    msg = TestUtils.prettyPrintParserAssertionArgs(actual, test.expected);

                assert.strictEqual(actual, test.expected, msg);

                done();
            }

            if (test.fragmentContext) {
                var result = JsDomParser.parseInnerHtml(test.input, test.fragmentContext, treeAdapter);
                assertResult(result);
            }

            else
                parseDocument(test.input, assertResult);
        };
    });
});


