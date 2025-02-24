
/* call this script from the command line with first argument either
    oboe, jsonParse, or clarinet.
    
   This script won't time the events, I'm using `time` on the command line
   to keep things simple.
 */

require('color');

var DB_URL = 'http://localhost:4444/db';  


function aggregateWithOboe() {

   var oboe = require('../dist/oboe-node.js');
   
   oboe(DB_URL).node('{id url}.url', function(url){
           
      oboe(url).node('name', function(name){
                      
         console.log(name);
         this.abort();
         console.log( process.memoryUsage().heapUsed );         
      });      
   });                 
}

function aggregateWithJsonParse() {

   var getJson = require('get-json');

   getJson(DB_URL, function(err, records) {
       
      records.data.forEach( function( record ){
       
         var url = record.url;
         
         getJson(url, function(err, record) {
            console.log(record.name);
            console.log( process.memoryUsage().heapUsed );
         });
      });

   });   

}


function aggregateWithClarinet() {

   var clarinet = require('clarinet');   
   var http = require('http');
   var outerClarinetStream = clarinet.createStream();
   var outerKey;
   
   var outerRequest = http.request(DB_URL, function(res) {
                              
      res.pipe(outerClarinetStream);
   });
   
   outerClarinetStream = clarinet.createStream();
      
   outerRequest.end();
      
   outerClarinetStream.on('openobject', function( keyName ){      
      if( keyName ) {
         outerKey = keyName;      
      }
   });
   
   outerClarinetStream.on('key', function(keyName){
      outerKey = keyName;
   });
   
   outerClarinetStream.on('value', function(value){
      if( outerKey == 'url' ) {
         innerRequest(value)
      }
   });      
   
   
   function innerRequest(url) {
      
      var innerRequest = http.request(url, function(res) {
                                 
         res.pipe(innerClarinetStream);
      });
      
      var innerClarinetStream = clarinet.createStream();
      
      innerRequest.end();            
      
      var innerKey;
      
      innerClarinetStream.on('openobject', function( keyName ){      
         if( keyName ) {
            innerKey = keyName;      
         }
      });
      
      innerClarinetStream.on('key', function(keyName){
         innerKey = keyName;
      });
      
      innerClarinetStream.on('value', function(value){
         if( innerKey == 'name' ) {
            console.log( value )
            console.log( process.memoryUsage().heapUsed );            
         }
      });            
   }
}

var strategies = {
   oboe:       aggregateWithOboe,
   jsonParse:  aggregateWithJsonParse,
   clarinet:   aggregateWithClarinet
}

var strategyName = process.argv[2];

// use any of the above three strategies depending on a command line argument:
console.log('benchmarking strategy', strategyName);

strategies[strategyName]();

