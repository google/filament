var fs = require('fs'),
    path = require('path');

var fileData = fs.readFileSync(path.join(__dirname, 'entities.json')).toString(),
    entitiesData = JSON.parse(fileData);

var trie = Object.keys(entitiesData).reduce(function (trie, entity) {
    var resultCodepoints = entitiesData[entity].codepoints;

    entity = entity.replace(/^&/, '');

    var last = entity.length - 1,
        leaf = trie;

    entity
        .split('')
        .map(function (ch) {
            return ch.charCodeAt(0);
        })
        .forEach(function (key, idx) {
            if (!leaf[key])
                leaf[key] = {};

            if (idx === last)
                leaf[key].c = resultCodepoints;

            else {
                if (!leaf[key].l)
                    leaf[key].l = {};

                leaf = leaf[key].l;
            }
        });

    return trie;
}, {});


var outData =
    '\'use strict\';\n\n' +
    '//NOTE: this file contains auto generated trie structure that is used for named entity references consumption\n' +
    '//(see: http://www.whatwg.org/specs/web-apps/current-work/multipage/tokenization.html#tokenizing-character-references and\n' +
    '//http://www.whatwg.org/specs/web-apps/current-work/multipage/named-character-references.html#named-character-references)\n' +
    'module.exports = ' + JSON.stringify(trie).replace(/"/g, '') + ';\n';

fs.writeFileSync(path.join(__dirname, '../lib/tokenization/named_entity_trie.js'), outData);
