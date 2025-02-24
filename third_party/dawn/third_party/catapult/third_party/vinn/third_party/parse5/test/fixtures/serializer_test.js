var assert = require('assert'),
    path = require('path'),
    parse5 = require('../../index'),
    Parser = parse5.Parser,
    Serializer = parse5.Serializer,
    TestUtils = require('../test_utils');


exports['Backward compatibility - parse5.TreeSerializer'] = function () {
    assert.strictEqual(Serializer, parse5.TreeSerializer);
};

exports['Regression - Get text node\'s parent tagName only if it\'s an Element node (GH-38)'] = {
    test: function () {
        var parser = new Parser(),
            serializer = new Serializer(),
            document = parser.parse('<template>yo<div></div>42</template>'),
            originalGetTagName = this.originalGetTagName = parse5.TreeAdapters.default.getTagName;

        parse5.TreeAdapters.default.getTagName = function (element) {
            assert.ok(element.tagName);

            return originalGetTagName(element);
        };

        serializer.serialize(document);
    },

    after: function () {
        parse5.TreeAdapters.default.getTagName = this.originalGetTagName;
    }
};

exports['Regression - SYSTEM-only doctype serialization'] = function () {
    var html = '<!DOCTYPE html SYSTEM "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">' +
               '<html><head></head><body></body></html>',
        parser = new Parser(),
        serializer = new Serializer(),
        document = parser.parse(html),
        serializedResult = serializer.serialize(document);

    assert.strictEqual(serializedResult, html);
};

exports['Regression - Escaping of doctypes with quotes in them'] = function () {
    var htmlStrs = [
            '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" ' +
            '"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">' +
            '<html><head></head><body></body></html>',

            '<!DOCTYPE html PUBLIC \'-//W3C//"DTD" XHTML 1.0 Transitional//EN\' ' +
            '"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">' +
            '<html><head></head><body></body></html>',

            '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" ' +
            '\'http://www.w3.org/TR/xhtml1/DTD/"xhtml1-transitional.dtd"\'>' +
            '<html><head></head><body></body></html>'
        ],
        parser = new Parser(),
        serializer = new Serializer();

    htmlStrs.forEach(function (html) {
        var document = parser.parse(html),
            serializedResult = serializer.serialize(document);

        assert.strictEqual(serializedResult, html);
    });
};

exports['Regression - new line in <pre> tag'] = function () {
    var htmlStrs = [
            {
                src: '<!DOCTYPE html><html><head></head><body><pre>\ntest</pre></body></html>',
                expected: '<!DOCTYPE html><html><head></head><body><pre>test</pre></body></html>'
            },

            {
                src: '<!DOCTYPE html><html><head></head><body><pre>\n\ntest</pre></body></html>',
                expected: '<!DOCTYPE html><html><head></head><body><pre>\n\ntest</pre></body></html>'
            }
        ],
        parser = new Parser(),
        serializer = new Serializer();

    htmlStrs.forEach(function (htmlStr) {
        var document = parser.parse(htmlStr.src),
            serializedResult = serializer.serialize(document);

        assert.strictEqual(serializedResult, htmlStr.expected);
    });
};

exports['Options - encodeHtmlEntities'] = function () {
    var testHtmlCases = [
            {
                options: {encodeHtmlEntities: true},
                src: '<!DOCTYPE html><html><head></head><body>&</body></html>',
                expected: '<!DOCTYPE html><html><head></head><body>&amp;</body></html>'
            },

            {
                options: {encodeHtmlEntities: false},
                src: '<!DOCTYPE html><html><head></head><body>&</body></html>',
                expected: '<!DOCTYPE html><html><head></head><body>&</body></html>'
            },
            {
                options: {encodeHtmlEntities: true},
                src: '<!DOCTYPE html><html><head></head><body><a href="http://example.com?hello=1&world=2"></a></body></html>',
                expected: '<!DOCTYPE html><html><head></head><body><a href="http://example.com?hello=1&amp;world=2"></a></body></html>'
            },

            {
                options: {encodeHtmlEntities: false},
                src: '<!DOCTYPE html><html><head></head><body><a href="http://example.com?hello=1&world=2"></a></body></html>',
                expected: '<!DOCTYPE html><html><head></head><body><a href="http://example.com?hello=1&world=2"></a></body></html>'
            }
        ],
        parser = new Parser();

    testHtmlCases.forEach(function (testCase) {
        var serializer = new Serializer(null, testCase.options);
        var document = parser.parse(testCase.src),
            serializedResult = serializer.serialize(document);

        assert.strictEqual(serializedResult, testCase.expected);
    });
};


TestUtils.generateTestsForEachTreeAdapter(module.exports, function (_test, treeAdapter) {
    function getFullTestName(test) {
        return ['Serializer - ', test.idx, '.', test.name].join('');
    }

    var testDataDir = path.join(__dirname, '../data/serialization');

    //Here we go..
    TestUtils.loadSerializationTestData(testDataDir).forEach(function (test) {
        _test[getFullTestName(test)] = function () {
            var parser = new Parser(treeAdapter),
                serializer = new Serializer(treeAdapter),
                document = parser.parse(test.src),
                serializedResult = TestUtils.removeNewLines(serializer.serialize(document)),
                expected = TestUtils.removeNewLines(test.expected);

            //NOTE: use ok assertion, so output will not be polluted by the whole content of the strings
            assert.ok(serializedResult === expected, TestUtils.getStringDiffMsg(serializedResult, expected));
        };
    });

});
