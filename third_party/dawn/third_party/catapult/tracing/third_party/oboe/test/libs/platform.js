


(function (Platform) { 
   
   Platform.isNode = (typeof window === "undefined");
   
   // from: https://gist.github.com/padolsey/527683   
   // ----------------------------------------------------------
   // A short snippet for detecting versions of IE in JavaScript
   // without resorting to user-agent sniffing
   // ----------------------------------------------------------
   // If you're not in IE (or IE version is less than 5) then:
   //     ie === undefined
   // If you're in IE (>=5) then you can determine which version:
   //     ie === 7; // IE7
   // Thus, to detect IE:
   //     if (ie) {}
   // And to detect the version:
   //     ie === 6 // IE6
   //     ie > 7 // IE8, IE9 ...
   //     ie < 9 // Anything less than IE9
   // ----------------------------------------------------------
    
   // UPDATE: Now using Live NodeList idea from @jdalton   
   Platform.isInternetExplorer = !Platform.isNode && (function(){
    
       var undef,
           v = 3,
           div = document.createElement('div'),
           all = div.getElementsByTagName('i');
       
       while (
           div.innerHTML = '<!--[if gt IE ' + (++v) + ']><i></i><![endif]-->',
           all[0]
       );
       
       return v > 4 ? v : undef;
       
   }());


   Platform.isPhantom =   (typeof window !== "undefined") && 
                  (window.navigator.userAgent.indexOf('PhantomJS') != -1)
                  
   console.log('platform detection:', Platform);

})(typeof exports === "undefined" ? window.Platform = {} : exports);                   