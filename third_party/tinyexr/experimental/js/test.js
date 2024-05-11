var fs = require('fs');
var tinyexr = require('./tinyexr.js')

var data = new Uint8Array(fs.readFileSync("../../asakusa.exr"))
console.log(data.length)

var instance = new tinyexr.EXRLoader(data);

console.log(instance.ok())
console.log(instance.width())
console.log(instance.height())

var image = instance.getBytes()
console.log(image[0])
console.log(image[1])
console.log(image[2])
console.log(image[3])
console.log(image[4])
console.log(image[5])
console.log(image[6])
console.log(image[7])
