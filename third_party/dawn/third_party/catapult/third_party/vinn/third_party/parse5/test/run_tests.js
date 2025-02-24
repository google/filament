var fs = require('fs'),
    path = require('path'),
    Mocha = require('mocha');

var fixturesDir = path.join(__dirname, './fixtures'),
    mocha = new Mocha()
        .ui('exports')
        .reporter('progress');

fs.readdirSync(fixturesDir).forEach(function (file) {
    mocha.addFile(path.join(fixturesDir, file));
});

mocha.run(function (failed) {
    process.on('exit', function () {
        process.exit(failed);
    });
});