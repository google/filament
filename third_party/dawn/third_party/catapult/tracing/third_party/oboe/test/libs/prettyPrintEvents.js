function prettyPrintEvent(event){

   switch(event) {
      case     HTTP_START:  return 'start';
      case     STREAM_DATA: return 'data';
      case     STREAM_END:  return 'end';
      case     SAX_KEY          : return 'sax_key';
      case     SAX_VALUE_OPEN   : return 'sax_open';
      case     SAX_VALUE_CLOSE  : return 'sax_close';
      case     FAIL_EVENT       : return 'fail';
      default: return 'unknown(' + event + ')'
   }
}
