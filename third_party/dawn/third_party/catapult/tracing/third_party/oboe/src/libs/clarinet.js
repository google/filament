/* 
   This is a slightly hacked-up browser only version of clarinet 
   
      *  some features removed to help keep browser Oboe under 
         the 5k micro-library limit
      *  plug directly into event bus
   
   For the original go here:
      https://github.com/dscape/clarinet

   We receive the events:
      STREAM_DATA
      STREAM_END
      
   We emit the events:
      SAX_KEY
      SAX_VALUE_OPEN
      SAX_VALUE_CLOSE      
      FAIL_EVENT      
 */

function clarinet(eventBus) {
  "use strict";
   
  var 
      // shortcut some events on the bus
      emitSaxKey           = eventBus(SAX_KEY).emit,
      emitValueOpen        = eventBus(SAX_VALUE_OPEN).emit,
      emitValueClose       = eventBus(SAX_VALUE_CLOSE).emit,
      emitFail             = eventBus(FAIL_EVENT).emit,
              
      MAX_BUFFER_LENGTH = 64 * 1024
  ,   stringTokenPattern = /[\\"\n]/g
  ,   _n = 0
  
      // states
  ,   BEGIN                = _n++
  ,   VALUE                = _n++ // general stuff
  ,   OPEN_OBJECT          = _n++ // {
  ,   CLOSE_OBJECT         = _n++ // }
  ,   OPEN_ARRAY           = _n++ // [
  ,   CLOSE_ARRAY          = _n++ // ]
  ,   STRING               = _n++ // ""
  ,   OPEN_KEY             = _n++ // , "a"
  ,   CLOSE_KEY            = _n++ // :
  ,   TRUE                 = _n++ // r
  ,   TRUE2                = _n++ // u
  ,   TRUE3                = _n++ // e
  ,   FALSE                = _n++ // a
  ,   FALSE2               = _n++ // l
  ,   FALSE3               = _n++ // s
  ,   FALSE4               = _n++ // e
  ,   NULL                 = _n++ // u
  ,   NULL2                = _n++ // l
  ,   NULL3                = _n++ // l
  ,   NUMBER_DECIMAL_POINT = _n++ // .
  ,   NUMBER_DIGIT         = _n   // [0-9]

      // setup initial parser values
  ,   bufferCheckPosition  = MAX_BUFFER_LENGTH
  ,   latestError                
  ,   c                    
  ,   p                    
  ,   textNode             = undefined
  ,   numberNode           = ""     
  ,   slashed              = false
  ,   closed               = false
  ,   state                = BEGIN
  ,   stack                = []
  ,   unicodeS             = null
  ,   unicodeI             = 0
  ,   depth                = 0
  ,   position             = 0
  ,   column               = 0  //mostly for error reporting
  ,   line                 = 1
  ;

  function checkBufferLength () {
     
    var maxActual = 0;
     
    if (textNode !== undefined && textNode.length > MAX_BUFFER_LENGTH) {
      emitError("Max buffer length exceeded: textNode");
      maxActual = Math.max(maxActual, textNode.length);
    }
    if (numberNode.length > MAX_BUFFER_LENGTH) {
      emitError("Max buffer length exceeded: numberNode");
      maxActual = Math.max(maxActual, numberNode.length);
    }
     
    bufferCheckPosition = (MAX_BUFFER_LENGTH - maxActual)
                               + position;
  }

  eventBus(STREAM_DATA).on(handleData);

   /* At the end of the http content close the clarinet 
    This will provide an error if the total content provided was not 
    valid json, ie if not all arrays, objects and Strings closed properly */
  eventBus(STREAM_END).on(handleStreamEnd);   

  function emitError (errorString) {
     if (textNode !== undefined) {
        emitValueOpen(textNode);
        emitValueClose();
        textNode = undefined;
     }

     latestError = Error(errorString + "\nLn: "+line+
                                       "\nCol: "+column+
                                       "\nChr: "+c);
     
     emitFail(errorReport(undefined, undefined, latestError));
  }

  function handleStreamEnd() {
    if( state == BEGIN ) {
      // Handle the case where the stream closes without ever receiving
      // any input. This isn't an error - response bodies can be blank,
      // particularly for 204 http responses
      
      // Because of how Oboe is currently implemented, we parse a
      // completely empty stream as containing an empty object.
      // This is because Oboe's done event is only fired when the
      // root object of the JSON stream closes.
      
      // This should be decoupled and attached instead to the input stream
      // from the http (or whatever) resource ending.
      // If this decoupling could happen the SAX parser could simply emit
      // zero events on a completely empty input.
      emitValueOpen({});
      emitValueClose();

      closed = true;
      return;
    }
  
    if (state !== VALUE || depth !== 0)
      emitError("Unexpected end");
 
    if (textNode !== undefined) {
      emitValueOpen(textNode);
      emitValueClose();
      textNode = undefined;
    }
     
    closed = true;
  }

  function whitespace(c){
     return c == '\r' || c == '\n' || c == ' ' || c == '\t';
  }
   
  function handleData (chunk) {
         
    // this used to throw the error but inside Oboe we will have already
    // gotten the error when it was emitted. The important thing is to
    // not continue with the parse.
    if (latestError)
      return;
      
    if (closed) {
       return emitError("Cannot write after close");
    }

    var i = 0;
    c = chunk[0]; 

    while (c) {
      p = c;
      c = chunk[i++];
      if(!c) break;

      position ++;
      if (c == "\n") {
        line ++;
        column = 0;
      } else column ++;
      switch (state) {

        case BEGIN:
          if (c === "{") state = OPEN_OBJECT;
          else if (c === "[") state = OPEN_ARRAY;
          else if (!whitespace(c))
            return emitError("Non-whitespace before {[.");
        continue;

        case OPEN_KEY:
        case OPEN_OBJECT:
          if (whitespace(c)) continue;
          if(state === OPEN_KEY) stack.push(CLOSE_KEY);
          else {
            if(c === '}') {
              emitValueOpen({});
              emitValueClose();
              state = stack.pop() || VALUE;
              continue;
            } else  stack.push(CLOSE_OBJECT);
          }
          if(c === '"')
             state = STRING;
          else
             return emitError("Malformed object key should start with \" ");
        continue;

        case CLOSE_KEY:
        case CLOSE_OBJECT:
          if (whitespace(c)) continue;

          if(c===':') {
            if(state === CLOSE_OBJECT) {
              stack.push(CLOSE_OBJECT);

               if (textNode !== undefined) {
                  // was previously (in upstream Clarinet) one event
                  //  - object open came with the text of the first
                  emitValueOpen({});
                  emitSaxKey(textNode);
                  textNode = undefined;
               }
               depth++;
            } else {
               if (textNode !== undefined) {
                  emitSaxKey(textNode);
                  textNode = undefined;
               }
            }
             state  = VALUE;
          } else if (c==='}') {
             if (textNode !== undefined) {
                emitValueOpen(textNode);
                emitValueClose();
                textNode = undefined;
             }
             emitValueClose();
            depth--;
            state = stack.pop() || VALUE;
          } else if(c===',') {
            if(state === CLOSE_OBJECT)
              stack.push(CLOSE_OBJECT);
             if (textNode !== undefined) {
                emitValueOpen(textNode);
                emitValueClose();
                textNode = undefined;
             }
             state  = OPEN_KEY;
          } else 
             return emitError('Bad object');
        continue;

        case OPEN_ARRAY: // after an array there always a value
        case VALUE:
          if (whitespace(c)) continue;
          if(state===OPEN_ARRAY) {
            emitValueOpen([]);
            depth++;             
            state = VALUE;
            if(c === ']') {
              emitValueClose();
              depth--;
              state = stack.pop() || VALUE;
              continue;
            } else {
              stack.push(CLOSE_ARRAY);
            }
          }
               if(c === '"') state = STRING;
          else if(c === '{') state = OPEN_OBJECT;
          else if(c === '[') state = OPEN_ARRAY;
          else if(c === 't') state = TRUE;
          else if(c === 'f') state = FALSE;
          else if(c === 'n') state = NULL;
          else if(c === '-') { // keep and continue
            numberNode += c;
          } else if(c==='0') {
            numberNode += c;
            state = NUMBER_DIGIT;
          } else if('123456789'.indexOf(c) !== -1) {
            numberNode += c;
            state = NUMBER_DIGIT;
          } else               
            return emitError("Bad value");
        continue;

        case CLOSE_ARRAY:
          if(c===',') {
            stack.push(CLOSE_ARRAY);
             if (textNode !== undefined) {
                emitValueOpen(textNode);
                emitValueClose();
                textNode = undefined;
             }
             state  = VALUE;
          } else if (c===']') {
             if (textNode !== undefined) {
                emitValueOpen(textNode);
                emitValueClose();
                textNode = undefined;
             }
             emitValueClose();
            depth--;
            state = stack.pop() || VALUE;
          } else if (whitespace(c))
              continue;
          else 
             return emitError('Bad array');
        continue;

        case STRING:
          if (textNode === undefined) {
              textNode = "";
          }

          // thanks thejh, this is an about 50% performance improvement.
          var starti              = i-1;
           
          STRING_BIGLOOP: while (true) {

            // zero means "no unicode active". 1-4 mean "parse some more". end after 4.
            while (unicodeI > 0) {
              unicodeS += c;
              c = chunk.charAt(i++);
              if (unicodeI === 4) {
                // TODO this might be slow? well, probably not used too often anyway
                textNode += String.fromCharCode(parseInt(unicodeS, 16));
                unicodeI = 0;
                starti = i-1;
              } else {
                unicodeI++;
              }
              // we can just break here: no stuff we skipped that still has to be sliced out or so
              if (!c) break STRING_BIGLOOP;
            }
            if (c === '"' && !slashed) {
              state = stack.pop() || VALUE;
              textNode += chunk.substring(starti, i-1);
              break;
            }
            if (c === '\\' && !slashed) {
              slashed = true;
              textNode += chunk.substring(starti, i-1);
               c = chunk.charAt(i++);
              if (!c) break;
            }
            if (slashed) {
              slashed = false;
                   if (c === 'n') { textNode += '\n'; }
              else if (c === 'r') { textNode += '\r'; }
              else if (c === 't') { textNode += '\t'; }
              else if (c === 'f') { textNode += '\f'; }
              else if (c === 'b') { textNode += '\b'; }
              else if (c === 'u') {
                // \uxxxx. meh!
                unicodeI = 1;
                unicodeS = '';
              } else {
                textNode += c;
              }
              c = chunk.charAt(i++);
              starti = i-1;
              if (!c) break;
              else continue;
            }

            stringTokenPattern.lastIndex = i;
            var reResult = stringTokenPattern.exec(chunk);
            if (!reResult) {
              i = chunk.length+1;
              textNode += chunk.substring(starti, i-1);
              break;
            }
            i = reResult.index+1;
            c = chunk.charAt(reResult.index);
            if (!c) {
              textNode += chunk.substring(starti, i-1);
              break;
            }
          }
        continue;

        case TRUE:
          if (!c)  continue; // strange buffers
          if (c==='r') state = TRUE2;
          else
             return emitError( 'Invalid true started with t'+ c);
        continue;

        case TRUE2:
          if (!c)  continue;
          if (c==='u') state = TRUE3;
          else
             return emitError('Invalid true started with tr'+ c);
        continue;

        case TRUE3:
          if (!c) continue;
          if(c==='e') {
            emitValueOpen(true);
            emitValueClose();
            state = stack.pop() || VALUE;
          } else
             return emitError('Invalid true started with tru'+ c);
        continue;

        case FALSE:
          if (!c)  continue;
          if (c==='a') state = FALSE2;
          else
             return emitError('Invalid false started with f'+ c);
        continue;

        case FALSE2:
          if (!c)  continue;
          if (c==='l') state = FALSE3;
          else
             return emitError('Invalid false started with fa'+ c);
        continue;

        case FALSE3:
          if (!c)  continue;
          if (c==='s') state = FALSE4;
          else
             return emitError('Invalid false started with fal'+ c);
        continue;

        case FALSE4:
          if (!c)  continue;
          if (c==='e') {
            emitValueOpen(false);
            emitValueClose();
            state = stack.pop() || VALUE;
          } else
             return emitError('Invalid false started with fals'+ c);
        continue;

        case NULL:
          if (!c)  continue;
          if (c==='u') state = NULL2;
          else
             return emitError('Invalid null started with n'+ c);
        continue;

        case NULL2:
          if (!c)  continue;
          if (c==='l') state = NULL3;
          else
             return emitError('Invalid null started with nu'+ c);
        continue;

        case NULL3:
          if (!c) continue;
          if(c==='l') {
            emitValueOpen(null);
            emitValueClose();
            state = stack.pop() || VALUE;
          } else 
             return emitError('Invalid null started with nul'+ c);
        continue;

        case NUMBER_DECIMAL_POINT:
          if(c==='.') {
            numberNode += c;
            state       = NUMBER_DIGIT;
          } else 
             return emitError('Leading zero not followed by .');
        continue;

        case NUMBER_DIGIT:
          if('0123456789'.indexOf(c) !== -1) numberNode += c;
          else if (c==='.') {
            if(numberNode.indexOf('.')!==-1)
               return emitError('Invalid number has two dots');
            numberNode += c;
          } else if (c==='e' || c==='E') {
            if(numberNode.indexOf('e')!==-1 ||
               numberNode.indexOf('E')!==-1 )
               return emitError('Invalid number has two exponential');
            numberNode += c;
          } else if (c==="+" || c==="-") {
            if(!(p==='e' || p==='E'))
               return emitError('Invalid symbol in number');
            numberNode += c;
          } else {
            if (numberNode) {
              emitValueOpen(parseFloat(numberNode));
              emitValueClose();
              numberNode = "";
            }
            i--; // go back one
            state = stack.pop() || VALUE;
          }
        continue;

        default:
          return emitError("Unknown state: " + state);
      }
    }
    if (position >= bufferCheckPosition)
      checkBufferLength();
  }
}
