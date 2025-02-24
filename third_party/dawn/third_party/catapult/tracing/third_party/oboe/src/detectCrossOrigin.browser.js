/**
 * Detect if a given URL is cross-origin in the scope of the
 * current page.
 * 
 * Browser only (since cross-origin has no meaning in Node.js)
 *
 * @param {Object} pageLocation - as in window.location
 * @param {Object} ajaxHost - an object like window.location describing the 
 *    origin of the url that we want to ajax in
 */
function isCrossOrigin(pageLocation, ajaxHost) {

   /*
    * NB: defaultPort only knows http and https.
    * Returns undefined otherwise.
    */
   function defaultPort(protocol) {
      return {'http:':80, 'https:':443}[protocol];
   }
   
   function portOf(location) {
      // pageLocation should always have a protocol. ajaxHost if no port or
      // protocol is specified, should use the port of the containing page
      
      return location.port || defaultPort(location.protocol||pageLocation.protocol);
   }

   // if ajaxHost doesn't give a domain, port is the same as pageLocation
   // it can't give a protocol but not a domain
   // it can't give a port but not a domain
   
   return !!(  (ajaxHost.protocol  && (ajaxHost.protocol  != pageLocation.protocol)) ||
               (ajaxHost.host      && (ajaxHost.host      != pageLocation.host))     ||
               (ajaxHost.host      && (portOf(ajaxHost) != portOf(pageLocation)))
          );
}

/* turn any url into an object like window.location */
function parseUrlOrigin(url) {
   // url could be domain-relative
   // url could give a domain

   // cross origin means:
   //    same domain
   //    same port
   //    some protocol
   // so, same everything up to the first (single) slash 
   // if such is given
   //
   // can ignore everything after that   
   
   var URL_HOST_PATTERN = /(\w+:)?(?:\/\/)([\w.-]+)?(?::(\d+))?\/?/,

         // if no match, use an empty array so that
         // subexpressions 1,2,3 are all undefined
         // and will ultimately return all empty
         // strings as the parse result:
       urlHostMatch = URL_HOST_PATTERN.exec(url) || [];
   
   return {
      protocol:   urlHostMatch[1] || '',
      host:       urlHostMatch[2] || '',
      port:       urlHostMatch[3] || ''
   };
}
