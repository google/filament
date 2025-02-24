var Benchmark = require('benchmark'),
    upstream = require('parse5'),
    workingCopy = require('../index'),
    path = require('path'),
    fs = require('fs');

var testHtml = fs.readFileSync(path.join(__dirname, './spec.html')).toString(),
    wcParser = new workingCopy.Parser(),
    usParser = new upstream.Parser();

new Benchmark.Suite()
    .add('Working copy', function () {
        wcParser.parse(testHtml);
    })

    .add('Upstream', function () {
        usParser.parse(testHtml);
    })

    .on('start', function () {
        console.log('Benchmarking...')
    })

    .on('cycle', function (event) {
        console.log(event.target.toString());
    })

    .on('complete', function () {
        console.log('Fastest is ' + this.filter('fastest').pluck('name'));

        console.log('Working copy is x' + (this[0].hz / this[1].hz).toFixed(2) + ' times faster vs upstream.');
    })

    .run();
