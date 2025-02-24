'use strict';

global.parse5 = {};

parse5.Parser = require('./lib/tree_construction/parser');
parse5.SimpleApiParser = require('./lib/simple_api/simple_api_parser');
parse5.TreeSerializer =
parse5.Serializer = require('./lib/serialization/serializer');
parse5.JsDomParser = require('./lib/jsdom/jsdom_parser');

parse5.TreeAdapters = {
    default: require('./lib/tree_adapters/default'),
    htmlparser2: require('./lib/tree_adapters/htmlparser2')
};
