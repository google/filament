/**   For testing asynchronous json downloading, here we have a tiny http server.
 *
 *    For every request (regardless f path) it responds with the json [0,1,2,3,4,5,6,7,8,9]
 *    but writing out each number one at a time.
 *
 *    You can start this server up and visit it in a browser to see the numbers stream in.
 */

"use strict";

require('color');

var express = require('express'),
    cors = require('cors'),
    fs = require('fs'),
    http = require('http'),

    JSON_MIME_TYPE = "application/octet-stream",

    httpServer;

module.exports.start = function(httpPort, grunt){

   function makeExpressApp(grunt) {
   
      var verboseLog = grunt? grunt.verbose.ok : console.log,
          errorLog = grunt? grunt.log.error : console.error;
      
      function echoBackBody(req, res) {
         req.pipe(res);
      }
      
      function echoBackHeadersAsBodyJson(req, res) {
         res.end(JSON.stringify(req.headers));
      }
      
      function echoBackQueryParamsAsBodyJson(req, res) {
         res.end(JSON.stringify(req.query));
      }
      
      function echoBackHeadersAsHeaders(req, res) {
         for( var name in req.headers ) {
            res.set(name, req.headers[name]);
         }
         res.end('{"see":"headers", "for":"content"}');
      }
      
      function replyWithTenSlowNumbers(_req, res) {
         sendJsonOkHeaders(res);
      
         var NUMBER_INTERVAL = 250;
         var MAX_NUMBER = 9;
      
         verboseLog(
               'slow number server: will write numbers 0 ..' +
               String(MAX_NUMBER).blue +
               ' out as a json array at a rate of one per',
               String(NUMBER_INTERVAL).blue + 'ms'
         );
      
         res.write('[\n');
      
         var curNumber = 0;
      
         var inervalId = setInterval(function () {
      
            res.write(String(curNumber));
      
            if (curNumber == MAX_NUMBER) {
      
               res.end(']');
               clearInterval(inervalId);
               verboseLog('slow number server: finished writing out');
            } else {
               res.write(',\n');
               curNumber++;
            }
      
         }, NUMBER_INTERVAL);
      }
      
      function replyWithInvalidJson(req, res) {
         res.end('{{');
      }

      function serve204Json(req, res) {
         res.status(204).send();
      }
      
      function serve404Json(req, res) {
         // our little REST endpoint with errors:
         res.status(404).send(JSON.stringify(
            {
               "found":"false",
               "errorMessage":"was not found"
            }
         ));
      }
      
      function replyWithStaticJson(req, res) {
         sendJsonOkHeaders(res);
      
         if( !req.url ) {
            throw new Error('no url given');
         }
      
         var filename = 'test/json/' + req.params.name + '.json';
      
         verboseLog('will respond with contents of file ' + filename);
      
         require('fs').createReadStream(filename)
            .on('error', function(err){
               errorLog('could not read static file ' + filename +
                  ' ' + err);
            }).pipe(res);
      
      }
      
      function sendJsonOkHeaders(res) {
      
         res.setHeader("Content-Type", JSON_MIME_TYPE);
         res.writeHead(200);
      }
      
      function twoHundredItems(_req, res) {
      
         var TIME_BETWEEN_RECORDS = 40;
         // 80 records but only every other one has a URL:
         var NUMBER_OF_RECORDS = 200;
      
         res.write('{"data": [');
      
         var i = 0;
      
         var inervalId = setInterval(function () {
      
            res.write(JSON.stringify({
               "id": i,
               "url": "http://localhost:4444/item/" + i,
               // let's get some entropy in here for gzip:
               "number1": Math.random(),
               "number2": Math.random(),
               "number3": Math.random(),
               "number4": Math.random()
            }));
      
            if (i == NUMBER_OF_RECORDS) {
      
               res.end(']}');
      
               clearInterval(inervalId);
      
               console.log('db server: finished writing to stream');
            } else {
               res.write(',');
            }
      
            i++;
      
         }, TIME_BETWEEN_RECORDS);
      }
      
      function replyWithTenSlowNumbersGzipped(req, serverResponse){
      
         // request out non-gzipped stream and re-serve gzipped
         require('http').get({
            host: 'localhost',
            path: '/twoHundredItems',
            port: httpPort })
            .on('response', function(clientResponse){
      
               var zlib = require('zlib');
      
               //res.writeHead(200, { 'content-encoding': 'gzip' });
      
               serverResponse.setHeader("content-type", JSON_MIME_TYPE);
               serverResponse.setHeader("content-encoding", 'gzip');
               serverResponse.writeHead(200);
      
               clientResponse.pipe(zlib.createGzip({
                  flush: zlib.Z_SYNC_FLUSH
               })).pipe(serverResponse);
      
            });
      }
   
   
      var app = express();
   
      app.use(cors({
         origin: function (origin, callback) {
            callback(null, true); // whitelist all domains
         },
         credentials: true // accept cookies from cross-domain requests
      })); // enable cross-domain for all resources
      app.use(app.router);
   
      app.get('/echoBackBody', function (req, res) {
         res.end("POST/PUT/PATCH here, don't GET")
      });
      app.post('/echoBackBody', echoBackBody);
      app.put('/echoBackBody', echoBackBody);
      app.patch('/echoBackBody', echoBackBody);
      app.get('/echoBackHeadersAsBodyJson', echoBackHeadersAsBodyJson);
      app.post('/echoBackHeadersAsBodyJson', echoBackHeadersAsBodyJson);
      app.put('/echoBackHeadersAsBodyJson', echoBackHeadersAsBodyJson);
      app.patch('/echoBackHeadersAsBodyJson', echoBackHeadersAsBodyJson);
      app.get('/echoBackQueryParamsAsBodyJson', echoBackQueryParamsAsBodyJson);
      app.get('/echoBackHeadersAsHeaders', echoBackHeadersAsHeaders);
      app.get('/static/json/:name.json', replyWithStaticJson);
      app.get('/tenSlowNumbers', replyWithTenSlowNumbers);
      app.get('/twoHundredItems', twoHundredItems);
      app.get('/gzippedTwoHundredItems', replyWithTenSlowNumbersGzipped);
      app.get('/invalidJson', replyWithInvalidJson);
      app.get('/404json', serve404Json);
      app.get('/204noData', serve204Json);
   
      return app;
   }
      
   var expressApp = makeExpressApp(grunt);

   httpServer = http.createServer(expressApp).listen(httpPort);

   grunt.verbose.ok('streaming source server started on ports'.green, 
      String(httpPort).blue
   );
};

module.exports.stop = function(){
   httpServer.close();
};


