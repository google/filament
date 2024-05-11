// (Note: invisible BOM on this line!)
/** 

  Markdeep.js
  Version 1.13

  Copyright 2015-2020, Morgan McGuire, https://casual-effects.com
  All rights reserved.

  -------------------------------------------------------------

  See https://casual-effects.com/markdeep for documentation on how to
  use this script make your plain text documents render beautifully
  in web browsers.

  Markdeep was created by Morgan McGuire. It extends the work of:

   - John Gruber's original Markdown
   - Ben Hollis' Maruku Markdown dialect
   - Michel Fortin's Markdown Extras dialect
   - Ivan Sagalaev's highlight.js
   - Contributors to the above open source projects

  -------------------------------------------------------------
 
  You may use, extend, and redistribute this code under the terms of
  the BSD license at https://opensource.org/licenses/BSD-2-Clause.

  Contains highlight.js (https://github.com/isagalaev/highlight.js) by Ivan
  Sagalaev, which is used for code highlighting. (BSD 3-clause license)

  There is an invisible Byte-Order-Marker at the start of this file to
  ensure that it is processed as UTF-8. Do not remove this character or it
  will break the regular expressions in highlight.js.
*/
/**See https://casual-effects.com/markdeep for @license and documentation.
markdeep.min.js 1.13 (C) 2020 Morgan McGuire 
highlight.min.js 10.5.0 (C) 2020 Ivan Sagalaev https://highlightjs.org */
(function() {
'use strict';

var MARKDEEP_FOOTER = '<div class="markdeepFooter"><i>formatted by <a href="https://casual-effects.com/markdeep" style="color:#999">Markdeep&nbsp;1.13&nbsp;&nbsp;</a></i><div style="display:inline-block;font-size:13px;font-family:\'Times New Roman\',serif;vertical-align:middle;transform:translate(-3px,-1px)rotate(135deg);">&#x2712;</div></div>';

{
// For minification. This is admittedly scary.
var _ = String.prototype;
_.rp = _.replace;
_.ss = _.substring;
if (!_.endsWith) {
    // For IE11
    _.endsWith = function(S, L) {
        if (L === undefined || L > this.length) {
            L = this.length;
        }
        return this.ss(L - S.length, L) === S;
    };
}

// Regular expression version of String.indexOf
_.regexIndexOf = function(regex, startpos) {
    var i = this.ss(startpos || 0).search(regex);
    return (i >= 0) ? (i + (startpos || 0)) : i;
}
}

/** Enable for debugging to view character bounds in diagrams */
var DEBUG_SHOW_GRID = false;

/** Overlay the non-empty characters of the original source in diagrams */
var DEBUG_SHOW_SOURCE = DEBUG_SHOW_GRID;

/** Use to suppress passing through text in diagrams */
var DEBUG_HIDE_PASSTHROUGH = DEBUG_SHOW_SOURCE;

/** In pixels of lines in diagrams */
var STROKE_WIDTH = 2;

/** A box of these denotes a diagram */
var DIAGRAM_MARKER = '*';

// http://stackoverflow.com/questions/1877475/repeat-character-n-times
// ECMAScript 6 has a String.repeat method, but that's not available everywhere
var DIAGRAM_START = Array(5 + 1).join(DIAGRAM_MARKER);

/** attribs are optional */
function entag(tag, content, attribs) {
    return '<' + tag + (attribs ? ' ' + attribs : '') + '>' + content + '</' + tag + '>';
}


function measureFontSize(fontStack) {
    try {
        var canvas = document.createElement('canvas');
        var ctx = canvas.getContext('2d');
        ctx.font = '10pt ' + fontStack;
        return ctx.measureText("M").width;
    } catch (e) {
        // Needed for Firefox include...canvas doesn't work for some reason
        return 10;
    }
}

// IE11 polyfill needed by Highlight.js, from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/assign#Polyfill
if (typeof Object.assign !== 'function') {
    // Must be writable: true, enumerable: false, configurable: true
    Object.defineProperty(Object, "assign", {
        value: function assign(target, varArgs) { // .length of function is 2
            if (target === null || target === undefined) {
                throw new TypeError('Cannot convert undefined or null to object');
            }
            
            var to = Object(target);
            
            for (var index = 1; index < arguments.length; index++) {
                var nextSource = arguments[index];
                
                if (nextSource !== null && nextSource !== undefined) { 
                    for (var nextKey in nextSource) {
                        // Avoid bugs when hasOwnProperty is shadowed
                        if (Object.prototype.hasOwnProperty.call(nextSource, nextKey)) {
                            to[nextKey] = nextSource[nextKey];
                        }
                    }
                }
            }
            return to;
        },
        writable: true,
        configurable: true
    });
}

// Polyfill for IE11 from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/includes
if (!String.prototype.includes) {
    String.prototype.includes = function(search, start) {
        if (search instanceof RegExp) {
            throw TypeError('first argument must not be a RegExp');
        } 
        if (start === undefined) { start = 0; }
        return this.indexOf(search, start) !== -1;
    };
}
if (!Array.prototype.includes) {
    Array.prototype.includes = function(search) {
        return !!~this.indexOf(search);
    }
}
    
   
 
// Lucida Console on Windows has capital V's that look like lower case, so don't use it
var codeFontStack = "Menlo,Consolas,monospace";
var codeFontSize  = Math.round(6.5 * 105.1316178 / measureFontSize(codeFontStack)) + '%';// + 'px';

var BODY_STYLESHEET = entag('style', 'body{max-width:680px;' +
    'margin:auto;' +
    'padding:20px;' +
    'text-align:justify;' +
    'line-height:140%;' +
    '-webkit-font-smoothing:antialiased;-moz-osx-font-smoothing:grayscale;font-smoothing:antialiased;' +
    'color:#222;' +
    'font-family:Palatino,Georgia,"Times New Roman",serif}');

/** You can embed your own stylesheet AFTER the <script> tags in your
    file to override these defaults. */
var STYLESHEET = entag('style',
                       // Force background images (except on the body) to print correctly on Chrome and Safari
                       // and remove text shadows, which Chrome can't print and will turn into
                       // boxes
    '@media print{*{-webkit-print-color-adjust:exact;text-shadow:none !important}}' +

    'body{' +
    'counter-reset: h1 paragraph line item list-item' +
    '}' +

    // Avoid header/footer in print to PDF. See https://productforums.google.com/forum/#!topic/chrome/LBMUDtGqr-0
    '@page{margin:0;size:auto}' +

    '#mdContextMenu{position:absolute;background:#383838;cursor:default;border:1px solid #999;color:#fff;padding:4px 0px;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Oxygen,Ubuntu,"Helvetica Neue",sans-serif;font-size:85%;font-weight:600;border-radius:4px;box-shadow:0px 3px 10px rgba(0,0,0,35%)}' +
    '#mdContextMenu div{padding:0px 20px}' +
    '#mdContextMenu div:hover{background:#1659d1}' +
                       
    '.md code,.md pre{' +
    'font-family:' + codeFontStack + ';' +
    'font-size:' + codeFontSize + ';' +
    'text-align:left;' +
    'line-height:140%' + 
    '}' +

    '.md .mediumToc code,.md longToc code,.md .shortToc code,.md h1 code,.md h2 code,.md h3 code,.md h4 code,.md h5 code,.md h6 code{font-size:unset}' +

    '.md div.title{' +
    'font-size:26px;' +
    'font-weight:800;' +
    'line-height:120%;' +
    'text-align:center' +
    '}' +

    '.md div.afterTitles{height:10px}' +

    '.md div.subtitle{' +
    'text-align:center' +
    '}' +

    '.md iframe.textinsert, .md object.textinsert,.md iframe:not(.markdeep){display:block;margin-top:10px;margin-bottom:10px;width:100%;height:75vh;border:1px solid #000;border-radius:4px;background:#f5f5f4}' +

    '.md .image{display:inline-block}' +

    '.md img{' +
    'max-width:100%;' +
    'page-break-inside:avoid' +
    '}' +

    // Justification tends to handle URLs and code blocks poorly
    // when inside of a bullet, so disable it there
    '.md li{text-align:left;text-indent:0}' +

    // Make code blocks use 4-space tabs.
    // Set up a line number counter. Do NOT use "overflow: scroll" or it will force scrollbars even when unused on Windows.
    // Don't use text-overflow:ellipsis; which on mac just makes the line short even when scrolled
    '.md pre.listing {width:100%;tab-size:4;-moz-tab-size:4;-o-tab-size:4;counter-reset:line;overflow-x:auto;resize:horizontal}' +

    '.md pre.listing .linenumbers span.line:before{width:30px;margin-left:-28px;font-size:80%;text-align:right;counter-increment:line;' +
    'content:counter(line);display:inline-block;padding-right:13px;margin-right:8px;color:#ccc}' +

     // Force captions on line listings down close and then center them
    '.md div.tilde{' +
    'margin:20px 0 -10px;' +
    'text-align:center' + 
    '}' +

    '.md .imagecaption,.md .tablecaption,.md .listingcaption{' +
    'display:inline-block;' +
    'margin:7px 5px 12px;' +
    'text-align:justify;' +
    'font-style:italic' +
    '}' +
                       
    '.md img.pixel{image-rendering:-moz-crisp-edges;image-rendering:pixelated}' +
                       
    '.md blockquote.fancyquote{' + 
    'margin:25px 0 25px;' +
    'text-align:left;' +
    'line-height:160%' +
    '}' +

    '.md blockquote.fancyquote::before{' +
    'content:"\u201C";' +
    'color:#DDD;' +
    'font-family:Times New Roman;' +
    'font-size:45px;' +
    'line-height:0;' +
    'margin-right:6px;' +
    'vertical-align:-0.3em' +
    '}' +

    '.md span.fancyquote{' +
    'font-size:118%;' +
    'color:#777;' +
    'font-style:italic' +
    '}' +

    '.md span.fancyquote::after{' +
    'content:"\u201D";' +
    'font-style:normal;' +
    'color:#DDD;' +
    'font-family:Times New Roman;' +
    'font-size:45px;' +
    'line-height:0;' +
    'margin-left:6px;' +
    'vertical-align:-0.3em' +
    '}' +

    '.md blockquote.fancyquote .author{' +
    'width:100%;' +
    'margin-top:10px;' + 
    'display:inline-block;' +
    'text-align:right' +
    '}' +

    '.md small{font-size:60%}' +
    '.md big{font-size:150%}' +

    '.md div.title,contents,.md .tocHeader,.md h1,.md h2,.md h3,.md h4,.md h5,.md h6,.md .shortTOC,.md .mediumTOC,.nonumberh1,.nonumberh2,.nonumberh3,.nonumberh4,.nonumberh5,.nonumberh6{' +
    'font-family:Verdana,Helvetica,Arial,sans-serif;' +
    'margin:13.4px 0 13.4px;' +
    'padding:15px 0 3px;' +
    'border-top:none;' +
    'clear:both' +
    '}' +
                       
    '.md .tocTop {display:none}' +

    '.md h1,.md h2,.md h3,.md h4,.md h5,.md h6,.md .nonumberh1,.md .nonumberh2,.md .nonumberh3,.md .nonumberh4,.md .nonumberh5,.md .nonumberh6{' +
     'page-break-after:avoid;break-after:avoid' +
    '}'+

    '.md svg.diagram{' +
    'display:block;' +
    'font-family:' + codeFontStack + ';' +
    'font-size:' + codeFontSize + ';' +
    'text-align:center;' +
    'stroke-linecap:round;' +
    'stroke-width:' + STROKE_WIDTH + 'px;'+
    'page-break-inside:avoid;' +
    'stroke:#000;' + 
    'fill:#000' +
    '}' +

    '.md svg.diagram .opendot{' +
    'fill:#fff' +
    '}' +

    '.md svg.diagram .shadeddot{' +
    'fill:#CCC' +
    '}' +

    '.md svg.diagram .dotteddot{' +
    'stroke:#000;stroke-dasharray:4;fill:none' +
    '}' +

    '.md svg.diagram text{' +
    'stroke:none' +
    '}' +

    // printing scale and margins
    '@media print{@page{margin:1in 5mm;transform: scale(150%)}}' +
                       
    // pagebreak hr
    '@media print{.md .pagebreak{page-break-after:always;visibility:hidden}}' +

    // Not restricted to a:link because we want things like svn URLs to have this font, which
    // makes "//" look better.
    '.md a{font-family:Georgia,Palatino,\'Times New Roman\'}' +

    '.md h1,.md .tocHeader,.md .nonumberh1{' +
    'border-bottom:3px solid;' +
    'font-size:20px;' +
    'font-weight:bold;' +
    '}' +

    '.md h1,.md .nonumberh1{' +
    'counter-reset:h2 h3 h4 h5 h6' +
    '}' +

    '.md h2,.md .nonumberh2{' +
    'counter-reset:h3 h4 h5 h6;' +
    'border-bottom:2px solid #999;' +
    'color:#555;' +
    'font-weight:bold;'+
    'font-size:18px;' +
    '}' +

    '.md h3,.md h4,.md h5,.md h6,.md .nonumberh3,.md .nonumberh4,.md .nonumberh5,.md .nonumberh6{' +
    'font-family:Verdana,Helvetica,Arial,sans-serif;' +
    'color:#555;' +
    'font-size:16px;' +
    '}' +

    '.md h3{counter-reset:h4 h5 h6}' +
    '.md h4{counter-reset:h5 h6}' +
    '.md h5{counter-reset:h6}' +

    '.md div.table{' +
    'margin:16px 0 16px 0' +
    '}' +
                       
    '.md table{' +
    'border-collapse:collapse;' +
    'line-height:140%;' +
    'page-break-inside:avoid' +
    '}' +

    '.md table.table{' +
    'margin:auto' +
    '}' +

    '.md table.calendar{' +
    'width:100%;' +
    'margin:auto;' +
    'font-size:11px;' +
    'font-family:Verdana,Helvetica,Arial,sans-serif' +
    '}' +

    '.md table.calendar th{' +
    'font-size:16px' +
    '}' +

    '.md .today{' +
    'background:#ECF8FA' +
    '}' +

    '.md .calendar .parenthesized{' +
    'color:#999;' + 
    'font-style:italic' +
    '}' +

    '.md table.table th{' +
    'color:#FFF;' +
    'background-color:#AAA;' +
    'border:1px solid #888;' +
     // top right bottom left
    'padding:8px 15px 8px 15px' +
    '}' +

    '.md table.table td{' +
     // top right bottom left
    'padding:5px 15px 5px 15px;' +
    'border:1px solid #888' +
    '}' +

    '.md table.table tr:nth-child(even){'+
    'background:#EEE' +
    '}' +

    '.md pre.tilde{' +
    'border-top: 1px solid #CCC;' + 
    'border-bottom: 1px solid #CCC;' + 
    'padding: 5px 0 5px 20px;' +
    'margin:0 0 0 0;' +
    'background:#FCFCFC;' +
    'page-break-inside:avoid' +
    '}' +

    '.md a.target{width:0px;height:0px;visibility:hidden;font-size:0px;display:inline-block}' +
    '.md a:link, .md a:visited{color:#38A;text-decoration:none}' +
    '.md a:link:hover{text-decoration:underline}' +

    '.md dt{' +
    'font-weight:700' +
    '}' +

    // Remove excess space above definitions due to paragraph breaks, and add some at the bottom
    '.md dl>dd{margin-top:-8px; margin-bottom:8px}' +
                       
     // Extra space around terse definition lists
    '.md dl>table{' +
    'margin:35px 0 30px' + 
    '}' +

    '.md code{' +
    'page-break-inside:avoid;' +
    '} @media print{.md .listing code{white-space:pre-wrap}}' +

    '.md .endnote{' +
    'font-size:13px;' +
    'line-height:15px;' +
    'padding-left:10px;' +
    'text-indent:-10px' +
    '}' +

    '.md .bib{' +
    'padding-left:80px;' +
    'text-indent:-80px;' +
    'text-align:left' +
    '}' +

    '.markdeepFooter{font-size:9px;text-align:right;padding-top:80px;color:#999}' +

    '.md .mediumTOC{float:right;font-size:12px;line-height:15px;border-left:1px solid #CCC;padding-left:15px;margin:15px 0px 15px 25px}' +

    '.md .mediumTOC .level1{font-weight:600}' +

    '.md .longTOC .level1{font-weight:600;display:block;padding-top:12px;margin:0 0 -20px}' +
     
    '.md .shortTOC{text-align:center;font-weight:bold;margin-top:15px;font-size:14px}' +

    '.md .admonition{' +
         'position:relative;' +
         'margin:1em 0;' +
         'padding:.4rem 1rem;' +
         'border-radius:.2rem;' +
         'border-left:2.5rem solid rgba(68,138,255,.4);' +
         'background-color:rgba(68,138,255,.15);' +
     '}' +

     '.md .admonition-title{' +
         'font-weight:bold;' +
         'border-bottom:solid 1px rgba(68,138,255,.4);' +
         'padding-bottom:4px;' +
         'margin-bottom:4px;' +
         'margin-left: -1rem;' +
         'padding-left:1rem;' +
         'margin-right:-1rem;' +
         'border-color:rgba(68,138,255,.4)' +
     '}' +

    '.md .admonition.tip{' +
       'border-left:2.5rem solid rgba(50,255,90,.4);' +
       'background-color:rgba(50,255,90,.15)' +
    '}' +
                       
    '.md .admonition.tip::before{' +
       'content:"\\24d8";' +
       'font-weight:bold;' +
       'font-size:150%;' +
       'position:relative;' +
       'top:3px;' +
       'color:rgba(26,128,46,.8);' +
       'left:-2.95rem;' +
       'display:block;' +
       'width:0;' +
       'height:0' +
     '}' +

     '.md .admonition.tip>.admonition-title{' +
       'border-color:rgba(50,255,90,.4)' +
     '}' +

     '.md .admonition.warn,.md .admonition.warning{' +
       'border-left:2.5rem solid rgba(255,145,0,.4);' +
       'background-color:rgba(255,145,0,.15)' +
     '}' +

     '.md .admonition.warn::before,.md .admonition.warning::before{' +
       'content:"\\26A0";' +
       'font-weight:bold;' +
       'font-size:150%;' +
       'position:relative;' +
       'top:2px;' +
       'color:rgba(128,73,0,.8);' +
       'left:-2.95rem;' +
       'display:block;' +
       'width:0;' +
       'height:0' +
     '}' +

     '.md .admonition.warn>.admonition-title,.md .admonition.warning>.admonition-title{' +
      'border-color:rgba(255,145,0,.4)' +
     '}' +

     '.md .admonition.error{' +
      'border-left: 2.5rem solid rgba(255,23,68,.4);'+    
      'background-color:rgba(255,23,68,.15)' +
    '}' +

    '.md .admonition.error>.admonition-title{' +
      'border-color:rgba(255,23,68,.4)'+
    '}' +

    '.md .admonition.error::before{' + 
    'content: "\\2612";' +
    'font-family:"Arial";' +
    'font-size:200%;' +
    'position:relative;' +
    'color:rgba(128,12,34,.8);' +
    'top:-2px;' +
    'left:-3rem;' +
    'display:block;' +
    'width:0;' +
    'height:0' +
   '}' +
                       
   '.md .admonition p:last-child{margin-bottom:0}'  +

   '.md li.checked,.md li.unchecked{'+
    'list-style:none;'+
    'overflow:visible;'+
    'text-indent:-1.2em'+
                       '}' +
                       
   '.md li.checked:before,.md li.unchecked:before{' +
   'content:"\\2611";' +
   'display:block;'+
   'float:left;' +
   'width:1em;' +
   'font-size:120%'+
                       '}'+
                       
   '.md li.unchecked:before{'+
   'content:"\\2610"' +
   '}'

);

var MARKDEEP_LINE = '<!-- Markdeep: --><style class="fallback">body{visibility:hidden;white-space:pre;font-family:monospace}</style><script src="markdeep.min.js"></script><script src="https://casual-effects.com/markdeep/latest/markdeep.min.js?"></script><script>window.alreadyProcessedMarkdeep||(document.body.style.visibility="visible")</script>';

// Language options:
var FRENCH = {
    keyword: {
        table:     'tableau',
        figure:    'figure',
        listing:   'liste',
        diagram:   'diagramme',
        contents:  'Table des matières',

        sec:       'sec',
        section:   'section',
        subsection: 'paragraphe',
        chapter:   'chapitre',

        Monday:    'lundi',
        Tuesday:   'mardi',
        Wednesday: 'mercredi',
        Thursday:  'jeudi',
        Friday:    'vendredi',
        Saturday:  'samedi',
        Sunday:    'dimanche',

        January:   'Janvier',
        February:  'Février',
        March:     'Mars',
        April:     'Avril',
        May:       'Mai',
        June:      'Juin', 
        July:      'Juillet',
        August:    'Août', 
        September: 'Septembre', 
        October:   'Octobre', 
        November:  'Novembre',
        December:  'Décembre',

        jan: 'janv.',
        feb: 'févr.',
        mar: 'mars',
        apr: 'avril',
        may: 'mai',
        jun: 'juin',
        jul: 'juil.',
        aug: 'août',
        sep: 'sept.',
        oct: 'oct.',
        nov: 'nov.',
        dec: 'déc.',

        '&ldquo;': '&laquo;&nbsp;',
        '&rtquo;': '&nbsp;&raquo;'
    }
};

// Translated by "Warmist"
var LITHUANIAN = {
    keyword: {
        table:     'lentelė',
        figure:    'paveikslėlis',
        listing:   'sąrašas',
        diagram:   'diagrama',
        contents:  'Turinys',

        sec:       'sk',
        section:   'skyrius',
        subsection: 'poskyris',
        chapter:   'skyrius',

        Monday:    'pirmadienis',
        Tuesday:   'antradienis',
        Wednesday: 'trečiadienis',
        Thursday:  'ketvirtadienis',
        Friday:    'penktadienis',
        Saturday:  'šeštadienis',
        Sunday:    'sekmadienis',

        January:   'Sausis',
        February:  'Vasaris',
        March:     'Kovas',
        April:     'Balandis',
        May:       'Gegužė',
        June:      'Birželis',
        July:      'Liepa',
        August:    'Rugpjūtis',
        September: 'Rugsėjis',
        October:   'Spalis',
        November:  'Lapkritis',
        December:  'Gruodis',

        jan: 'saus',
        feb: 'vas',
        mar: 'kov',
        apr: 'bal',
        may: 'geg',
        jun: 'birž',
        jul: 'liep',
        aug: 'rugpj',
        sep: 'rugs',
        oct: 'spal',
        nov: 'lapkr',
        dec: 'gruod',

        '&ldquo;': '&bdquo;',
        '&rtquo;': '&ldquo;'
    }
};

    
// Translated by Zdravko Velinov
var BULGARIAN = {
    keyword: {
        table:     'таблица',
        figure:    'фигура',
        listing:   'списък',
        diagram:   'диаграма',

        contents:  'cъдържание',

        sec:       'сек',
        section:   'раздел',
        subsection: 'подраздел',
        chapter:   'глава',

        Monday:    'понеделник',
        Tuesday:   'вторник',
        Wednesday: 'сряда',
        Thursday:  'четвъртък',
        Friday:    'петък',
        Saturday:  'събота',
        Sunday:    'неделя',

        January:   'януари',
        February:  'февруари',
        March:     'март',
        April:     'април',
        May:       'май',
        June:      'юни', 
        July:      'юли',
        August:    'август', 
        September: 'септември', 
        October:   'октомври', 
        November:  'ноември',
        December:  'декември',

        jan: 'ян',
        feb: 'февр',
        mar: 'март',
        apr: 'апр',
        may: 'май',
        jun: 'юни',
        jul: 'юли',
        aug: 'авг',
        sep: 'септ',
        oct: 'окт',
        nov: 'ноем',
        dec: 'дек',

        '&ldquo;': '&bdquo;',
        '&rdquo;': '&rdquo;'
    }
};


// Translated by Tiago Antão
var PORTUGUESE = {
    keyword: {
        table:     'tabela',
        figure:    'figura',
        listing:   'lista',
        diagram:   'diagrama',
        contents:  'conteúdo',

        sec:       'sec',
        section:   'secção',
        subsection: 'subsecção',
        chapter:   'capítulo',

        Monday:    'Segunda-feira',
        Tuesday:   'Terça-feira',
        Wednesday: 'Quarta-feira',
        Thursday:  'Quinta-feira',
        Friday:    'Sexta-feira',
        Saturday:  'Sábado',
        Sunday:    'Domingo',

        January:   'Janeiro',
        February:  'Fevereiro',
        March:     'Março',
        April:     'Abril',
        May:       'Maio',
        June:      'Junho', 
        July:      'Julho',
        August:    'Agosto', 
        September: 'Setembro', 
        October:   'Outubro', 
        November:  'Novembro',
        December:  'Dezembro',

        jan: 'jan',
        feb: 'fev',
        mar: 'mar',
        apr: 'abr',
        may: 'mai',
        jun: 'jun',
        jul: 'jul',
        aug: 'ago',
        sep: 'set',
        oct: 'oct',
        nov: 'nov',
        dec: 'dez',

        '&ldquo;': '&laquo;',
        '&rtquo;': '&raquo;'
    }
};


// Translated by Jan Toušek
var CZECH = {
    keyword: {
        table:     'Tabulka',
        figure:    'Obrázek',
        listing:   'Seznam',
        diagram:   'Diagram',

        contents:  'Obsah',

        sec:       'kap.',  // Abbreviation for section
        section:   'kapitola',
        subsection:'podkapitola',
        chapter:   'kapitola',

        Monday:    'pondělí',
        Tuesday:   'úterý',
        Wednesday: 'středa',
        Thursday:  'čtvrtek',
        Friday:    'pátek',
        Saturday:  'sobota',
        Sunday:    'neděle',

        January:   'leden',
        February:  'únor',
        March:     'březen',
        April:     'duben',
        May:       'květen',
        June:      'červen',
        July:      'červenec',
        August:    'srpen',
        September: 'září',
        October:   'říjen',
        November:  'listopad',
        December:  'prosinec',

        jan: 'led',
        feb: 'úno',
        mar: 'bře',
        apr: 'dub',
        may: 'kvě',
        jun: 'čvn',
        jul: 'čvc',
        aug: 'srp',
        sep: 'zář',
        oct: 'říj',
        nov: 'lis',
        dec: 'pro',

        '&ldquo;': '&bdquo;',
        '&rdquo;': '&ldquo;'
    }
};


var ITALIAN = {
    keyword: {
        table:     'tabella',
        figure:    'figura',
        listing:   'lista',
        diagram:   'diagramma',
        contents:  'indice',

        sec:       'sez',
        section:   'sezione',
        subsection: 'paragrafo',
        chapter:   'capitolo',

        Monday:    'lunedì',
        Tuesday:   'martedì',
        Wednesday: 'mercoledì',
        Thursday:  'giovedì',
        Friday:    'venerdì',
        Saturday:  'sabato',
        Sunday:    'domenica',

        January:   'Gennaio',
        February:  'Febbraio',
        March:     'Marzo',
        April:     'Aprile',
        May:       'Maggio',
        June:      'Giugno', 
        July:      'Luglio',
        August:    'Agosto', 
        September: 'Settembre', 
        October:   'Ottobre', 
        November:  'Novembre',
        December:  'Dicembre',

        jan: 'gen',
        feb: 'feb',
        mar: 'mar',
        apr: 'apr',
        may: 'mag',
        jun: 'giu',
        jul: 'lug',
        aug: 'ago',
        sep: 'set',
        oct: 'ott',
        nov: 'nov',
        dec: 'dic',

        '&ldquo;': '&ldquo;',
        '&rtquo;': '&rdquo;'
    }
};

var RUSSIAN = {
    keyword: {
        table:     'таблица',
        figure:    'рисунок',
        listing:   'листинг',
        diagram:   'диаграмма',

        contents:  'Содержание',

        sec:       'сек',
        section:   'раздел',
        subsection: 'подраздел',
        chapter:   'глава',

        Monday:    'понедельник',
        Tuesday:   'вторник',
        Wednesday: 'среда',
        Thursday:  'четверг',
        Friday:    'пятница',
        Saturday:  'суббота',
        Sunday:    'воскресенье',

        January:   'январьr',
        February:  'февраль',
        March:     'март',
        April:     'апрель',
        May:       'май',
        June:      'июнь', 
        July:      'июль',
        August:    'август', 
        September: 'сентябрь', 
        October:   'октябрь', 
        November:  'ноябрь',
        December:  'декабрь',

        jan: 'янв',
        feb: 'февр',
        mar: 'март',
        apr: 'апр',
        may: 'май',
        jun: 'июнь',
        jul: 'июль',
        aug: 'авг',
        sep: 'сент',
        oct: 'окт',
        nov: 'ноябрь',
        dec: 'дек',
        
        '&ldquo;': '«',
        '&rdquo;': '»'
    }
};

// Translated by Dariusz Kuśnierek 
var POLISH = {
    keyword: {
        table:     'tabela',
        figure:    'ilustracja',
        listing:   'wykaz',
        diagram:   'diagram',
        contents:  'Spis treści',

        sec:       'rozdz.',
        section:   'rozdział',
        subsection: 'podrozdział',
        chapter:   'kapituła',

        Monday:    'Poniedziałek',
        Tuesday:   'Wtorek',
        Wednesday: 'Środa',
        Thursday:  'Czwartek',
        Friday:    'Piątek',
        Saturday:  'Sobota',
        Sunday:    'Niedziela',

        January:   'Styczeń',
        February:  'Luty',
        March:     'Marzec',
        April:     'Kwiecień',
        May:       'Maj',
        June:      'Czerwiec', 
        July:      'Lipiec',
        August:    'Sierpień', 
        September: 'Wrzesień', 
        October:   'Październik', 
        November:  'Listopad',
        December:  'Grudzień',

        jan: 'sty',
        feb: 'lut',
        mar: 'mar',
        apr: 'kwi',
        may: 'maj',
        jun: 'cze',
        jul: 'lip',
        aug: 'sie',
        sep: 'wrz',
        oct: 'paź',
        nov: 'lis',
        dec: 'gru',
        
        '&ldquo;': '&bdquo;',
        '&rdquo;': '&rdquo;'
    }
};

// Translated by Sandor Berczi
var HUNGARIAN = {
    keyword: {
        table:     'táblázat',
        figure:    'ábra',
        listing:   'lista',
        diagram:   'diagramm',

        contents:  'Tartalomjegyzék',

        sec:       'fej',  // Abbreviation for section
        section:   'fejezet',
        subsection:'alfejezet',
        chapter:   'fejezet',

        Monday:    'hétfő',
        Tuesday:   'kedd',
        Wednesday: 'szerda',
        Thursday:  'csütörtök',
        Friday:    'péntek',
        Saturday:  'szombat',
        Sunday:    'vasárnap',

        January:   'január',
        February:  'február',
        March:     'március',
        April:     'április',
        May:       'május',
        June:      'június',
        July:      'július',
        August:    'augusztus',
        September: 'szeptember',
        October:   'október',
        November:  'november',
        December:  'december',

        jan: 'jan',
        feb: 'febr',
        mar: 'márc',
        apr: 'ápr',
        may: 'máj',
        jun: 'jún',
        jul: 'júl',
        aug: 'aug',
        sep: 'szept',
        oct: 'okt',
        nov: 'nov',
        dec: 'dec',

        '&ldquo;': '&bdquo;',
        '&rdquo;': '&rdquo;'
    }
};

// Translated by Takashi Masuyama
var JAPANESE = {
    keyword: {
        table:     '表',
        figure:    '図',
        listing:   '一覧',
        diagram:   '図',
        contents:  '目次',

        sec:       '節',
        section:   '節',
        subsection: '項',
        chapter:   '章',

        Monday:    '月',
        Tuesday:   '火',
        Wednesday: '水',
        Thursday:  '木',
        Friday:    '金',
        Saturday:  '土',
        Sunday:    '日',

        January:   '1月',
        February:  '2月',
        March:     '3月',
        April:     '4月',
        May:       '5月',
        June:      '6月',
        July:      '7月',
        August:    '8月',
        September: '9月',
        October:   '10月',
        November:  '11月',
        December:  '12月',

        jan: '1月',
        feb: '2月',
        mar: '3月',
        apr: '4月',
        may: '5月',
        jun: '6月',
        jul: '7月',
        aug: '8月',
        sep: '9月',
        oct: '10月',
        nov: '11月',
        dec: '12月',

        '&ldquo;': '「',
        '&rdquo;': '」'
    }
};    
    
// Translated by Sandor Berczi
var GERMAN = {
    keyword: {
        table:     'Tabelle',
        figure:    'Abbildung',
        listing:   'Auflistung',
        diagram:   'Diagramm',

        contents:  'Inhaltsverzeichnis',

        sec:       'Kap',
        section:   'Kapitel',
        subsection:'Unterabschnitt',
        chapter:   'Kapitel',

        Monday:    'Montag',
        Tuesday:   'Dienstag',
        Wednesday: 'Mittwoch',
        Thursday:  'Donnerstag',
        Friday:    'Freitag',
        Saturday:  'Samstag',
        Sunday:    'Sonntag',

        January:   'Januar',
        February:  'Februar',
        March:     'März',
        April:     'April',
        May:       'Mai',
        June:      'Juni',
        July:      'Juli',
        August:    'August',
        September: 'September',
        October:   'Oktober',
        November:  'November',
        December:  'Dezember',

        jan: 'Jan',
        feb: 'Feb',
        mar: 'Mär',
        apr: 'Apr',
        may: 'Mai',
        jun: 'Jun',
        jul: 'Jul',
        aug: 'Aug',
        sep: 'Sep',
        oct: 'Okt',
        nov: 'Nov',
        dec: 'Dez',
        
        '&ldquo;': '&bdquo;',
        '&rdquo;': '&ldquo;'
    }
};

// Translated by Marcelo Arroyo
var SPANISH = {
    keyword: {
        table:     'Tabla',
        figure:    'Figura',
        listing:   'Listado',
        diagram:   'Diagrama',
        contents:  'Tabla de Contenidos',

        sec:       'sec',
        section:   'Sección',
        subsection: 'Subsección',
        chapter:    'Capítulo',

        Monday:    'Lunes',
        Tuesday:   'Martes',
        Wednesday: 'Miércoles',
        Thursday:  'Jueves',
        Friday:    'Viernes',
        Saturday:  'Sábado',
        Sunday:    'Domingo',

        January:   'Enero',
        February:  'Febrero',
        March:     'Marzo',
        April:     'Abril',
        May:       'Mayo',
        June:      'Junio',
        July:      'Julio',
        August:    'Agosto',
        September: 'Septiembre',
        October:   'Octubre',
        November:  'Noviembre',
        December:  'Diciembre',

        jan: 'ene',
        feb: 'feb',
        mar: 'mar',
        apr: 'abr',
        may: 'may',
        jun: 'jun',
        jul: 'jul',
        aug: 'ago',
        sep: 'sept',
        oct: 'oct',
        nov: 'nov',
        dec: 'dic',

        '&ldquo;': '&laquo;&nbsp;',
        '&rtquo;': '&nbsp;&raquo;'
    }
};

// Translated by Nils Nilsson
var SWEDISH = {
    keyword: {
        table:     'tabell',
        figure:    'figur',
        listing:   'lista',
        diagram:   'diagram',

        contents:  'Innehållsförteckning',
        sec:       'sek',
        section:   'sektion',
        subsection:'sektion',
        chapter:   'kapitel',

        Monday:    'måndag',
        Tuesday:   'tisdag',
        Wednesday: 'onsdag',
        Thursday:  'torsdag',
        Friday:    'fredag',
        Saturday:  'lördag',
        Sunday:    'söndag',

        January:   'januari',
        February:  'februari',
        March:     'mars',
        April:     'april',
        May:       'maj',
        June:      'juni',
        July:      'juli',
        August:    'augusti',
        September: 'september',
        October:   'oktober',
        November:  'november',
        December:  'december',

        jan: 'jan',
        feb: 'feb',
        mar: 'mar',
        apr: 'apr',
        may: 'maj',
        jun: 'jun',
        jul: 'jul',
        aug: 'aug',
        sep: 'sep',
        oct: 'okt',
        nov: 'nov',
        dec: 'dec',
        
        '&ldquo;': '&rdquo;',
        '&rdquo;': '&rdquo;'
    }
};


// Translated by Marc Izquierdo
var CATALAN = {
    keyword: {
        table:     'Taula',
        figure:    'Figura',
        listing:   'Llistat',
        diagram:   'Diagrama',
        contents:  'Taula de Continguts',

        sec:        'sec',
        section:    'Secció',
        subsection: 'Subsecció',
        chapter:    'Capítol',

        Monday:    'Dilluns',
        Tuesday:   'Dimarts',
        Wednesday: 'Dimecres',
        Thursday:  'Dijous',
        Friday:    'Divendres',
        Saturday:  'Dissabte',
        Sunday:    'Dimenge',

        January:   'Gener',
        February:  'Febrer',
        March:     'Març',
        April:     'Abril',
        May:       'Maig',
        June:      'Juny',
        July:      'Juliol',
        August:    'Agost',
        September: 'Septembre',
        October:   'Octubre',
        November:  'Novembre',
        December:  'Desembre',

        jan: 'gen',
        feb: 'feb',
        mar: 'mar',
        apr: 'abr',
        may: 'mai',
        jun: 'jun',
        jul: 'jul',
        aug: 'ago',
        sep: 'sept',
        oct: 'oct',
        nov: 'nov',
        dec: 'des',

        '&ldquo;': '&laquo;&nbsp;',
        '&rtquo;': '&nbsp;&raquo;'
    }
};
 
var DEFAULT_OPTIONS = {
    mode:               'markdeep',
    detectMath:         true,
    lang:               {keyword:{}}, // English
    tocStyle:           'auto',
    hideEmptyWeekends:  true,
    showLabels:         false,
    sortScheduleLists:  true,
    definitionStyle:    'auto',
    linkAPIDefinitions: false,
    inlineCodeLang:     false,
    scrollThreshold:    90,
    captionAbove:       {diagram: false,
                         image:   false,
                         table:   false,
                         listing: false},
    smartQuotes:        true
};


// See http://www.i18nguy.com/unicode/language-identifiers.html for keys
var LANG_TABLE = {
    en: {keyword:{}},        
    ru: RUSSIAN,
    fr: FRENCH,
    pl: POLISH,
    bg: BULGARIAN,
    de: GERMAN,
    hu: HUNGARIAN,
    sv: SWEDISH,
    pt: PORTUGUESE,
    ja: JAPANESE,
    it: ITALIAN,
    lt: LITHUANIAN,
    cz: CZECH,
    es: SPANISH,
    'es-ES': SPANISH,
    'es-ca': CATALAN
    // Contribute your language here! I only accept translations
    // from native speakers.
};

[].slice.call(document.getElementsByTagName('meta')).forEach(function(elt) {
    var att = elt.getAttribute('lang');
    if (att) {
        var lang = LANG_TABLE[att];
        if (lang) {
            DEFAULT_OPTIONS.lang = lang;
        }
    }
});


var max = Math.max;
var min = Math.min;
var abs = Math.abs;
var sign = Math.sign || function (x) {
    return ( +x === x ) ? ((x === 0) ? x : (x > 0) ? 1 : -1) : NaN;
};


/** Get an option, or return the corresponding value from DEFAULT_OPTIONS */
function option(key, key2) {
    if (window.markdeepOptions && (window.markdeepOptions[key] !== undefined)) {
        var val = window.markdeepOptions[key];
        if (key2) {
            val = val[key2]
            if (val !== undefined) {
                return val;
            } else {
                return DEFAULT_OPTIONS[key][key2];
            }
        } else {
            return window.markdeepOptions[key];
        }
    } else if (DEFAULT_OPTIONS[key] !== undefined) {
        if (key2) {
            return DEFAULT_OPTIONS[key][key2];
        } else {
            return DEFAULT_OPTIONS[key];
        }
    } else {
        console.warn('Illegal option: "' + key + '"');
        return undefined;
    }
}


function maybeShowLabel(url, tag) {
    if (option('showLabels')) {
        var text = ' {\u00A0' + url + '\u00A0}';
        return tag ? entag(tag, text) : text;
    } else {
        return '';
    }
}


// Returns the localized version of word, defaulting to the word itself
function keyword(word) {
    return option('lang').keyword[word] || option('lang').keyword[word.toLowerCase()] || word;
}


/** Converts <>&" to their HTML escape sequences */
function escapeHTMLEntities(str) {
    return String(str).rp(/&/g, '&amp;').rp(/</g, '&lt;').rp(/>/g, '&gt;').rp(/"/g, '&quot;');
}


/** Restores the original source string's '<' and '>' as entered in
    the document, before the browser processed it as HTML. There is no
    way in an HTML document to distinguish an entity that was entered
    as an entity. */
function unescapeHTMLEntities(str) {
    // Process &amp; last so that we don't recursively unescape
    // escaped escape sequences.
    return str.
        rp(/&lt;/g, '<').
        rp(/&gt;/g, '>').
        rp(/&quot;/g, '"').
        rp(/&#39;/g, "'").
        rp(/&ndash;/g, '\u2013').
        rp(/&mdash;/g, '---').
        rp(/&amp;/g, '&');
}


function removeHTMLTags(str) {
    return str.rp(/<.*?>/g, '');
}


/** Turn the argument into a legal URL anchor */
function mangle(text) {
    return encodeURI(text.rp(/\s/g, '').toLowerCase());
}

/** Creates a style sheet containing elements like:

  hn::before { 
    content: counter(h1) "." counter(h2) "." ... counter(hn) " "; 
    counter-increment: hn; 
   } 
*/
function sectionNumberingStylesheet() {
    var s = '';

    for (var i = 1; i <= 6; ++i) {
        s += '.md h' + i + '::before {\ncontent:';
        for (var j = 1; j <= i; ++j) {
            s += 'counter(h' + j + ') "' + ((j < i) ? '.' : ' ') + '"';
        }
        s += ';\ncounter-increment: h' + i + ';margin-right:10px}\n\n';
    }

    return entag('style', s);
}

/**
   \param node  A node from an HTML DOM

   \return A String that is a very good reconstruction of what the
   original source looked like before the browser tried to correct
   it to legal HTML.
 */
function nodeToMarkdeepSource(node, leaveEscapes) {
    var source = node ? node.innerHTML : '';

    // Markdown uses <john@bar.com> email syntax, which HTML parsing
    // will try to close by inserting the matching close tags at the end of the
    // document. Remove anything that looks like that and comes *after*
    // the first fallback style.
    //source = source.rp(/<style class="fallback">[\s\S]*?<\/style>/gi, '');
    
    // Remove artificially inserted close tags from URLs and
    source = source.rp(/<\/https?:.*>|<\/ftp:.*>|<\/[^ "\t\n>]+@[^ "\t\n>]+>/gi, '');
    
    // Now try to fix the URLs themselves, which will be 
    // transformed like this: <http: casual-effects.com="" markdeep="">
    source = source.rp(/<(https?|ftp): (.*?)>/gi, function (match, protocol, list) {

        // Remove any quotes--they wouldn't have been legal in the URL anyway
        var s = '<' + protocol + '://' + list.rp(/=""\s/g, '/');

        if (s.ss(s.length - 3) === '=""') {
            s = s.ss(0, s.length - 3);
        }

        // Remove any lingering quotes (since they
        // wouldn't have been legal in the URL)
        s = s.rp(/"/g, '');

        return s + '>';
    });

    // Remove the "fallback" style tags
    source = source.rp(/<style class=["']fallback["']>.*?<\/style>/gmi, '');

    source = unescapeHTMLEntities(source);

    return source;
}


/** Extracts one diagram from a Markdown string.

    Returns {beforeString, diagramString, alignmentHint, afterString}
    diagramString will be empty if nothing was found. The
    DIAGRAM_MARKER is stripped from the diagramString. 

    alignmentHint may be:
    floatleft  
    floatright
    center
    flushleft

    diagramString does not include the marker characters. 
    If there is a caption, it will appear in the afterString and not be parsed.
*/
function extractDiagram(sourceString) {
    // Returns the number of wide Unicode symbols (outside the BMP) in string s between indices
    // start and end - 1
    function unicodeSyms(s, start, end) {
        var p = start;
        for (var i = start; i < end; ++i, ++p) {
            var c = s.charCodeAt(p);
            p += (c >= 0xD800) && (c <= 0xDBFF);
        }
        return p - end;
    }

    function advance() {
        nextLineBeginning = sourceString.indexOf('\n', lineBeginning) + 1;
        wideCharacters = unicodeSyms(sourceString, lineBeginning + xMin, lineBeginning + xMax);
        textOnLeft  = textOnLeft  || /\S/.test(sourceString.ss(lineBeginning, lineBeginning + xMin));
        noRightBorder = noRightBorder || (sourceString[lineBeginning + xMax + wideCharacters] !== '*');

        // Text on the right ... if the line is not all '*'
        textOnRight = ! noRightBorder && (textOnRight || /[^ *\t\n\r]/.test(sourceString.ss(lineBeginning + xMax + wideCharacters + 1, nextLineBeginning)));
    }

    var noDiagramResult = {beforeString: sourceString, diagramString: '', alignmentHint: '', afterString: ''};

    // Search sourceString for the first rectangle of enclosed
    // DIAGRAM_MARKER characters at least DIAGRAM_START.length wide
    for (var i = sourceString.indexOf(DIAGRAM_START);
         i >= 0;
         i = sourceString.indexOf(DIAGRAM_START, i + DIAGRAM_START.length)) {

        // We found what looks like a diagram start. See if it has either a full border of
        // aligned '*' characters, or top-left-bottom borders and nothing but white space on
        // the left.
        
        // Look backwards to find the beginning of the line (or of the string)
        // and measure the start character relative to it
        var lineBeginning = max(0, sourceString.lastIndexOf('\n', i)) + 1;
        var xMin = i - lineBeginning;
        
        // Find the first non-diagram character on this line...or the end of the entire source string
        var j;
        for (j = i + DIAGRAM_START.length; sourceString[j] === DIAGRAM_MARKER; ++j) {}
        var xMax = j - lineBeginning - 1;
        
        // We have a potential hit. Start accumulating a result. If there was anything
        // between the newline and the diagram, move it to the after string for proper alignment.
        var result = {
            beforeString: sourceString.ss(0, lineBeginning), 
            diagramString: '',
            alignmentHint: 'center', 
            afterString: sourceString.ss(lineBeginning, i).rp(/[ \t]+$/, ' ')
        };

        var nextLineBeginning = 0, wideCharacters = 0;
        var textOnLeft = false, textOnRight = false;
        var noRightBorder = false;

        advance();
                                  
        // Now, see if the pattern repeats on subsequent lines
        for (var good = true, previousEnding = j; good; ) {
            // Find the next line
            lineBeginning = nextLineBeginning;
            advance();
            if (lineBeginning === 0) {
                // Hit the end of the string before the end of the pattern
                return noDiagramResult; 
            }
            
            if (textOnLeft) {
                // Even if there is text on *both* sides
                result.alignmentHint = 'floatright';
            } else if (textOnRight) {
                result.alignmentHint = 'floatleft';
            }
            
            // See if there are markers at the correct locations on the next line
            if ((sourceString[lineBeginning + xMin] === DIAGRAM_MARKER) && 
                (! textOnLeft || (sourceString[lineBeginning + xMax + wideCharacters] === DIAGRAM_MARKER))) {

                // See if there's a complete line of DIAGRAM_MARKER, which would end the diagram
                var x;
                for (x = xMin; (x < xMax) && (sourceString[lineBeginning + x] === DIAGRAM_MARKER); ++x) {}
           
                var begin = lineBeginning + xMin;
                var end   = lineBeginning + xMax + wideCharacters;
                
                if (! textOnLeft) {
                    // This may be an incomplete line
                    var newlineLocation = sourceString.indexOf('\n', begin);
                    if (newlineLocation !== -1) {
                        end = Math.min(end, newlineLocation);
                    }
                }

                // Trim any excess whitespace caused by our truncation because Markdown will
                // interpret that as fixed-formatted lines
                result.afterString += sourceString.ss(previousEnding, begin).rp(/^[ \t]*[ \t]/, ' ').rp(/[ \t][ \t]*$/, ' ');
                if (x === xMax) {
                    // We found the last row. Put everything else into
                    // the afterString and return the result.
                
                    result.afterString += sourceString.ss(lineBeginning + xMax + 1);
                    return result;
                } else {
                    // A line of a diagram. Extract everything before
                    // the diagram line started into the string of
                    // content to be placed after the diagram in the
                    // final HTML
                    result.diagramString += sourceString.ss(begin + 1, end) + '\n';
                    previousEnding = end + 1;
                }
            } else {
                // Found an incorrectly delimited line. Abort
                // processing of this potential diagram, which is now
                // known to NOT be a diagram after all.
                good = false;
            }
        } // Iterate over verticals in the potential box
    } // Search for the start

    return noDiagramResult;
}

/** 
    Find the specified delimiterRegExp used as a quote (e.g., *foo*)
    and replace it with the HTML tag and optional attributes.
*/
function replaceMatched(string, delimiterRegExp, tag, attribs) {
    var delimiter = delimiterRegExp.source;
    var flanking = '[^ \\t\\n' + delimiter + ']';
    var pattern  = '([^A-Za-z0-9])(' + delimiter + ')' +
        '(' + flanking + '.*?(\\n.+?)*?)' + 
        delimiter + '(?![A-Za-z0-9])';

    return string.rp(new RegExp(pattern, 'g'), 
                          '$1<' + tag + (attribs ? ' ' + attribs : '') +
                          '>$3</' + tag + '>');
}
    
/** Maruku ("github")-style table processing */
function replaceTables(s, protect) {
    var TABLE_ROW       = /(?:\n[ \t]*(?:(?:\|?[ \t\S]+?(?:\|[ \t\S]+?)+\|?)|\|[ \t\S]+\|)(?=\n))/.source;
    var TABLE_SEPARATOR = /\n[ \t]*(?:(?:\|? *\:?-+\:?(?: *\| *\:?-+\:?)+ *\|?|)|\|[\:-]+\|)(?=\n)/.source;
    var TABLE_CAPTION   = /\n[ \t]*\[[^\n\|]+\][ \t]*(?=\n)/.source;
    var TABLE_REGEXP    = new RegExp(TABLE_ROW + TABLE_SEPARATOR + TABLE_ROW + '+(' + TABLE_CAPTION + ')?', 'g');

    function trimTableRowEnds(row) {
        return row.trim().rp(/^\||\|$/g, '');
    }

    s = s.rp(TABLE_REGEXP, function (match) {
        // Found a table, actually parse it by rows
        var rowArray = match.split('\n');
        
        var result = '';
        
        // Skip the bogus leading row
        var startRow = (rowArray[0] === '') ? 1 : 0;

        var caption = rowArray[rowArray.length - 1].trim();

        if ((caption.length > 3) && (caption[0] === '[') && (caption[caption.length - 1] === ']')) {
            // Remove the caption from the row array
            rowArray.pop();
            caption = caption.ss(1, caption.length - 1);
        } else {
            caption = undefined;
        }

        // Parse the separator row for left/center/right-indicating colons
        var columnStyle = [];
        trimTableRowEnds(rowArray[startRow + 1]).rp(/:?-+:?/g, function (match) {
            var left = (match[0] === ':');
            var right = (match[match.length - 1] === ':');
            columnStyle.push(protect(' style="text-align:' + ((left && right) ? 'center' : (right ? 'right' : 'left')) + '"'));
        });

        var row = rowArray[startRow + 1].trim();
        var hasLeadingBar  = row[0] === '|';
        var hasTrailingBar = row[row.length - 1] === '|';
        
        var tag = 'th';
        
        for (var r = startRow; r < rowArray.length; ++r) {
            // Remove leading and trailing whitespace and column delimiters
            row = rowArray[r].trim();
            
            if (! hasLeadingBar && (row[0] === '|')) {
                // Empty first column
                row = '&nbsp;' + row;
            }
            
            if (! hasTrailingBar && (row[row.length - 1] === '|')) {
                // Empty last column
                row += '&nbsp;';
            }
            
            row = trimTableRowEnds(row);
            var i = 0;
            result += entag('tr', '<' + tag + columnStyle[0] + '> ' + 
                            row.rp(/ *\| */g, function () {
                                ++i;
                                return ' </' + tag + '><' + tag + columnStyle[i] + '> ';
                            }) + ' </' + tag + '>') + '\n';
            
            // Skip the header-separator row
            if (r == startRow) { 
                ++r; 
                tag = 'td';
            }
        }
        
        result = entag('table', result, protect('class="table"'));

        if (caption) {
            caption = entag('center', entag('div', caption, protect('class="tablecaption"')));
            if (option('captionAbove', 'table')) {
                result = caption + result;
            } else {
                result = '\n' + result + caption;
            }
        }

        return entag('div', result, "class='table'");
    });

    return s;
}


function replaceLists(s, protect) {
    // Identify task list bullets in a few patterns and reformat them to a standard format for
    // easier processing.
    s = s.rp(/^(\s*)(?:-\s*)?(?:\[ \]|\u2610)(\s+)/mg, '$1\u2610$2');
    s = s.rp(/^(\s*)(?:-\s*)?(?:\[[xX]\]|\u2611)(\s+)/mg, '$1\u2611$2');
        
    // Identify list blocks:
    // Blank line or line ending in colon, line that starts with #., *, +, -, ☑, or ☐
    // and then any number of lines until another blank line
    var BLANK_LINES = /\n\s*\n/.source;

    // Preceding line ending in a colon

    // \u2610 is the ballot box (unchecked box) character
    var PREFIX     = /[:,]\s*\n/.source;
    var LIST_BLOCK_REGEXP = 
        new RegExp('(' + PREFIX + '|' + BLANK_LINES + '|<p>\s*\n|<br/>\s*\n?)' +
                   /((?:[ \t]*(?:\d+\.|-|\+|\*|\u2611|\u2610)(?:[ \t]+.+\n(?:[ \t]*\n)?)+)+)/.source, 'gm');

    var keepGoing = true;

    var ATTRIBS = {'+': protect('class="plus"'), '-': protect('class="minus"'), '*': protect('class="asterisk"'),
                   '\u2611': protect('class="checked"'), '\u2610': protect('class="unchecked"')};
    var NUMBER_ATTRIBS = protect('class="number"');

    // Sometimes the list regexp grabs too much because subsequent lines are indented *less*
    // than the first line. So, if that case is found, re-run the regexp.
    while (keepGoing) {
        keepGoing = false;
        s = s.rp(LIST_BLOCK_REGEXP, function (match, prefix, block) {
            var result = prefix;
            
            // Contains {indentLevel, tag}
            var stack = [];
            var current = {indentLevel: -1};
            
            /* function logStack(stack) {
               var s = '[';
               stack.forEach(function(v) { s += v.indentLevel + ', '; });
               console.log(s.ss(0, s.length - 2) + ']');
               } */
            block.split('\n').forEach(function (line) {
                var trimmed     = line.rp(/^\s*/, '');
                
                var indentLevel = line.length - trimmed.length;
                
                // Add a CSS class based on the type of list bullet
                var attribs = ATTRIBS[trimmed[0]];
                var isUnordered = !! attribs; // JavaScript for: attribs !== undefined
                attribs = attribs || NUMBER_ATTRIBS;
                var isOrdered   = /^\d+\.[ \t]/.test(trimmed);
                var isBlank     = trimmed === '';
                var start       = isOrdered ? ' ' + protect('start=' + trimmed.match(/^\d+/)[0]) : '';

                if (isOrdered || isUnordered) {
                    // Add the indentation for the bullet itself
                    indentLevel += 2;
                }

                if (! current) {
                    // Went below top-level indent
                    result += '\n' + line;
                } else if (! isOrdered && ! isUnordered && (isBlank || (indentLevel >= current.indentLevel))) {
                    // Line without a marker
                    result += '\n' + current.indentChars + line;
                } else {
                    //console.log(indentLevel + ":" + line);
                    if (indentLevel !== current.indentLevel) {
                        // Enter or leave indentation level
                        if ((current.indentLevel !== -1) && (indentLevel < current.indentLevel)) {
                            while (current && (indentLevel < current.indentLevel)) {
                                stack.pop();
                                // End the current list and decrease indentation
                                result += '\n</li></' + current.tag + '>';
                                current = stack[stack.length - 1];
                            }
                        } else {
                            // Start a new list that is more indented
                            current = {indentLevel: indentLevel,
                                       tag:         isOrdered ? 'ol' : 'ul',
                                       // Subtract off the two indent characters we added above
                                       indentChars: line.ss(0, indentLevel - 2)};
                            stack.push(current);
                            result += '\n<' + current.tag + start + '>';
                        }
                    } else if (current.indentLevel !== -1) {
                        // End previous list item, if there was one
                        result += '\n</li>';
                    } // Indent level changed
                    
                    if (current) {
                        // Add the list item
                        result += '\n' + current.indentChars + '<li ' + attribs + '>' + trimmed.rp(/^(\d+\.|-|\+|\*|\u2611|\u2610) /, '');
                    } else {
                        // Just reached something that is *less* indented than the root--
                        // copy forward and then re-process that list
                        result += '\n' + line;
                        keepGoing = true;
                    }
                }
            }); // For each line

            // Remove trailing whitespace
            result = result.replace(/\s+$/,'');
            
            // Finish the last item and anything else on the stack (if needed)
            for (current = stack.pop(); current; current = stack.pop()) {
                result += '</li></' + current.tag + '>';
            }
       
            return result + '\n\n';
        });
    } // while keep going

    return s;
}


/** 
    Identifies schedule lists, which look like:

  date: title
    events

  Where date must contain a day, month, and four-number year and may
  also contain a day of the week.  Note that the date must not be
  indented and the events must be indented.

  Multiple events per date are permitted.
*/
function replaceScheduleLists(str, protect) {
    // Must open with something other than indentation or a list
    // marker.  There must be a four-digit number somewhere on the
    // line. Exclude lines that begin with an HTML tag...this will
    // avoid parsing headers that have dates in them.
    var BEGINNING = /^(?:[^\|<>\s-\+\*\d].*[12]\d{3}(?!\d).*?|(?:[12]\d{3}(?!\.).*\d.*?)|(?:\d{1,3}(?!\.).*[12]\d{3}(?!\d).*?))/.source;

    // There must be at least one more number in a date, a colon, and then some more text
    var DATE_AND_TITLE = '(' + BEGINNING + '):' + /[ \t]+([^ \t\n].*)\n/.source;

    // The body of the schedule item. It may begin with a blank line and contain
    // multiple paragraphs separated by blank lines...as long as there is indenting
    var EVENTS = /(?:[ \t]*\n)?((?:[ \t]+.+\n(?:[ \t]*\n){0,3})*)/.source;
    var ENTRY = DATE_AND_TITLE + EVENTS;

    var BLANK_LINE = '\n[ \t]*\n';
    var ENTRY_REGEXP = new RegExp(ENTRY, 'gm');

    var rowAttribs = protect('valign="top"');
    var dateTDAttribs = protect('style="width:100px;padding-right:15px" rowspan="2"');
    var eventTDAttribs = protect('style="padding-bottom:25px"');

    var DAY_NAME   = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'].map(keyword);
    var MONTH_NAME = ['jan', 'feb', 'mar', 'apr', 'may', 'jun', 'jul', 'aug', 'sep', 'oct', 'nov', 'dec'].map(keyword);
    var MONTH_FULL_NAME = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'].map(keyword);

    function clean(s) { return s.toLowerCase().rp('.', ''); }
    var LOWERCASE_MONTH_NAME = MONTH_NAME.map(clean);
    var LOWERCASE_MONTH_FULL_NAME = MONTH_FULL_NAME.map(clean);

    // Allow a period (and capture it) after each word, but eliminate
    // the periods that are in abbreviations so that they do not appear
    // in the regexp as wildcards or word breaks
    var MONTH_NAME_LIST = '\\b' + MONTH_NAME.concat(MONTH_FULL_NAME).join('(?:\\.|\\b)|\\b').rp(/([^\\])\./g, '$1') + '(?:\\.|\\b)';

    // Used to mark the center of each day. Not close to midnight to avoid daylight
    // savings problems.
    var standardHour = 9;

    try {
        var scheduleNumber = 0;
        str = str.rp(new RegExp(BLANK_LINE + '(' + ENTRY + '){2,}', 'gm'),
                     function (schedule) {
                       ++scheduleNumber;
                       // Each entry has the form {date:date, title:string, text:string}
                       var entryArray = [];

                       // Now parse the schedule into individual day entries

                       var anyWeekendEvents = false;

                       schedule.rp
                         (ENTRY_REGEXP,
                          function (entry, date, title, events) {
                              // Remove the day from the date (we'll reconstruct it below). This is actually unnecessary, since we
                              // explicitly compute the value anyway and the parser is robust to extra characters, but it aides
                              // in debugging.
                              // 
                              // date = date.rp(/(?:(?:sun|mon|tues|wednes|thurs|fri|satur)day|(?:sun|mon|tue|wed|thu|fri|sat)\.?|(?:su|mo|tu|we|th|fr|sa)),?/gi, '');
                              
                              // Parse the date. The Javascript Date class's parser is useless because it
                              // is locale dependent, so we do this with a regexp.
                              
                              var year = '', month = '', day = '', parenthesized = false;
                              
                              date = date.trim();
                              
                              if ((date[0] === '(') && (date.slice(-1) === ')')) {
                                  // This is a parenthesized entry
                                  date = date.slice(1, -1);
                                  parenthesized = true;
                              }
                              
                              // DD MONTH YYYY
                              var DD_MONTH_YYYY = RegExp('([0123]?\\d)\\D+([01]?\\d|' + MONTH_NAME_LIST + ')\\D+([12]\\d{3})', 'i')
                              var match = date.match(DD_MONTH_YYYY);
                              
                              if (match) {
                                  day = match[1]; month = match[2]; year = match[3];
                              } else {
                                  // YYYY MONTH DD
                                  match = date.match(RegExp('([12]\\d{3})\\D+([01]?\\d|' + MONTH_NAME_LIST + ')\\D+([0123]?\\d)', 'i')); 
                                  if (match) {
                                      day = match[3]; month = match[2]; year = match[1];
                                  } else {
                                      // MONTH DD YYYY
                                      match = date.match(RegExp('(' + MONTH_NAME_LIST + ')\\D+([0123]?\\d)\\D+([12]\\d{3})', 'i'));
                                      if (match) {
                                          day = match[2]; month = match[1]; year = match[3];
                                      } else {
                                          throw "Could not parse date";
                                      }
                                  }
                              }
                              
                              // Reconstruct standardized date format
                              date = day + '&nbsp;' + keyword(month) + '&nbsp;' + year;

                              // Detect the month
                              var monthNumber = parseInt(month) - 1;
                              if (isNaN(monthNumber)) {
                                  var target = clean(month);
                                  monthNumber = LOWERCASE_MONTH_NAME.indexOf(target);
                                  if (monthNumber === -1) {
                                      monthNumber = LOWERCASE_MONTH_FULL_NAME.indexOf(target);
                                  }
                              }

                              var dateVal = new Date(Date.UTC(parseInt(year), monthNumber, parseInt(day), standardHour));
                              // Reconstruct the day of the week
                              var dayOfWeek = dateVal.getUTCDay();
                              date = DAY_NAME[dayOfWeek] + '<br/>' + date;
                              
                              anyWeekendEvents = anyWeekendEvents || (dayOfWeek === 0) || (dayOfWeek === 6);
                              
                              entryArray.push({date: dateVal, 
                                               title: title,
                                               sourceOrder: entryArray.length,
                                               parenthesized: parenthesized,
                                               
                                               // Don't show text if parenthesized with no body
                                               text: parenthesized ? '' :
                                               entag('tr',
                                                     entag('td', 
                                                           '<a ' + protect('class="target" name="schedule' + scheduleNumber + '_' + dateVal.getUTCFullYear() + '-' + (dateVal.getUTCMonth() + 1) + '-' + dateVal.getUTCDate() + '"') + '>&nbsp;</a>' +
                                                           date, dateTDAttribs) + 
                                                     entag('td', entag('b', title)), rowAttribs) + 
                                               entag('tr', entag('td', '\n\n' + events, eventTDAttribs), rowAttribs)});
                              
                              return '';
                          });
                         
                         // Shallow copy the entries to bypass sorting if needed
                         var sourceEntryArray = option('sortScheduleLists') ? entryArray : entryArray.slice(0);

                         // Sort by date
                         entryArray.sort(function (a, b) {
                             // Javascript's sort is not specified to be
                             // stable, so we have to preserve
                             // sourceOrder in ties.
                             var ta = a.date.getTime();
                             var tb = b.date.getTime();
                             return (ta === tb) ? (a.sourceOrder - b.sourceOrder) : (ta - tb);
                         });
                         
                         var MILLISECONDS_PER_DAY = 1000 * 60 * 60 * 24;
                         
                         // May be slightly off due to daylight savings time
                         var approximateDaySpan = (entryArray[entryArray.length - 1].date.getTime() - entryArray[0].date.getTime()) / MILLISECONDS_PER_DAY;
                         
                         var today = new Date();
                         // Move back to midnight
                         today = new Date(Date.UTC(today.getUTCFullYear(), today.getUTCMonth(), today.getUTCDate(), standardHour));
                         
                         var calendar = '';
                         // Make a calendar view with links, if suitable
                         if ((approximateDaySpan > 14) && (approximateDaySpan / entryArray.length < 16)) {
                             var DAY_HEADER_ATTRIBS = protect('colspan="2" width="14%" style="padding-top:5px;text-align:center;font-style:italic"');
                             var DATE_ATTRIBS       = protect('width="1%" height="30px" style="text-align:right;border:1px solid #EEE;border-right:none;"');
                             var FADED_ATTRIBS      = protect('width="1%" height="30px" style="color:#BBB;text-align:right;"');
                             var ENTRY_ATTRIBS      = protect('width="14%" style="border:1px solid #EEE;border-left:none;"');
                             var PARENTHESIZED_ATTRIBS = protect('class="parenthesized"');
                             
                             // Find the first day of the first month
                             var date = entryArray[0].date;
                             var index = 0;
                             
                             var hideWeekends = ! anyWeekendEvents && option('hideEmptyWeekends');
                             var showDate = hideWeekends ? function(date) { return (date.getUTCDay() > 0) && (date.getUTCDay() < 6);} : function() { return true; };
                             
                             var sameDay = function (d1, d2) {
                                 // Account for daylight savings time
                                 return (abs(d1.getTime() - d2.getTime()) < MILLISECONDS_PER_DAY / 2);
                             }
                             
                             // Go to the first of the month
                             date = new Date(date.getUTCFullYear(), date.getUTCMonth(), 1, standardHour);
                             
                             while (date.getTime() < entryArray[entryArray.length - 1].date.getTime()) {
                                 
                                 // Create the calendar header
                                 calendar += '<table ' + protect('class="calendar"') + '>\n' +
                                     entag('tr', entag('th', MONTH_FULL_NAME[date.getUTCMonth()] + ' ' + date.getUTCFullYear(), protect('colspan="14"'))) + '<tr>';
                                 
                                 (hideWeekends ? DAY_NAME.slice(1, 6) : DAY_NAME).forEach(function (name) {
                                     calendar += entag('td', name, DAY_HEADER_ATTRIBS);
                                 });
                                 calendar += '</tr>';
                                 
                                 // Go back into the previous month to reach a Sunday. Check the time at noon
                                 // to avoid problems with daylight saving time occuring early in the morning
                                 while (date.getUTCDay() !== 0) { 
                                     date = new Date(date.getTime() - MILLISECONDS_PER_DAY); 
                                 }
                                 
                                 // Insert the days from the previous month
                                 if (date.getDate() !== 1) {
                                     calendar += '<tr ' + rowAttribs + '>';
                                     while (date.getDate() !== 1) {
                                         if (showDate(date)) { calendar += '<td ' + FADED_ATTRIBS + '>' + date.getUTCDate() + '</td><td>&nbsp;</td>'; }
                                         date = new Date(date.getTime() + MILLISECONDS_PER_DAY);
                                     }
                                 }
                                 
                                 // Run until the end of the month
                                 do {
                                     if (date.getUTCDay() === 0) {
                                         // Sunday, start a row
                                         calendar += '<tr ' + rowAttribs + '>';
                                     }
                                     
                                     if (showDate(date)) {
                                         var attribs = '';
                                         if (sameDay(date, today)) {
                                             attribs = protect('class="today"');
                                         }
                                         
                                         // Insert links as needed from entries
                                         var contents = '';
                                         
                                         for (var entry = entryArray[index]; entry && sameDay(entry.date, date); ++index, entry = entryArray[index]) {
                                             if (contents) { contents += '<br/>'; }
                                             if (entry.parenthesized) {
                                                 // Parenthesized with no body, no need for a link
                                                 contents += entag('span', entry.title, PARENTHESIZED_ATTRIBS);
                                             } else {
                                                 contents += entag('a', entry.title, protect('href="#schedule' + scheduleNumber + '_' + date.getUTCFullYear() + '-' + (date.getUTCMonth() + 1) + '-' + date.getUTCDate() + '"'));
                                             }
                                         }
                                         
                                         if (contents) {
                                             calendar += entag('td', entag('b', date.getUTCDate()), DATE_ATTRIBS + attribs) + entag('td', contents, ENTRY_ATTRIBS + attribs);
                                         } else {
                                             calendar += '<td ' + DATE_ATTRIBS + attribs + '></a>' + date.getUTCDate() + '</td><td ' + ENTRY_ATTRIBS + attribs + '> &nbsp; </td>';
                                         }
                                     }                                   
                                     
                                     if (date.getUTCDay() === 6) {
                                         // Saturday, end a row
                                         calendar += '</tr>';
                                     }
                                     
                                     // Go to (approximately) the next day
                                     date = new Date(date.getTime() + MILLISECONDS_PER_DAY);
                                 } while (date.getUTCDate() > 1);
                                 
                               // Finish out the week after the end of the month
                               if (date.getUTCDay() !== 0) {
                                   while (date.getUTCDay() !== 0) {
                                       if (showDate(date)) { calendar += '<td ' + FADED_ATTRIBS + '>' + date.getUTCDate() + '</td><td>&nbsp</td>'; }
                                       date = new Date(date.getTime() + MILLISECONDS_PER_DAY);
                                   }
                                   
                                   calendar += '</tr>';
                               }

                               calendar += '</table><br/>\n';

                               // Go to the first of the (new) month
                               date = new Date(Date.UTC(date.getUTCFullYear(), date.getUTCMonth(), 1, standardHour));
                               
                           } // Until all days covered
                       } // if add calendar

                       // Construct the schedule
                       schedule = '';
                       sourceEntryArray.forEach(function (entry) {
                           schedule += entry.text;
                       });

                       return '\n\n' + calendar + entag('table', schedule, protect('class="schedule"')) + '\n\n';
                     });
    } catch (ignore) {
        // Maybe this wasn't a schedule after all, since we couldn't parse a date. Don't alarm
        // the user, though
    }

    return str;
}


/**
 Term
 :     description, which might be multiple 
       lines and include blanks.

 Next Term

becomes

<dl>
  <dt>Term</dt>
  <dd> description, which might be multiple 
       lines and include blanks.</dd>
  <dt>Next Term</dt>
</dl>

... unless it is very short, in which case it becomes a table.

*/
function replaceDefinitionLists(s, protect) {
    var TERM       = /^.+\n:(?=[ \t])/.source;

    // Definition can contain multiple paragraphs
    var DEFINITION = '(\s*\n|[: \t].+\n)+';

    s = s.rp(new RegExp('(' + TERM + DEFINITION + ')+', 'gm'),
             function (block) {
                 
                 var list = [];

                 // Parse the block
                 var currentEntry = null;
 
                 block.split('\n').forEach(function (line, i) {
                     // What kind of line is this?
                     if (line.trim().length === 0) {
                         if (currentEntry) {
                             // Empty line
                             currentEntry.definition += '\n';
                         }
                     } else if (! /\s/.test(line[0]) && (line[0] !== ':')) {
                         currentEntry = {term: line, definition: ''};
                         list.push(currentEntry);
                     } else {
                         // Add the line to the current definition, stripping any single leading ':'
                         if (line[0] === ':') { line = ' ' + line.ss(1); }
                         currentEntry.definition += line + '\n';
                     }
                 });

                 var longestDefinition = 0;
                 list.forEach(function (entry) {
                     if (/\n\s*\n/.test(entry.definition.trim())) {
                         // This definition contains multiple paragraphs. Force it into long mode
                         longestDefinition = Infinity;
                     } else {
                         // Normal case
                         longestDefinition = max(longestDefinition, unescapeHTMLEntities(removeHTMLTags(entry.definition)).length);
                     }
                 });

                 var result = '';
                 var definitionStyle = option('definitionStyle');
                 if ((definitionStyle === 'short') || ((definitionStyle !== 'long') && (longestDefinition < 160))) {
                     var rowAttribs = protect('valign=top');
                     // This list has short definitions. Format it as a table
                     list.forEach(function (entry) {
                         result += entag('tr',
                                         entag('td', entag('dt', entry.term)) + 
                                         entag('td', entag('dd', entag('p', entry.definition))), 
                                         rowAttribs);
                     });
                     result = entag('table', result);

                 } else {
                     list.forEach(function (entry) {
                         // Leave *two* blanks at the start of a
                         // definition so that subsequent processing
                         // can detect block formatting within it.
                         result += entag('dt', entry.term) + entag('dd', entag('p', entry.definition));
                     });
                 }

                 return entag('dl', result);

             });

    return s;
}


/** Inserts a table of contents in the document and then returns
    [string, table], where the table maps strings to levels. */
function insertTableOfContents(s, protect, exposer) {

    // Gather headers for table of contents (TOC). We
    // accumulate a long and short TOC and then choose which
    // to insert at the end.
    var fullTOC = '<a href="#" class="tocTop">(Top)</a><br/>\n';
    var shortTOC = '';

    // names of parent sections
    var nameStack = [];
    
    // headerCounter[i] is the current counter for header level (i - 1)
    var headerCounter = [0];
    var currentLevel = 0;
    var numAboveLevel1 = 0;

    var table = {};
    s = s.rp(/<h([1-6])>(.*?)<\/h\1>/gi, function (header, level, text) {
        level = parseInt(level)
        text = text.trim();
        
        // If becoming more nested:
        for (var i = currentLevel; i < level; ++i) {
            nameStack[i] = '';
            headerCounter[i] = 0;
        }
        
        // If becoming less nested:
        headerCounter.splice(level, currentLevel - level);
        nameStack.splice(level, currentLevel - level);
        currentLevel = level;

        ++headerCounter[currentLevel - 1];
        
        // Generate a unique name for this element
        var number = headerCounter.join('.');

        // legacy, for when toc links were based on
        // numbers instead of mangled names
        var oldname = 'toc' + number;

        var cleanText = removeHTMLTags(exposer(text)).trim().toLowerCase();
        
        table[cleanText] = number;

        // Remove links from the title itself
        text = text.rp(/<a\s.*>(.*?)<\/a>/g, '$1');

        nameStack[currentLevel - 1] = mangle(cleanText);

        var name = nameStack.join('/');

        // Only insert for the first three levels
        if (level <= 3) {
            // Indent and append (the Array() call generates spaces)
            fullTOC += Array(level).join('&nbsp;&nbsp;') + '<a href="#' + name + '" class="level' + level + '"><span class="tocNumber">' + number + '&nbsp; </span>' + text + '</a><br/>\n';
            
            if (level === 1) {
                shortTOC += ' &middot; <a href="#' + name + '">' + text + '</a>';
            } else {
                ++numAboveLevel1;
            }
        }

        return entag('a', '&nbsp;', protect('class="target" name="' + name + '"')) +
            entag('a', '&nbsp;', protect('class="target" name="' + oldname + '"')) +
            header;
    });

    if (shortTOC.length > 0) {
        // Strip the leading " &middot; "
        shortTOC = shortTOC.ss(10);
    }
    
    var numLevel1 = headerCounter[0];
    var numHeaders = numLevel1 + numAboveLevel1;

    // The location of the first header is indicative of the length of
    // the abstract...as well as where we insert. The first header may be accompanied by
    // <a name> tags, which we want to appear before.
    var firstHeaderLocation = s.regexIndexOf(/((<a\s+\S+>&nbsp;<\/a>)\s*)*?<h\d>/i);
    if (firstHeaderLocation === -1) { firstHeaderLocation = 0; }

    var AFTER_TITLES = '<div class="afterTitles"><\/div>';
    var insertLocation = s.indexOf(AFTER_TITLES);
    if (insertLocation === -1) {
        insertLocation = 0;
    } else {
        insertLocation += AFTER_TITLES.length;
    }

    // Which TOC style should we use?
    var tocStyle = option('tocStyle');

    var TOC = '';
    if ((tocStyle === 'auto') || (tocStyle === '')) {
        if (((numHeaders < 4) && (numLevel1 <= 1)) || (s.length < 2048)) {
            // No TOC; this document is really short
            tocStyle = 'none';
        } else if ((numLevel1 < 7) && (numHeaders / numLevel1 < 2.5)) {
            // We can use the short TOC
            tocStyle = 'short';
        } else if ((firstHeaderLocation === -1) || (firstHeaderLocation / 55 > numHeaders)) {
            // The abstract is long enough to float alongside, and there
            // are not too many levels.        
            // Insert the medium-length TOC floating
            tocStyle = 'medium';
        } else {
            // This is a long table of contents or a short abstract
            // Insert a long toc...right before the first header
            tocStyle = 'long';
        }
    }

    switch (tocStyle) {
    case 'none':
    case '':
        break;

    case 'short':
        TOC = '<div class="shortTOC">' + shortTOC + '</div>';
        break;

    case 'medium':
        TOC = '<div class="mediumTOC"><center><b>' + keyword('Contents') + '</b></center><p>' + fullTOC + '</p></div>';
        break;

    case 'long':
        insertLocation = firstHeaderLocation;
        TOC = '<div class="longTOC"><div class="tocHeader">' + keyword('Contents') + '</div><p>' + fullTOC + '</p></div>';
        break;

    default:
        console.log('markdeepOptions.tocStyle = "' + tocStyle + '" specified in your document is not a legal value');
    }

    s = s.ss(0, insertLocation) + TOC + s.ss(insertLocation);

    return [s, table];
}


function escapeRegExpCharacters(str) {
    return str.rp(/([\.\[\]\(\)\*\+\?\^\$\\\{\}\|])/g, '\\$1');
}


/** Returns true if there are at least two newlines in each of the arguments */
function isolated(preSpaces, postSpaces) {
    if (preSpaces && postSpaces) {
        preSpaces  = preSpaces.match(/\n/g);
        postSpaces = postSpaces.match(/\n/g);
        return preSpaces && (preSpaces.length > 1) && postSpaces && (postSpaces.length > 1);
    } else {
        return false;
    }
}


/**
    Performs Markdeep processing on str, which must be a string or a
    DOM element.  Returns a string that is the HTML to display for the
    body. The result does not include the header: Markdeep stylesheet
    and script tags for including a math library, or the Markdeep
    signature footer.

    Optional argument elementMode defaults to true. This avoids turning a bold first word into a 
    title or introducing a table of contents. Section captions are unaffected by this argument.
    Set elementMode = false if processing a whole document instead of an internal node.

 */
function markdeepToHTML(str, elementMode) {
    // Map names to the number used for end notes, in the order
    // encountered in the text.
    var endNoteTable = {}, endNoteCount = 0;

    // Reference links
    var referenceLinkTable = {};

    // In the private use area
    var PROTECT_CHARACTER = '\ue010';

    // Use base 32 for encoding numbers, which is efficient in terms of 
    // characters but avoids 'x' to avoid the pattern \dx\d, which Markdeep would
    // beautify as a dimension
    var PROTECT_RADIX     = 32;
    var protectedStringArray = [];

    // Gives 1e6 possible sequences in base 32, which should be sufficient
    var PROTECT_DIGITS    = 4;

    // Put the protect character at BOTH ends to avoid having the protected number encoding
    // look like an actual number to further markdown processing
    var PROTECT_REGEXP    = RegExp(PROTECT_CHARACTER + '[0-9a-w]{' + PROTECT_DIGITS + ',' + PROTECT_DIGITS + '}' + PROTECT_CHARACTER, 'g');

    /** Given an arbitrary string, returns an escaped identifier
        string to temporarily replace it with to prevent Markdeep from
        processing the contents. See expose() */
    function protect(s) {
        // Generate the replacement index, converted to an alphanumeric string
        var i = (protectedStringArray.push(s) - 1).toString(PROTECT_RADIX);

        // Ensure fixed length
        while (i.length < PROTECT_DIGITS) {
            i = '0' + i;
        }

        return PROTECT_CHARACTER + i + PROTECT_CHARACTER;
    }

    var exposeRan = false;
    /** Given the escaped identifier string from protect(), returns
        the orginal string. */
    function expose(i) {
        // Strip the escape character and parse, then look up in the
        // dictionary.
        var j = parseInt(i.ss(1, i.length - 1), PROTECT_RADIX);
        exposeRan = true;
        return protectedStringArray[j];
    }

    /** First-class function to pass to String.replace to protect a
        sequence defined by a regular expression. */
    function protector(match, protectee) {
        return protect(protectee);
    }

    function protectorWithPrefix(match, prefix, protectee) {
        return prefix + protect(protectee);
    }

    // SECTION HEADERS
    // This is common code for numbered headers. No-number ATX headers are processed
    // separately
    function makeHeaderFunc(level) {
        return function (match, header) {
            return '\n\n</p>\n<a ' + protect('class="target" name="' + mangle(removeHTMLTags(header.rp(PROTECT_REGEXP, expose))) + '"') + 
                '>&nbsp;</a>' + entag('h' + level, header) + '\n<p>\n\n';
        }
    }

    if (elementMode === undefined) { 
        elementMode = true;
    }
    
    if (str.innerHTML !== undefined) {
        str = str.innerHTML;
    }

    // Prefix a newline so that blocks beginning at the top of the
    // document are processed correctly
    str = '\n\n' + str;

    // Replace pre-formatted script tags that are used to protect
    // less-than signs, e.g., in std::vector<Value>
    str = str.rp(/<script\s+type\s*=\s*['"]preformatted['"]\s*>([\s\S]*?)<\/script>/gi, '$1');

    function replaceDiagrams(str) {
        var result = extractDiagram(str);
        if (result.diagramString) {
            var CAPTION_REGEXP = /^\n*[ \t]*\[[^\n]+\][ \t]*(?=\n)/;
            result.afterString = result.afterString.rp(CAPTION_REGEXP, function (caption) {
                // Strip whitespace and enclosing brackets from the caption
                caption = caption.trim();
                caption = caption.ss(1, caption.length - 1);
                
                result.caption = entag('center', entag('div', caption, protect('class="imagecaption"')));
                return '';
            });

            var diagramSVG = diagramToSVG(result.diagramString, result.alignmentHint);
            var captionAbove = option('captionAbove', 'diagram')

            return result.beforeString +
                (result.caption && captionAbove ? result.caption : '') +
                diagramSVG +
                (result.caption && ! captionAbove ? result.caption : '') + '\n' +
                replaceDiagrams(result.afterString);
        } else {
            return str;
        }
    }

    // CODE FENCES, with styles. Do this before other processing so that their code is
    // protected from further Markdown processing
    var stylizeFence = function (cssClass, symbol) {
        var pattern = new RegExp('\n([ \\t]*)' + symbol + '{3,}([ \\t]*\\S*)([ \\t]+.+)?\n([\\s\\S]+?)\n\\1' + symbol + '{3,}[ \t]*\n([ \\t]*\\[.+(?:\n.+){0,3}\\])?', 'g');
        
        str = str.rp(pattern, function(match, indent, lang, cssSubClass, sourceCode, caption) {
            if (caption) {
                caption = caption.trim();
                caption = entag('center', '<div ' + protect('class="listingcaption ' + cssClass + '"') + '>' + caption.ss(1, caption.length - 1) + '</div>') + '\n';
            }
            // Remove the block's own indentation from each line of sourceCode
            sourceCode = sourceCode.rp(new RegExp('(^|\n)' + indent, 'g'), '$1');

            var captionAbove = option('captionAbove', 'listing')
            var nextSourceCode, nextLang, nextCssSubClass;
            var body = [];

            // Process multiple-listing blocks
            do {
                nextSourceCode = nextLang = nextCssSubClass = undefined;
                sourceCode = sourceCode.rp(new RegExp('\\n([ \\t]*)' + symbol + '{3,}([ \\t]*\\S+)([ \\t]+.+)?\n([\\s\\S]*)'),
                                           function (match, indent, lang, cssSubClass, everythingElse) {
                                               nextLang = lang;
                                               nextCssSubClass = cssSubClass;
                                               nextSourceCode = everythingElse;
                                               return '';
                                           });

                // Highlight and append this block
                lang = lang ? lang.trim() : undefined;
                var result;
                if (lang === 'none') {
                    result = hljs.highlightAuto(sourceCode, []);
                } else if (lang === undefined) {
                    result = hljs.highlightAuto(sourceCode);
                } else {
                    try {
                        result = hljs.highlight(lang, sourceCode, true);
                    } catch (e) {
                        // Some unknown language specified. Force to no formatting.
                        result = hljs.highlightAuto(sourceCode, []);
                    }
                }
                
                var highlighted = result.value;

                // Mark each line as a span to support line numbers
                highlighted = highlighted.rp(/^(.*)$/gm, entag('span', '$1', 'class="line"'));

                if (cssSubClass) {
                    highlighted = entag('div', highlighted, 'class="' + cssSubClass + '"');
                }

                body.push(highlighted);

                // Advance the next nested block
                sourceCode = nextSourceCode;
                lang = nextLang;
                cssSubClass = nextCssSubClass;
            } while (sourceCode);

            // Insert paragraph close/open tags, since browsers force them anyway around pre tags
            // We need the indent in case this is a code block inside a list that is indented.
            return '\n' + indent + '</p>' + (caption && captionAbove ? caption : '') +
                protect(entag('pre', entag('code', body.join('')), 'class="listing ' + cssClass + '"')) +
                (caption && ! captionAbove ? caption : '') + '<p>\n';
        });
    };

    stylizeFence('tilde', '~');
    stylizeFence('backtick', '`');
    
    // Highlight explicit inline code
    str = str.rp(/<code\s+lang\s*=\s*["']?([^"'\)\[\]\n]+)["'?]\s*>(.*)<\/code>/gi, function (match, lang, body) {
        return entag('code', hljs.highlight(lang, body, true).value, 'lang=' + lang);
    });
    
    // Protect raw <CODE> content
    str = str.rp(/(<code\b.*?<\/code>)/gi, protector);

    // Remove XML/HTML COMMENTS
    str = str.rp(/<!-(-+)[^-][\s\S]*?-->/g, '');

    str = replaceDiagrams(str);
    
    // Protect SVG blocks (including the ones we just inserted)
    str = str.rp(/<svg( .*?)?>([\s\S]*?)<\/svg>/gi, function (match, attribs, body) {
        return '<svg' + protect(attribs) + '>' + protect(body) + '</svg>';
    });
    
    // Protect STYLE blocks
    str = str.rp(/<style>([\s\S]*?)<\/style>/gi, function (match, body) {
        return entag('style', protect(body));
    });

    // Protect the very special case of img tags with newlines and
    // breaks in them AND mismatched angle brackets. This happens for
    // Gravizo graphs.
    str = str.rp(/<img\s+src=(["'])[\s\S]*?\1\s*>/gi, function (match, quote) {
        // Strip the "<img " and ">", and then protect the interior:
        return "<img " + protect(match.ss(5, match.length - 1)) + ">";
    });

    // INLINE CODE: Surrounded in (non-escaped!) back ticks on a single line.  Do this before any other
    // processing except for diagrams to protect code blocks from further interference. Don't process back ticks
    // inside of code fences. Allow a single newline, but not wrapping further because that
    // might just pick up quotes used as other punctuation across lines. Explicitly exclude
    // cases where the second quote immediately preceeds a number, e.g., "the old `97"
    var inlineLang = option('inlineCodeLang');
    var inlineCodeRegexp = /(^|[^\\])`(.*?(?:\n.*?)?[^\n\\`])`(?!\d)/g;
    if (inlineLang) {
        // Syntax highlight as well as converting to code. Protect
        // so that the hljs output isn't itself escaped below.
        var filenameRegexp = /^[a-zA-Z]:\\|^\/[a-zA-Z_\.]|^[a-z]{3,5}:\/\//;
        str = str.rp(inlineCodeRegexp, function (match, before, body) {
            if (filenameRegexp.test(body)) {
                // This looks like a filename, don't highlight it
                return before + entag('code', body);
            } else {
                return before + protect(entag('code', hljs.highlight(inlineLang, body, true).value));
            }
        });
    } else {
        str = str.rp(inlineCodeRegexp, '$1' + entag('code', '$2'));
    }

    // Unescape escaped backticks
    str = str.rp(/\\`/g, '`');
    
    // CODE: Escape angle brackets inside code blocks (including the ones we just introduced),
    // and then protect the blocks themselves
    str = str.rp(/(<code(?: .*?)?>)([\s\S]*?)<\/code>/gi, function (match, open, inlineCode) {
        return protect(open + escapeHTMLEntities(inlineCode) + '</code>');
    });
    
    // PRE: Protect pre blocks
    str = str.rp(/(<pre\b[\s\S]*?<\/pre>)/gi, protector);
    
    // Protect raw HTML attributes from processing
    str = str.rp(/(<\w[^ \n<>]*?[ \t]+)(.*?)(?=\/?>)/g, protectorWithPrefix);

    // End of processing literal blocks
    /////////////////////////////////////////////////////////////////////////////

    // Temporarily hide $$ MathJax LaTeX blocks from Markdown processing (this must
    // come before single $ block detection below)
    str = str.rp(/(\$\$[\s\S]+?\$\$)/g, protector);

    // Convert LaTeX $ ... $ to MathJax, but verify that this
    // actually looks like math and not just dollar
    // signs. Don't rp double-dollar signs. Do this only
    // outside of protected blocks.

    // Also allow LaTeX of the form $...$ if the close tag is not US$ or Can$
    // and there are spaces outside of the dollar signs.
    //
    // Test: " $3 or US$2 and 3$, $x$ $y + \n 2x$ or ($z$) $k$. or $2 or $2".match(pattern) = 
    // ["$x$", "$y +  2x$", "$z$", "$k$"];
    str = str.rp(/((?:[^\w\d]))\$(\S(?:[^\$]*?\S(?!US|Can))??)\$(?![\w\d])/g, '$1\\($2\\)');

    //
    // Literally: find a non-dollar sign, non-number followed
    // by a dollar sign and a space.  Then, find any number of
    // characters until the same pattern reversed, allowing
    // one punctuation character before the final space. We're
    // trying to exclude things like Canadian 1$ and US $1
    // triggering math mode.

    str = str.rp(/((?:[^\w\d]))\$([ \t][^\$]+?[ \t])\$(?![\w\d])/g, '$1\\($2\\)');

    // Temporarily hide MathJax LaTeX blocks from Markdown processing
    str = str.rp(/(\\\([\s\S]+?\\\))/g, protector);
    str = str.rp(/(\\begin\{equation\}[\s\S]*?\\end\{equation\})/g, protector);
    str = str.rp(/(\\begin\{eqnarray\}[\s\S]*?\\end\{eqnarray\})/g, protector);
    str = str.rp(/(\\begin\{equation\*\}[\s\S]*?\\end\{equation\*\})/g, protector);

    // HEADERS
    //
    // We consume leading and trailing whitespace to avoid creating an extra paragraph tag
    // around the header itself.

    // Setext-style H1: Text with ======== right under it
    str = str.rp(/(?:^|\s*\n)(.+?)\n[ \t]*={3,}[ \t]*\n/g, makeHeaderFunc(1));
    
    // Setext-style H2: Text with -------- right under it
    str = str.rp(/(?:^|\s*\n)(.+?)\n[ \t]*-{3,}[ \t]*\n/g, makeHeaderFunc(2));

    // ATX-style headers:
    //
    //  # Foo #
    //  # Foo
    //  (# Bar)
    //
    // If note that '#' in the title are only stripped if they appear at the end, in
    // order to allow headers with # in the title.

    for (var i = 6; i > 0; --i) {
        str = str.rp(new RegExp(/^\s*/.source + '#{' + i + ',' + i +'}(?:[ \t])([^\n]+?)#*[ \t]*\n', 'gm'), 
                 makeHeaderFunc(i));

        // No-number headers
        str = str.rp(new RegExp(/^\s*/.source + '\\(#{' + i + ',' + i +'}\\)(?:[ \t])([^\n]+?)\\(?#*\\)?\\n[ \t]*\n', 'gm'), 
                     '\n</p>\n' + entag('div', '$1', protect('class="nonumberh' + i + '"')) + '\n<p>\n\n');
    }

    // HORIZONTAL RULE: * * *, - - -, _ _ _
    str = str.rp(/\n[ \t]*((\*|-|_)[ \t]*){3,}[ \t]*\n/g, '\n<hr/>\n');

    // PAGE BREAK or HORIZONTAL RULE: +++++
    str = str.rp(/\n[ \t]*\+{5,}[ \t]*\n/g, '\n<hr ' + protect('class="pagebreak"') + '/>\n');

    // ADMONITION: !!! (class) (title)\n body
    str = str.rp(/^!!![ \t]*([^\s"'><&\:]*)\:?(.*)\n([ \t]{3,}.*\s*\n)*/gm, function (match, cssClass, title) {
        // Have to extract the body by splitting match because the regex doesn't capture the body correctly in the multi-line case
        match = match.trim();
        return '\n\n' + entag('div', ((title ? entag('div', title, protect('class="admonition-title"')) + '\n' : '') + match.ss(match.indexOf('\n'))).trim(), protect('class="admonition ' + cssClass.toLowerCase().trim() + '"')) + '\n\n';
    });

    // FANCY QUOTE in a blockquote:
    // > " .... "
    // >    -- Foo

    var FANCY_QUOTE = protect('class="fancyquote"');
    str = str.rp(/\n>[ \t]*"(.*(?:\n>.*)*)"[ \t]*(?:\n>[ \t]*)?(\n>[ \t]{2,}\S.*)?\n/g,
                 function (match, quote, author) {
                     return entag('blockquote', 
                                  entag('span',
                                        quote.rp(/\n>/g, '\n'), 
                                        FANCY_QUOTE) + 
                                  (author ? entag('span',
                                                  author.rp(/\n>/g, '\n'),
                                                  protect('class="author"')) : ''),
                                  FANCY_QUOTE);
                });

    // BLOCKQUOTE: > in front of a series of lines
    // Process iteratively to support nested blockquotes
    var foundBlockquote = false;
    do {
        foundBlockquote = false;
        str = str.rp(/(?:\n>.*){2,}/g, function (match) {
            // Strip the leading '>'
            foundBlockquote = true;
            return entag('blockquote', match.rp(/\n>/g, '\n'));
        });
    } while (foundBlockquote);


    // FOOTNOTES/ENDNOTES: [^symbolic name]. Disallow spaces in footnote names to
    // make parsing unambiguous. Consume leading space before the footnote.
    function endNote(match, symbolicNameA) {
        var symbolicName = symbolicNameA.toLowerCase().trim();

        if (! (symbolicName in endNoteTable)) {
            ++endNoteCount;
            endNoteTable[symbolicName] = endNoteCount;
        }

        return '<sup><a ' + protect('href="#endnote-' + symbolicName + '"') + 
            '>' + endNoteTable[symbolicName] + '</a></sup>';
    }    
    str = str.rp(/[ \t]*\[\^([^\]\n\t ]+)\](?!:)/g, endNote);
    str = str.rp(/(\S)[ \t]*\[\^([^\]\n\t ]+)\]/g, function(match, pre, symbolicNameA) { return pre + endNote(match, symbolicNameA); });


    // CITATIONS: [#symbolicname]
    // The bibliography entry:
    str = str.rp(/\n\[#(\S+)\]:[ \t]+((?:[ \t]*\S[^\n]*\n?)*)/g, function (match, symbolicName, entry) {
        symbolicName = symbolicName.trim();
        return '<div ' + protect('class="bib"') + '>[<a ' + protect('class="target" name="citation-' + symbolicName.toLowerCase() + '"') + 
            '>&nbsp;</a><b>' + symbolicName + '</b>] ' + entry + '</div>';
    });
    
    // A reference:
    // (must process AFTER the definitions, since the syntax is a subset)
    str = str.rp(/\[(#[^\)\(\[\]\.#\s]+(?:\s*,\s*#(?:[^\)\(\[\]\.#\s]+))*)\]/g, function (match, symbolicNameList) {
        // Parse the symbolicNameList
        symbolicNameList = symbolicNameList.split(',');
        var s = '[';
        for (var i = 0; i < symbolicNameList.length; ++i) {
            // Strip spaces and # signs
            var name = symbolicNameList[i].rp(/#| /g, '');
            s += entag('a', name, protect('href="#citation-' + name.toLowerCase() + '"'));
            if (i < symbolicNameList.length - 1) { s += ', '; }
        }
        return s + ']';
    });
    

    // TABLES: line with | over line containing only | and -
    // (process before reference links to avoid ambiguity on the captions)
    str = replaceTables(str, protect);

    // REFERENCE-LINK TABLE: [foo]: http://foo.com
    // (must come before reference images and reference links in processing)
    str = str.rp(/^\[([^\^#].*?)\]:(.*?)$/gm, function (match, symbolicName, url) {
        referenceLinkTable[symbolicName.toLowerCase().trim()] = {link: url.trim(), used: false};
        return '';
    });

    // EMAIL ADDRESS: <foo@bar.baz> or foo@bar.baz if it doesn't look like a URL
    str = str.rp(/(?:<|(?!<)\b)(\S+@(\S+\.)+?\S{2,}?)(?:$|>|(?=<)|(?=\s)(?!>))/g, function (match, addr) {
        console.log(match);
        if (/http:|ftp:|https:|svn:|:\/\/|\.html|\(|\)|\]/.test(match)) {
            // This is a hyperlink to a url with an @ sign, not an email address
            return match;
        } else {
            return '<a ' + protect('href="mailto:' + addr + '"') + '>' + addr + '</a>';
        }
    });

    // Common code for formatting images
    var formatImage = function (ignore, url, attribs) {
        attribs = attribs || '';
        var img;
        var hash;

        // Detect videos
        if (/\.(mp4|m4v|avi|mpg|mov|webm)$/i.test(url)) {
            // This is video. Any attributes provided will override the defaults given here
            img = '<video ' + protect('class="markdeep" src="' + url + '"' + attribs + ' width="480px" controls="true"') + '></video>';
        } else if (/\.(mp3|mp2|ogg|wav|m4a|aac|flac)$/i.test(url)) {
            // Audio
            img = '<audio ' + protect('class="markdeep" controls ' + attribs + '><source src="' + url + '"') + '></audio>';
        } else if (hash = url.match(/^https:\/\/(?:www\.)?(?:youtube\.com\/\S*?v=|youtu\.be\/)([\w\d-]+)(&.*)?$/i)) {
            // YouTube video
            img = '<iframe ' + protect('class="markdeep" src="https://www.youtube.com/embed/' + hash[1] + '"' + attribs + ' width="480px" height="300px" frameborder="0" allowfullscreen webkitallowfullscreen mozallowfullscreen') + '></iframe>';
        } else if (hash = url.match(/^https:\/\/(?:www\.)?vimeo.com\/\S*?\/([\w\d-]+)$/i)) {
            // Vimeo video
            img = '<iframe ' + protect('class="markdeep" src="https://player.vimeo.com/video/' + hash[1] + '"' + attribs + ' width="480px" height="300px" frameborder="0" allowfullscreen webkitallowfullscreen mozallowfullscreen') + '></iframe>';
        } else {
            // Image (trailing space is needed in case attribs must be quoted by the
            // browser...without the space, the browser will put the closing slash in the
            // quotes.)

            var classList = 'markdeep';
            // Remove classes from attribs
            attribs = attribs.rp(/class *= *(["'])([^'"]+)\1/, function (match, quote, cls) {
                classList += ' ' + cls;
                return '';
            });
            attribs = attribs.rp(/class *= *([^"' ]+)/, function (match, cls) {
                classList += ' ' + cls;
                return '';
            });
            
            img = '<img ' + protect('class="' + classList + '" src="' + url + '"' + attribs) + ' />';
            img = entag('a', img, protect('href="' + url + '" target="_blank"'));
        }

        return img;
    };

    // Reformat equation links that have brackets: eqn [foo] --> eqn \ref{foo} so that
    // mathjax can process them.
    str = str.rp(/\b(equation|eqn\.|eq\.)\s*\[([^\s\]]+)\]/gi, function (match, eq, label) {
        return eq + ' \\ref{' + label + '}';
    });


    // Reformat figure links that have subfigure labels in parentheses, to avoid them being
    // processed as links
    str = str.rp(/\b(figure|fig\.|table|tbl\.|listing|lst\.)\s*\[([^\s\]]+)\](?=\()/gi, function (match) {
        return match + '<span></span>';
    });


    // Process links before images so that captions can contain links

    // Detect gravizo URLs inside of markdown images and protect them, 
    // which will cause them to be parsed sort-of reasonably. This is
    // a really special case needed to handle the newlines and potential
    // nested parentheses. Use the pattern from http://blog.stevenlevithan.com/archives/regex-recursion
    // (could be extended to multiple nested parens if needed)
    str = str.rp(/\(http:\/\/g.gravizo.com\/(.*g)\?((?:[^\(\)]|\([^\(\)]*\))*)\)/gi, function(match, protocol, url) {
        return "(http://g.gravizo.com/" + protocol + "?" + encodeURIComponent(url) + ")";
    });

    // HYPERLINKS: [text](url attribs)
    str = str.rp(/(^|[^!])\[([^\[\]]+?)\]\(("?)([^<>\s"]+?)\3(\s+[^\)]*?)?\)/g, function (match, pre, text, maybeQuote, url, attribs) {
        attribs = attribs || '';
        return pre + '<a ' + protect('href="' + url + '"' + attribs) + '>' + text + '</a>' + maybeShowLabel(url);
    });

    // EMPTY HYPERLINKS: [](url)
    str = str.rp(/(^|[^!])\[[ \t]*?\]\(("?)([^<>\s"]+?)\2\)/g, function (match, pre, maybeQuote, url) {
        return pre + '<a ' + protect('href="' + url + '"') + '>' + url + '</a>';
    });

    // REFERENCE LINK
    str = str.rp(/(^|[^!])\[([^\[\]]+)\]\[([^\[\]]*)\]/g, function (match, pre, text, symbolicName) {
        // Empty symbolic name is replaced by the label text
        if (! symbolicName.trim()) {
            symbolicName = text;
        }
        
        symbolicName = symbolicName.toLowerCase().trim();
        var t = referenceLinkTable[symbolicName];
        if (! t) {
            console.log("Reference link '" + symbolicName + "' never defined");
            return '?';
        } else {
            t.used = true;
            return pre + '<a ' + protect('href="' + t.link + '"') + '>' + text + '</a>';
        }
    });

    var CAPTION_PROTECT_CHARACTER = '\ue011';
    var protectedCaptionArray = [];
    
    // Temporarily protect image captions (or things that look like
    // them) because the following code is really slow at parsing
    // captions since they have regexps that are complicated to
    // evaluate due to branching.
    //
    // The regexp is really just /.*?\n{0,5}.*/, but that executes substantially more slowly on Chrome.
    str = str.rp(/!\[([^\n\]].*?\n?.*?\n?.*?\n?.*?\n?.*?)\]([\[\(])/g, function (match, caption, bracket) {
        // This is the same as the body of the protect() function, but using the protectedCaptionArray instead
        var i = (protectedCaptionArray.push(caption) - 1).toString(PROTECT_RADIX);
        while (i.length < PROTECT_DIGITS) { i = '0' + i; }
        return '![' + CAPTION_PROTECT_CHARACTER + i + CAPTION_PROTECT_CHARACTER + ']' + bracket;
    });
    
    
    // REFERENCE IMAGE: ![...][ref attribs]
    // Rewrite as a regular image for further processing below.
    str = str.rp(/(!\[.*?\])\[([^<>\[\]\s]+?)([ \t][^\n\[\]]*?)?\]/g, function (match, caption, symbolicName, attribs) {
        symbolicName = symbolicName.toLowerCase().trim();
        var t = referenceLinkTable[symbolicName];
        if (! t) {
            console.log("Reference image '" + symbolicName + "' never defined");
            return '?';
        } else {
            t.used = true;
            var s = caption + '(' + t.link + (t.attribs || '') + ')';
            return s;
        }
    });

    
    // IMAGE GRID: Rewrite rows and grids of images into a grid
    var imageGridAttribs = protect('width="100%"');
    var imageGridRowAttribs = protect('valign="top"');
    // This regex is the pattern for multiple images followed by an optional single image in case the last row is ragged
    // with only one extra
    str = str.rp(/(?:\n(?:[ \t]*!\[.*?\]\(("?)[^<>\s]+?(?:[^\n\)]*?)?\)){2,}[ \t]*)+(?:\n(?:[ \t]*!\[.*?\]\(("?)[^<>\s]+?(?:[^\n\)]*?)?\))[ \t]*)?\n/g, function (match) {
        var table = '';

        // Break into rows:
        match = match.split('\n');

        // Parse each row:
        match.forEach(function(row) {
            row = row.trim();
            if (row) {
                // Parse each image
                table += entag('tr', row.rp(/[ \t]*!\[.*?\]\([^\)\s]+([^\)]*?)?\)/g, function(image, attribs) {
                    //if (! /width|height/i.test(attribs) {
                        // Add a bogus "width" attribute to force the images to be hyperlinked to their
                        // full-resolution versions
                    //}
                    return entag('td', '\n\n'+ image + '\n\n');
                }), imageGridRowAttribs);
            }
        });

        return '\n' + entag('table', table, imageGridAttribs) + '\n';
    });

    // SIMPLE IMAGE: ![](url attribs)
    str = str.rp(/(\s*)!\[\]\(("?)([^"<>\s]+?)\2(\s[^\)]*?)?\)(\s*)/g, function (match, preSpaces, maybeQuote, url, attribs, postSpaces) {
        var img = formatImage(match, url, attribs);

        if (isolated(preSpaces, postSpaces)) {
            // In a block by itself: center
            img = entag('center', img);
        }

        return preSpaces + img + postSpaces;
    });

    // Explicit loop so that the output will be re-processed, preserving spaces between blocks.
    // Note that there is intentionally no global flag on the first regexp since we only want
    // to process the first occurance.
    var loop = true;
    var imageCaptionAbove = option('captionAbove', 'image');
    while (loop) {
        loop = false;

        // CAPTIONED IMAGE: ![caption](url attribs)
        str = str.rp(/(\s*)!\[(.+?)\]\(("?)([^"<>\s]+?)\3(\s[^\)]*?)?\)(\s*)/, function (match, preSpaces, caption, maybeQuote, url, attribs, postSpaces) {
            loop = true;
            var divStyle = '';
            var iso = isolated(preSpaces, postSpaces);

            // Only floating images get their size attributes moved to the whole box
            if (attribs && ! iso) {
                // Move any width *attribute* specification to the box itself
                attribs = attribs.rp(/((?:max-)?width)\s*:\s*[^;'"]*/g, function (attribMatch, attrib) {
                    divStyle = attribMatch + ';';
                    return attrib + ':100%';
                });
                
                // Move any width *style* specification to the box itself
                attribs = attribs.rp(/((?:max-)?width)\s*=\s*('\S+?'|"\S+?")/g, function (attribMatch, attrib, expr) {
                    // Strip the quotes
                    divStyle = attrib + ':' + expr.ss(1, expr.length - 1) + ';';
                    return 'style="' + attrib + ':100%" ';
                });
            }

            var img = formatImage(match, url, attribs);

            if (iso) {
                // In its own block: center
                preSpaces += '<center>';
                postSpaces = '</center>' + postSpaces;
            } else {
                // Embedded: float
                divStyle += 'float:right;margin:4px 0px 0px 25px;'
            }
            var floating = !iso;
            
            caption = entag('center', entag('span', caption + maybeShowLabel(url), protect('class="imagecaption"')));

            // This code used to put floating images in <span> instead of <div>,
            // but it wasn't clear why and this broke centered captions
            return preSpaces + 
                entag('div', (imageCaptionAbove ? caption : '') + img + (! imageCaptionAbove ? caption : ''), protect('class="image" style="' + divStyle + '"')) + 
                postSpaces;
        });
    } // while replacements made

    // Uprotect image captions
    var exposeCaptionRan = false;
    var CAPTION_PROTECT_REGEXP    = RegExp(CAPTION_PROTECT_CHARACTER + '[0-9a-w]{' + PROTECT_DIGITS + ',' + PROTECT_DIGITS + '}' + CAPTION_PROTECT_CHARACTER, 'g');
    /** Given the escaped identifier string from protect(), returns
        the orginal string. */
    function exposeCaption(i) {
        // Strip the escape character and parse, then look up in the
        // dictionary.
        var j = parseInt(i.ss(1, i.length - 1), PROTECT_RADIX);
        exposeCaptionRan = true;
        return protectedCaptionArray[j];
    }
    exposeCaptionRan = true;
    while ((str.indexOf(CAPTION_PROTECT_CHARACTER) + 1) && exposeCaptionRan) {
        exposeCaptionRan = false;
        str = str.rp(CAPTION_PROTECT_REGEXP, exposeCaption);
    }
    
    ////////////////////////////////////////////

    // Process these after links, so that URLs with underscores and tildes are protected.

    // STRONG: Must run before italic, since they use the
    // same symbols. **b** __b__
    str = replaceMatched(str, /\*\*/, 'strong', protect('class="asterisk"'));
    str = replaceMatched(str, /__/, 'strong', protect('class="underscore"'));

    // EM (ITALICS): *i* _i_
    str = replaceMatched(str, /\*/, 'em', protect('class="asterisk"'));
    str = replaceMatched(str, /_/, 'em', protect('class="underscore"'));
    
    // STRIKETHROUGH: ~~text~~
    str = str.rp(/\~\~([^~].*?)\~\~/g, entag('del', '$1'));

    // SMART DOUBLE QUOTES: "a -> localized &ldquo;   z"  -> localized &rdquo;
    // Allow situations such as "foo"==>"bar" and foo:"bar", but not 3' 9"
    if (option('smartQuotes')) {
        str = str.rp(/(^|[ \t->])(")(?=\w)/gm, '$1' + keyword('&ldquo;'));
        str = str.rp(/([A-Za-z\.,:;\?!=<])(")(?=$|\W)/gm, '$1' + keyword('&rdquo;'));
    }
    
    // ARROWS:
    str = str.rp(/(\s|^)<==(\s)/g, '$1\u21D0$2');
    str = str.rp(/(\s|^)->(\s)/g, '$1&rarr;$2');
    // (this requires having removed HTML comments first)
    str = str.rp(/(\s|^)-->(\s)/g, '$1&xrarr;$2');
    str = str.rp(/(\s|^)==>(\s)/g, '$1\u21D2$2');
    str = str.rp(/(\s|^)<-(\s)/g, '$1&larr;$2');
    str = str.rp(/(\s|^)<--(\s)/g, '$1&xlarr;$2');
    str = str.rp(/(\s|^)<==>(\s)/g, '$1\u21D4$2');
    str = str.rp(/(\s|^)<->(\s)/g, '$1\u2194$2');

    // EM DASH: ---
    // (exclude things that look like table delimiters!)
    str = str.rp(/([^-!\:\|])---([^->\:\|])/g, '$1&mdash;$2');

    // other EM DASH: -- (we don't support en dash...it is too short and looks like a minus)
    // (exclude things that look like table delimiters!)
    str = str.rp(/([^-!\:\|])--([^->\:\|])/g, '$1&mdash;$2');

    // NUMBER x NUMBER:
    str = str.rp(/(\d+[ \t]?)x(?=[ \t]?\d+)/g, '$1&times;');

    // MINUS: -4 or 2 - 1
    str = str.rp(/([\s\(\[<\|])-(\d)/g, '$1&minus;$2');
    str = str.rp(/(\d) - (\d)/g, '$1 &minus; $2');

    // EXPONENTS: ^1 ^-1 (no decimal places allowed)
    str = str.rp(/\^([-+]?\d+)\b/g, '<sup>$1</sup>');

    // PAGE BREAK:
    str = str.rp(/(^|\s|\b)\\(pagebreak|newpage)(\b|\s|$)/gi, protect('<div style="page-break-after:always"> </div>\n'))
    
    // SCHEDULE LISTS: date : title followed by indented content
    str = replaceScheduleLists(str, protect);

    // DEFINITION LISTS: Word followed by a colon list
    // Use <dl><dt>term</dt><dd>definition</dd></dl>
    // https://developer.mozilla.org/en-US/docs/Web/HTML/Element/dl
    //
    // Process these before lists so that lists within definition lists
    // work correctly
    str = replaceDefinitionLists(str, protect);

    // LISTS: lines with -, +, *, or number.
    str = replaceLists(str, protect);

    // DEGREE: ##-degree
    str = str.rp(/(\d+?)[ \t-]degree(?:s?)/g, '$1&deg;');

    // PARAGRAPH: Newline, any amount of space, newline...as long as there isn't already
    // a paragraph break there.
    str = str.rp(/(?:<p>)?\n\s*\n+(?!<\/p>)/gi,
                 function(match) { return (/^<p>/i.test(match)) ? match : '\n\n</p><p>\n\n';});

    // Remove empty paragraphs (mostly avoided by the above, but some can still occur)
    str = str.rp(/<p>[\s\n]*<\/p>/gi, '');


    // FOOTNOTES/ENDNOTES
    str = str.rp(/\n\[\^(\S+)\]: ((?:.+?\n?)*)/g, function (match, symbolicName, note) {
        symbolicName = symbolicName.toLowerCase().trim();
        if (symbolicName in endNoteTable) {
            return '\n<div ' + protect('class="endnote"') + '><a ' + 
                protect('class="target" name="endnote-' + symbolicName + '"') + 
                '>&nbsp;</a><sup>' + endNoteTable[symbolicName] + '</sup> ' + note + '</div>';
        } else {
            return "\n";
        }
    });
    

    // SECTION LINKS: XXX section, XXX subsection.
    // Do this by rediscovering the headers and then recursively
    // searching for links to them. Process after other
    // forms of links to avoid ambiguity.
    
    var allHeaders = str.match(/<h([1-6])>(.*?)<\/h\1>/gi);
    if (allHeaders) {
        allHeaders.forEach(function (header) {
            header = removeHTMLTags(header.ss(4, header.length - 5)).trim();
            var link = '<a ' + protect('href="#' + mangle(header) + '"') + '>';

            var sectionExp = '(' + keyword('section') + '|' + keyword('subsection') + '|' + keyword('chapter') + ')';
            var headerExp = '(\\b' + escapeRegExpCharacters(header) + ')';
            
            // Search for links to this section
            str = str.rp(RegExp(headerExp + '\\s+' + sectionExp, 'gi'), link + "$1</a> $2");
            str = str.rp(RegExp(sectionExp + '\\s+' + headerExp, 'gi'), '$1 ' + link + "$2</a>");
        });
    }

    // TABLE, LISTING, and FIGURE LABEL NUMBERING: Figure [symbol]: Table [symbol]: Listing [symbol]: Diagram [symbol]:

    // This data structure maps caption types [by localized name] to a count of how many of
    // that type of object exist.
    var refCounter = {};

    // refTable['type_symbolicName'] = {number: number to link to, used: bool}
    var refTable = {};

    str = str.rp(RegExp(/($|>)\s*/.source + '(' + keyword('figure') + '|' + keyword('table') + '|' + keyword('listing') + '|' + keyword('diagram') + ')' + /\s+\[(.+?)\]:/.source, 'gim'), function (match, prefix, _type, _ref) {
        var type = _type.toLowerCase();
        // Increment the counter
        var count = refCounter[type] = (refCounter[type] | 0) + 1;
        var ref = type + '_' + mangle(_ref.toLowerCase().trim());

        // Store the reference number
        refTable[ref] = {number: count, used: false, source: type + ' [' + _ref + ']'};
        
        return prefix +
               entag('a', '&nbsp;', protect('class="target" name="' + ref + '"')) + entag('b', type[0].toUpperCase() + type.ss(1) + '&nbsp;' + count + ':', protect('style="font-style:normal;"')) +
               maybeShowLabel(_ref);
    });

    // FIGURE, TABLE, DIAGRAM, and LISTING references:
    // (must come after figure/table/listing processing, obviously)
    str = str.rp(RegExp('\\b(fig\\.|tbl\\.|lst\\.|' + keyword('figure') + '|' + keyword('table') + '|' + keyword('listing') + '|' + keyword('diagram') + ')\\s+\\[([^\\s\\]]+)\\]', 'gi'), function (match, _type, _ref) {
        // Fix abbreviations
        var type = _type.toLowerCase();
        switch (type) {
        case 'fig.': type = keyword('figure').toLowerCase(); break;
        case 'tbl.': type = keyword('table').toLowerCase(); break;
        case 'lst.': type = keyword('listing').toLowerCase(); break;
        }

        // Clean up the reference
        var ref = type + '_' + mangle(_ref.toLowerCase().trim());
        var t = refTable[ref];

        if (t) {
            t.used = true;
            return '<a ' + protect('href="#' + ref + '"') + '>' + _type + '&nbsp;' + t.number + maybeShowLabel(_ref) + '</a>';
        } else {
            console.log("Reference to undefined '" + type + " [" + _ref + "]'");
            return _type + ' ?';
        }
    });

    // URL: <http://baz> or http://baz
    // Must be detected after [link]() processing 
    str = str.rp(/(?:<|(?!<)\b)(\w{3,6}:\/\/.+?)(?:$|>|(?=<)|(?=\s|\u00A0)(?!<))/g, function (match, url) {
        var extra = '';
        if (url[url.length - 1] == '.') {
            // Accidentally sucked in a period at the end of a sentence
            url = url.ss(0, url.length - 1);
            extra = '.';
        }
        // svn and perforce URLs are not hyperlinked. All others (http/https/ftp/mailto/tel, etc. are)
        return '<a ' + ((url[0] !== 's' && url[0] !== 'p') ? protect('href="' + url + '" class="url"') : '') + '>' + url + '</a>' + extra;
    });

    if (! elementMode) {
        var TITLE_PATTERN = /^\s*(?:<\/p><p>)?\s*<strong.*?>([^ \t\*].*?[^ \t\*])<\/strong>(?:<\/p>)?[ \t]*\n/.source;
        
        var ALL_SUBTITLES_PATTERN = /([ {4,}\t][ \t]*\S.*\n)*/.source;

        // Detect a bold first line and make it into a title; detect indented lines
        // below it and make them subtitles
        str = str.rp(
            new RegExp(TITLE_PATTERN + ALL_SUBTITLES_PATTERN, 'g'),
            function (match, title) {
                title = title.trim();

                // rp + RegExp won't give us the full list of
                // subtitles, only the last one. So, we have to
                // re-process match.
                var subtitles = match.ss(match.indexOf('\n', match.indexOf('</strong>')));
                subtitles = subtitles ? subtitles.rp(/[ \t]*(\S.*?)\n/g, '<div class="subtitle"> $1 </div>\n') : '';
                
                // Remove all tags from the title when inside the <TITLE> tag, as well
                // as unicode characters that don't render well in tabs and window bars.
                // These regexps look like they are full of spaces but are actually various
                // unicode space characters. http://jkorpela.fi/chars/spaces.html
                title = removeHTMLTags(title).replace(/[     ]/g, '').replace(/[         　]/g, ' ');
                
                return entag('title', title) + maybeShowLabel(window.location.href, 'center') +
                    '<div class="title"> ' + title + 
                    ' </div>\n' + subtitles + '<div class="afterTitles"></div>\n';
            });
    } // if ! noTitles

    // Remove any bogus leading close-paragraph tag inserted by our extra newlines
    str = str.rp(/^\s*<\/p>/, '');


    // If not in element mode and not an INSERT child, maybe add a TOC
    if (! elementMode) {
        var temp = insertTableOfContents(str, protect, function (text) {return text.rp(PROTECT_REGEXP, expose)});
        str = temp[0];
        var toc = temp[1];
        // SECTION LINKS: Replace sec. [X], section [X], subsection [X]
        str = str.rp(RegExp('\\b(' + keyword('sec') + '\\.|' + keyword('section') + '|' + keyword('subsection') + '|' + keyword('chapter') + ')\\s\\[(.+?)\\]', 'gi'), 
                    function (match, prefix, ref) {
                        var link = toc[ref.toLowerCase().trim()];
                        if (link) {
                            return prefix + '  <a ' + protect('href="#toc' + link + '"') + '>' + link + '</a>';  
                        } else {
                            return prefix + ' ?';
                        }
                    });
    }

    // Expose all protected values. We may need to do this
    // recursively, because pre and code blocks can be nested.
    var maxIterations = 50;

    exposeRan = true;
    while ((str.indexOf(PROTECT_CHARACTER) + 1) && exposeRan && (maxIterations > 0)) {
        exposeRan = false;
        str = str.rp(PROTECT_REGEXP, expose);
        --maxIterations;
    }
    
    if (maxIterations <= 0) { console.log('WARNING: Ran out of iterations while expanding protected substrings'); }

    // Warn about unused references
    Object.keys(referenceLinkTable).forEach(function (key) {
        if (! referenceLinkTable[key].used) {
            console.log("Reference link '[" + key + "]' is defined but never used");
        }
    });

    Object.keys(refTable).forEach(function (key) {
        if (! refTable[key].used) {
            console.log("'" + refTable[key].source + "' is never referenced");
        }
    });

    if (option('linkAPIDefinitions')) {
        // API DEFINITION LINKS
        
        var apiDefined = {};

        // Find link targets for APIs, which look like:
        // '<dt><code...>variablename' followed by (, [, or <
        //
        // If there is syntax highlighting because we're documenting
        // keywords for the language supported by HLJS, then there may
        // be an extra span around the variable name.
        str = str.rp(/<dt><code(\b[^<>\n]*)>(<span class="[a-zA-Z\-_0-9]+">)?([A-Za-z_][A-Za-z_\.0-9:\->]*)(<\/span>)?([\(\[<])/g, function (match, prefix, syntaxHighlight, name, syntaxHighlightEnd, next) {
            var linkName = name + (next === '<' ? '' : next === '(' ? '-fcn' : next === '[' ? '-array' : next);
            apiDefined[linkName] = true;
            // The 'ignore' added to the code tag below is to
            // prevent the link finding code from finding this (since
            // we don't have lookbehinds in JavaScript to recognize
            // the <dt>)
            return '<dt><a name="apiDefinition-' + linkName + '"></a><code ignore ' + prefix + '>' + (syntaxHighlight || '') + name + (syntaxHighlightEnd || '') + next;
        });

        // Hide links that are also inside of a <h#>...</h#>, where we don't want them
        // modified by API links. Assume that these are on a single line. The space in
        // the close tag prevents the next regexp from matching.
        str = str.rp(/<h([1-9])>(.*<code\b[^<>\n]*>.*)<\/code>(.*<\/h\1>)/g, '<h$1>$2</code >$3');

        // Now find potential links, which look like:
        // '<code...>variablename</code>' and may contain () or [] after the variablename
        // They may also have an extra syntax-highlighting span
        str = str.rp(/<code(?! ignore)\b[^<>\n]*>(<span class="[a-zA-Z\-_0-9]+">)?([A-Za-z_][A-Za-z_\.0-9:\->]*)(<\/span>)?(\(\)|\[\])?<\/code>/g, function (match, syntaxHighlight, name, syntaxHighlightEnd, next) {
            var linkName = name + (next ? (next[0] === '(' ? '-fcn' : next[0] === '[' ? '-array' : next[0]) : '');
            return (apiDefined[linkName] === true) ? entag('a', match, 'href="#apiDefinition-' + linkName + '"') : match;
        });
    }
           
    return '<span class="md">' + entag('p', str) + '</span>';
}

 
/** Workaround for IE11 */
function strToArray(s) {
    if (Array.from) {
        return Array.from(s);
    } else {
        var a = [];
        for (var i = 0; i < s.length; ++i) {
            a[i] = s[i];
        }
        return a;
    }
}

/**
   Adds whitespace at the end of each line of str, so that all lines have equal length in
   unicode characters (which is not the same as JavaScript characters when high-index/escape
   characters are present).
*/
function equalizeLineLengths(str) {
    var lineArray = str.split('\n');

    if ((lineArray.length > 0) && (lineArray[lineArray.length - 1] === '')) {
        // Remove the empty last line generated by split on a trailing newline
        lineArray.pop();
    }
        
    var longest = 0;
    lineArray.forEach(function(line) {
        longest = max(longest, strToArray(line).length);
    });

    // Worst case spaces needed for equalizing lengths
    // http://stackoverflow.com/questions/1877475/repeat-character-n-times
    var spaces = Array(longest + 1).join(' ');

    var result = '';
    lineArray.forEach(function(line) {
        // Append the needed number of spaces onto each line, and
        // reconstruct the output with newlines
        result += line + spaces.ss(strToArray(line).length) + '\n';
    });

    return result;
}

/** Finds the longest common whitespace prefix of all non-empty lines
    and then removes it */
function removeLeadingSpace(str) {
    var lineArray = str.split('\n');

    var minimum = Infinity;
    lineArray.forEach(function (line) {
        if (line.trim() !== '') {
            // This is a non-empty line
            var spaceArray = line.match(/^([ \t]*)/);
            if (spaceArray) {
                minimum = min(minimum, spaceArray[0].length);
            }
        }
    });

    if (minimum === 0) {
        // No leading space
        return str;
    }

    var result = '';
    lineArray.forEach(function(line) {
        // Strip the common spaces
        result += line.ss(minimum) + '\n';
    });

    return result;
}

/** Returns true if this character is a "letter" under the ASCII definition */
function isASCIILetter(c) {
    var code = c.charCodeAt(0);
    return ((code >= 65) && (code <= 90)) || ((code >= 97) && (code <= 122));
}

/** Converts diagramString, which is a Markdeep diagram without the surrounding asterisks, to
    SVG (HTML). Lines may have ragged lengths.

    alignmentHint is the float alignment desired for the SVG tag,
    which can be 'floatleft', 'floatright', or ''
 */
function diagramToSVG(diagramString, alignmentHint) {
    // Clean up diagramString if line endings are ragged
    diagramString = equalizeLineLengths(diagramString);

    // Temporarily replace 'o' that is surrounded by other text
    // with another character to avoid processing it as a point 
    // decoration. This will be replaced in the final svg and is
    // faster than checking each neighborhood each time.
    var HIDE_O = '\ue004';
    diagramString = diagramString.rp(/([a-zA-Z]{2})o/g, '$1' + HIDE_O);
    diagramString = diagramString.rp(/o([a-zA-Z]{2})/g, HIDE_O + '$1');
    diagramString = diagramString.rp(/([a-zA-Z\ue004])o([a-zA-Z\ue004])/g, '$1' + HIDE_O + '$2');

    /** Pixels per character */
    var SCALE   = 8;

    /** Multiply Y coordinates by this when generating the final SVG
        result to account for the aspect ratio of text files. This
        MUST be 2 */
    var ASPECT = 2;

    var DIAGONAL_ANGLE = Math.atan(1.0 / ASPECT) * 180 / Math.PI;

    var EPSILON = 1e-6;

    // The order of the following is based on rotation angles
    // and is used for ArrowSet.toSVG
    var ARROW_HEAD_CHARACTERS            = '>v<^';
    var POINT_CHARACTERS                 = 'o*◌○◍●';
    var JUMP_CHARACTERS                  = '()';
    var UNDIRECTED_VERTEX_CHARACTERS     = "+";
    var VERTEX_CHARACTERS                = UNDIRECTED_VERTEX_CHARACTERS + ".'";

    // GRAY[i] is the Unicode block character for (i+1)/4 level gray
    var GRAY_CHARACTERS = '\u2591\u2592\u2593\u2588';

    // TRI[i] is a right-triangle rotated by 90*i
    var TRI_CHARACTERS  = '\u25E2\u25E3\u25E4\u25E5';

    var DECORATION_CHARACTERS            = ARROW_HEAD_CHARACTERS + POINT_CHARACTERS + JUMP_CHARACTERS + GRAY_CHARACTERS + TRI_CHARACTERS;

    function isUndirectedVertex(c) { return UNDIRECTED_VERTEX_CHARACTERS.indexOf(c) + 1; }
    function isVertex(c)           { return VERTEX_CHARACTERS.indexOf(c) !== -1; }
    function isTopVertex(c)        { return isUndirectedVertex(c) || (c === '.'); }
    function isBottomVertex(c)     { return isUndirectedVertex(c) || (c === "'"); }
    function isVertexOrLeftDecoration(c){ return isVertex(c) || (c === '<') || isPoint(c); }
    function isVertexOrRightDecoration(c){return isVertex(c) || (c === '>') || isPoint(c); }
    function isArrowHead(c)        { return ARROW_HEAD_CHARACTERS.indexOf(c) + 1; }
    function isGray(c)             { return GRAY_CHARACTERS.indexOf(c) + 1; }
    function isTri(c)              { return TRI_CHARACTERS.indexOf(c) + 1; }

    // "D" = Diagonal slash (/), "B" = diagonal Backslash (\)
    // Characters that may appear anywhere on a solid line
    function isSolidHLine(c)       { return (c === '-') || isUndirectedVertex(c) || isJump(c); }
    function isSolidVLineOrJumpOrPoint(c) { return isSolidVLine(c) || isJump(c) || isPoint(c); }
    function isSolidVLine(c)       { return (c === '|') || isUndirectedVertex(c); }
    function isSolidDLine(c)       { return (c === '/') || isUndirectedVertex(c) }
    function isSolidBLine(c)       { return (c === '\\') || isUndirectedVertex(c); }
    function isJump(c)             { return JUMP_CHARACTERS.indexOf(c) + 1; }
    function isPoint(c)            { return POINT_CHARACTERS.indexOf(c) + 1; }
    function isDecoration(c)       { return DECORATION_CHARACTERS.indexOf(c) + 1; }
    function isEmpty(c)            { return c === ' '; }
   
    ///////////////////////////////////////////////////////////////////////////////
    // Math library

    /** Invoke as new Vec2(v) to clone or new Vec2(x, y) to create from coordinates.
        Can also invoke without new for brevity. */
    function Vec2(x, y) {
        // Detect when being run without new
        if (! (this instanceof Vec2)) { return new Vec2(x, y); }

        if (y === undefined) {
            if (x === undefined) { x = y = 0; } 
            else if (x instanceof Vec2) { y = x.y; x = x.x; }
            else { console.error("Vec2 requires one Vec2 or (x, y) as an argument"); }
        }
        this.x = x;
        this.y = y;
        Object.seal(this);
    }

    /** Returns an SVG representation */
    Vec2.prototype.toString = Vec2.prototype.toSVG = 
        function () { return '' + (this.x * SCALE) + ',' + (this.y * SCALE * ASPECT) + ' '; };

    /** Converts a "rectangular" string defined by newlines into 2D
        array of characters. Grids are immutable. */
    function makeGrid(str) {
        /** Returns ' ' for out of bounds values */
        var grid = function(x, y) {
            if (y === undefined) {
                if (x instanceof Vec2) { y = x.y; x = x.x; }
                else { console.error('grid requires either a Vec2 or (x, y)'); }
            }
            
            return ((x >= 0) && (x < grid.width) && (y >= 0) && (y < grid.height)) ?
                str[y * (grid.width + 1) + x] : ' ';
        };

        // Elements are true when consumed
        grid._used   = [];

        grid.height  = str.split('\n').length;
        if (str[str.length - 1] === '\n') { --grid.height; }

        // Convert the string to an array to better handle greater-than 16-bit unicode
        // characters, which JavaScript does not process correctly with indices. Do this after
        // the above string processing.
        str = strToArray(str);
        grid.width = str.indexOf('\n');

        /** Mark this location. Takes a Vec2 or (x, y) */
        grid.setUsed = function (x, y) {
            if (y === undefined) {
                if (x instanceof Vec2) { y = x.y; x = x.x; }
                else { console.error('grid requires either a Vec2 or (x, y)'); }
            }
            if ((x >= 0) && (x < grid.width) && (y >= 0) && (y < grid.height)) {
                // Match the source string indexing
                grid._used[y * (grid.width + 1) + x] = true;
            }
        };
        
        grid.isUsed = function (x, y) {
            if (y === undefined) {
                if (x instanceof Vec2) { y = x.y; x = x.x; }
                else { console.error('grid requires either a Vec2 or (x, y)'); }
            }
            return (this._used[y * (this.width + 1) + x] === true);
        };
        
        /** Returns true if there is a solid vertical line passing through (x, y) */
        grid.isSolidVLineAt = function (x, y) {
            if (y === undefined) { y = x.x; x = x.x; }
            
            var up = grid(x, y - 1);
            var c  = grid(x, y);
            var dn = grid(x, y + 1);
            
            var uprt = grid(x + 1, y - 1);
            var uplt = grid(x - 1, y - 1);
            
            if (isSolidVLine(c)) {
                // Looks like a vertical line...does it continue?
                return (isTopVertex(up)    || (up === '^') || isSolidVLine(up) || isJump(up) ||
                        isBottomVertex(dn) || (dn === 'v') || isSolidVLine(dn) || isJump(dn) ||
                        isPoint(up) || isPoint(dn) || (grid(x, y - 1) === '_') || (uplt === '_') ||
                        (uprt === '_') ||
                        
                        // Special case of 1-high vertical on two curved corners 
                        ((isTopVertex(uplt) || isTopVertex(uprt)) &&
                         (isBottomVertex(grid(x - 1, y + 1)) || isBottomVertex(grid(x + 1, y + 1)))));
                
            } else if (isTopVertex(c) || (c === '^')) {
                // May be the top of a vertical line
                return isSolidVLine(dn) || (isJump(dn) && (c !== '.'));
            } else if (isBottomVertex(c) || (c === 'v')) {
                return isSolidVLine(up) || (isJump(up) && (c !== "'"));
            } else if (isPoint(c)) {
                return isSolidVLine(up) || isSolidVLine(dn);
            } 
            
            return false;
        };
    
    
        /** Returns true if there is a solid middle (---) horizontal line
            passing through (x, y). Ignores underscores. */
        grid.isSolidHLineAt = function (x, y) {
            if (y === undefined) { y = x.x; x = x.x; }
            
            var ltlt = grid(x - 2, y);
            var lt   = grid(x - 1, y);
            var c    = grid(x + 0, y);
            var rt   = grid(x + 1, y);
            var rtrt = grid(x + 2, y);
            
            if (isSolidHLine(c) || (isSolidHLine(lt) && isJump(c))) {
                // Looks like a horizontal line...does it continue? We need three in a row.
                if (isSolidHLine(lt)) {
                    return isSolidHLine(rt) || isVertexOrRightDecoration(rt) || 
                        isSolidHLine(ltlt) || isVertexOrLeftDecoration(ltlt);
                } else if (isVertexOrLeftDecoration(lt)) {
                    return isSolidHLine(rt);
                } else {
                    return isSolidHLine(rt) && (isSolidHLine(rtrt) || isVertexOrRightDecoration(rtrt));
                }

            } else if (c === '<') {
                return isSolidHLine(rt) && isSolidHLine(rtrt)
                
            } else if (c === '>') {
                return isSolidHLine(lt) && isSolidHLine(ltlt);
                
            } else if (isVertex(c)) {
                return ((isSolidHLine(lt) && isSolidHLine(ltlt)) || 
                        (isSolidHLine(rt) && isSolidHLine(rtrt)));
            }
            
            return false;
        };
        
        
        /** Returns true if there is a solid backslash line passing through (x, y) */
        grid.isSolidBLineAt = function (x, y) {
            if (y === undefined) { y = x.x; x = x.x; }
            var c = grid(x, y);
            var lt = grid(x - 1, y - 1);
            var rt = grid(x + 1, y + 1);
            
            if (c === '\\') {
                // Looks like a diagonal line...does it continue? We need two in a row.
                return (isSolidBLine(rt) || isBottomVertex(rt) || isPoint(rt) || (rt === 'v') ||
                        isSolidBLine(lt) || isTopVertex(lt) || isPoint(lt) || (lt === '^') ||
                        (grid(x, y - 1) === '/') || (grid(x, y + 1) === '/') || (rt === '_') || (lt === '_')); 
            } else if (c === '.') {
                return (rt === '\\');
            } else if (c === "'") {
                return (lt === '\\');
            } else if (c === '^') {
                return rt === '\\';
            } else if (c === 'v') {
                return lt === '\\';
            } else if (isVertex(c) || isPoint(c) || (c === '|')) {
                return isSolidBLine(lt) || isSolidBLine(rt);
            }
        };
        

        /** Returns true if there is a solid diagonal line passing through (x, y) */
        grid.isSolidDLineAt = function (x, y) {
            if (y === undefined) { y = x.x; x = x.x; }
            
            var c = grid(x, y);
            var lt = grid(x - 1, y + 1);
            var rt = grid(x + 1, y - 1);
            
            if (c === '/' && ((grid(x, y - 1) === '\\') || (grid(x, y + 1) === '\\'))) {
                // Special case of tiny hexagon corner
                return true;
            } else if (isSolidDLine(c)) {
                // Looks like a diagonal line...does it continue? We need two in a row.
                return (isSolidDLine(rt) || isTopVertex(rt) || isPoint(rt) || (rt === '^') || (rt === '_') ||
                        isSolidDLine(lt) || isBottomVertex(lt) || isPoint(lt) || (lt === 'v') || (lt === '_')); 
            } else if (c === '.') {
                return (lt === '/');
            } else if (c === "'") {
                return (rt === '/');
            } else if (c === '^') {
                return lt === '/';
            } else if (c === 'v') {
                return rt === '/';
            } else if (isVertex(c) || isPoint(c) || (c === '|')) {
                return isSolidDLine(lt) || isSolidDLine(rt);
            }
            return false;
        };
        
        grid.toString = function () { return str; };
        
        return Object.freeze(grid);
    }
    
    
    /** A 1D curve. If C is specified, the result is a bezier with
        that as the tangent control point */
    function Path(A, B, C, D, dashed) {
        if (! ((A instanceof Vec2) && (B instanceof Vec2))) {
            console.error('Path constructor requires at least two Vec2s');
        }
        this.A = A;
        this.B = B;
        if (C) {
            this.C = C;
            if (D) {
                this.D = D;
            } else {
                this.D = C;
            }
        }

        this.dashed = dashed || false;

        Object.freeze(this);
    }

    var _ = Path.prototype;
    _.isVertical = function () {
        return this.B.x === this.A.x;
    };

    _.isHorizontal = function () {
        return this.B.y === this.A.y;
    };

    /** Diagonal lines look like: / See also backDiagonal */
    _.isDiagonal = function () {
        var dx = this.B.x - this.A.x;
        var dy = this.B.y - this.A.y;
        return (abs(dy + dx) < EPSILON);
    };

    _.isBackDiagonal = function () {
        var dx = this.B.x - this.A.x;
        var dy = this.B.y - this.A.y;
        return (abs(dy - dx) < EPSILON);
    };

    _.isCurved = function () {
        return this.C !== undefined;
    };

    /** Does this path have any end at (x, y) */
    _.endsAt = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return ((this.A.x === x) && (this.A.y === y)) ||
            ((this.B.x === x) && (this.B.y === y));
    };

    /** Does this path have an up end at (x, y) */
    _.upEndsAt = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return this.isVertical() && (this.A.x === x) && (min(this.A.y, this.B.y) === y);
    };

    /** Does this path have an up end at (x, y) */
    _.diagonalUpEndsAt = function (x, y) {
        if (! this.isDiagonal()) { return false; }
        if (y === undefined) { y = x.y; x = x.x; }
        if (this.A.y < this.B.y) {
            return (this.A.x === x) && (this.A.y === y);
        } else {
            return (this.B.x === x) && (this.B.y === y);
        }
    };

    /** Does this path have a down end at (x, y) */
    _.diagonalDownEndsAt = function (x, y) {
        if (! this.isDiagonal()) { return false; }
        if (y === undefined) { y = x.y; x = x.x; }
        if (this.B.y < this.A.y) {
            return (this.A.x === x) && (this.A.y === y);
        } else {
            return (this.B.x === x) && (this.B.y === y);
        }
    };

    /** Does this path have an up end at (x, y) */
    _.backDiagonalUpEndsAt = function (x, y) {
        if (! this.isBackDiagonal()) { return false; }
        if (y === undefined) { y = x.y; x = x.x; }
        if (this.A.y < this.B.y) {
            return (this.A.x === x) && (this.A.y === y);
        } else {
            return (this.B.x === x) && (this.B.y === y);
        }
    };

    /** Does this path have a down end at (x, y) */
    _.backDiagonalDownEndsAt = function (x, y) {
        if (! this.isBackDiagonal()) { return false; }
        if (y === undefined) { y = x.y; x = x.x; }
        if (this.B.y < this.A.y) {
            return (this.A.x === x) && (this.A.y === y);
        } else {
            return (this.B.x === x) && (this.B.y === y);
        }
    };

    /** Does this path have a down end at (x, y) */
    _.downEndsAt = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return this.isVertical() && (this.A.x === x) && (max(this.A.y, this.B.y) === y);
    };

    /** Does this path have a left end at (x, y) */
    _.leftEndsAt = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return this.isHorizontal() && (this.A.y === y) && (min(this.A.x, this.B.x) === x);
    };

    /** Does this path have a right end at (x, y) */
    _.rightEndsAt = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return this.isHorizontal() && (this.A.y === y) && (max(this.A.x, this.B.x) === x);
    };

    _.verticalPassesThrough = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return this.isVertical() && 
            (this.A.x === x) && 
            (min(this.A.y, this.B.y) <= y) &&
            (max(this.A.y, this.B.y) >= y);
    }

    _.horizontalPassesThrough = function (x, y) {
        if (y === undefined) { y = x.y; x = x.x; }
        return this.isHorizontal() && 
            (this.A.y === y) && 
            (min(this.A.x, this.B.x) <= x) &&
            (max(this.A.x, this.B.x) >= x);
    }
    
    /** Returns a string suitable for inclusion in an SVG tag */
    _.toSVG = function () {
        var svg = '<path d="M ' + this.A;

        if (this.isCurved()) {
            svg += 'C ' + this.C + this.D + this.B;
        } else {
            svg += 'L ' + this.B;
        }
        svg += '" style="fill:none;"';
        if (this.dashed) {
            svg += ' stroke-dasharray="3,6"';
        }
        svg += '/>';
        return svg;
    };


    /** A group of 1D curves. This was designed so that all of the
        methods can later be implemented in O(1) time, but it
        currently uses O(n) implementations for source code
        simplicity. */
    function PathSet() {
        this._pathArray = [];
    }

    var PS = PathSet.prototype;
    PS.insert = function (path) {
        this._pathArray.push(path);
    };

    /** Returns a new method that returns true if method(x, y) 
        returns true on any element of _pathAray */
    function makeFilterAny(method) {
        return function(x, y) {
            for (var i = 0; i < this._pathArray.length; ++i) {
                if (method.call(this._pathArray[i], x, y)) { return true; }
            }
            // Fall through: return undefined == false
        }
    }

    // True if an up line ends at these coordinates. Recall that the
    // variable _ is bound to the Path prototype still.
    PS.upEndsAt                = makeFilterAny(_.upEndsAt);
    PS.diagonalUpEndsAt        = makeFilterAny(_.diagonalUpEndsAt);
    PS.backDiagonalUpEndsAt    = makeFilterAny(_.backDiagonalUpEndsAt);
    PS.diagonalDownEndsAt      = makeFilterAny(_.diagonalDownEndsAt);
    PS.backDiagonalDownEndsAt  = makeFilterAny(_.backDiagonalDownEndsAt);
    PS.downEndsAt              = makeFilterAny(_.downEndsAt);
    PS.leftEndsAt              = makeFilterAny(_.leftEndsAt);
    PS.rightEndsAt             = makeFilterAny(_.rightEndsAt);
    PS.endsAt                  = makeFilterAny(_.endsAt);
    PS.verticalPassesThrough   = makeFilterAny(_.verticalPassesThrough);
    PS.horizontalPassesThrough = makeFilterAny(_.horizontalPassesThrough);

    /** Returns an SVG string */
    PS.toSVG = function () {
        var svg = '';
        for (var i = 0; i < this._pathArray.length; ++i) {
            svg += this._pathArray[i].toSVG() + '\n';
        }
        return svg;
    };


    function DecorationSet() {
        this._decorationArray = [];
    }

    var DS = DecorationSet.prototype;

    /** insert(x, y, type, <angle>)  
        insert(vec, type, <angle>)

        angle is the angle in degrees to rotate the result */
    DS.insert = function(x, y, type, angle) {
        if (type === undefined) { type = y; y = x.y; x = x.x; }

        if (! isDecoration(type)) {
            console.error('Illegal decoration character: ' + type); 
        }
        var d = {C: Vec2(x, y), type: type, angle:angle || 0};

        // Put arrows at the front and points at the back so that
        // arrows always draw under points

        if (isPoint(type)) {
            this._decorationArray.push(d);
        } else {
            this._decorationArray.unshift(d);
        }
    };


    DS.toSVG = function () {
        var svg = '';
        for (var i = 0; i < this._decorationArray.length; ++i) {
            var decoration = this._decorationArray[i];
            var C = decoration.C;
            
            if (isJump(decoration.type)) {
                // Slide jumps
                var dx = (decoration.type === ')') ? +0.75 : -0.75;
                var up  = Vec2(C.x, C.y - 0.5);
                var dn  = Vec2(C.x, C.y + 0.5);
                var cup = Vec2(C.x + dx, C.y - 0.5);
                var cdn = Vec2(C.x + dx, C.y + 0.5);

                svg += '<path d="M ' + dn + ' C ' + cdn + cup + up + '" style="fill:none;"/>';

            } else if (isPoint(decoration.type)) {
                var cls = {'*':'closed', 'o':'open', '◌':'dotted', '○':'open', '◍':'shaded', '●':'closed'}[decoration.type];
                svg += '<circle cx="' + (C.x * SCALE) + '" cy="' + (C.y * SCALE * ASPECT) +
                       '" r="' + (SCALE - STROKE_WIDTH) + '" class="' + cls + 'dot"/>';
            } else if (isGray(decoration.type)) {
                
                var shade = Math.round((3 - GRAY_CHARACTERS.indexOf(decoration.type)) * 63.75);
                svg += '<rect x="' + ((C.x - 0.5) * SCALE) + '" y="' + ((C.y - 0.5) * SCALE * ASPECT) + '" width="' + SCALE + '" height="' + (SCALE * ASPECT) + '" stroke=none fill="rgb(' + shade + ',' + shade + ',' + shade +')"/>';

            } else if (isTri(decoration.type)) {
                // 30-60-90 triangle
                var index = TRI_CHARACTERS.indexOf(decoration.type);
                var xs  = 0.5 - (index & 1);
                var ys  = 0.5 - (index >> 1);
                xs *= sign(ys);
                var tip = Vec2(C.x + xs, C.y - ys);
                var up  = Vec2(C.x + xs, C.y + ys);
                var dn  = Vec2(C.x - xs, C.y + ys);
                svg += '<polygon points="' + tip + up + dn + '" style="stroke:none"/>\n';
            } else { // Arrow head
                var tip = Vec2(C.x + 1, C.y);
                var up =  Vec2(C.x - 0.5, C.y - 0.35);
                var dn =  Vec2(C.x - 0.5, C.y + 0.35);
                svg += '<polygon points="' + tip + up + dn + 
                    '"  style="stroke:none" transform="rotate(' + decoration.angle + ',' + C + ')"/>\n';
            }
        }
        return svg;
    };

    ////////////////////////////////////////////////////////////////////////////

    function findPaths(grid, pathSet) {
        // Does the line from A to B contain at least one c?
        function lineContains(A, B, c) {
            var dx = sign(B.x - A.x);
            var dy = sign(B.y - A.y);
            var x, y;

            for (x = A.x, y = A.y; (x !== B.x) || (y !== B.y); x += dx, y += dy) {
                if (grid(x, y) === c) { return true; }
            }

            // Last point
            return (grid(x, y) === c);
        }

        // Find all solid vertical lines. Iterate horizontally
        // so that we never hit the same line twice
        for (var x = 0; x < grid.width; ++x) {
            for (var y = 0; y < grid.height; ++y) {
                if (grid.isSolidVLineAt(x, y)) {
                    // This character begins a vertical line...now, find the end
                    var A = Vec2(x, y);
                    do  { grid.setUsed(x, y); ++y; } while (grid.isSolidVLineAt(x, y));
                    var B = Vec2(x, y - 1);
                    
                    var up = grid(A);
                    var upup = grid(A.x, A.y - 1);

                    if (! isVertex(up) && ((upup === '-') || (upup === '_') ||
                                           (upup === '┳') ||
                                           (grid(A.x - 1, A.y - 1) === '_') ||
                                           (grid(A.x + 1, A.y - 1) === '_') || 
                                           isBottomVertex(upup)) || isJump(upup)) {
                        // Stretch up to almost reach the line above (if there is a decoration,
                        // it will finish the gap)
                        A.y -= 0.5;
                    }

                    var dn = grid(B);
                    var dndn = grid(B.x, B.y + 1);
                    if (! isVertex(dn) && ((dndn === '-') || (dndn === '┻') || isTopVertex(dndn)) || isJump(dndn) ||
                        (grid(B.x - 1, B.y) === '_') || (grid(B.x + 1, B.y) === '_')) {
                        // Stretch down to almost reach the line below
                        B.y += 0.5;
                    }

                    // Don't insert degenerate lines
                    if ((A.x !== B.x) || (A.y !== B.y)) {
                        pathSet.insert(new Path(A, B));
                    }

                    // Continue the search from the end value y+1
                } 

                // Some very special patterns for the short lines needed on
                // circuit diagrams. Only invoke these if not also on a curve
                //      _  _    
                //    -'    '-   -'
                else if ((grid(x, y) === "'") &&
                    (((grid(x - 1, y) === '-') && (grid(x + 1, y - 1) === '_') &&
                     ! isSolidVLineOrJumpOrPoint(grid(x - 1, y - 1))) ||
                     ((grid(x - 1, y - 1) === '_') && (grid(x + 1, y) === '-') &&
                     ! isSolidVLineOrJumpOrPoint(grid(x + 1, y - 1))))) {
                    pathSet.insert(new Path(Vec2(x, y - 0.5), Vec2(x, y)));
                }

                //    _.-  -._  
                else if ((grid(x, y) === '.') &&
                         (((grid(x - 1, y) === '_') && (grid(x + 1, y) === '-') && 
                           ! isSolidVLineOrJumpOrPoint(grid(x + 1, y + 1))) ||
                          ((grid(x - 1, y) === '-') && (grid(x + 1, y) === '_') &&
                           ! isSolidVLineOrJumpOrPoint(grid(x - 1, y + 1))))) {
                    pathSet.insert(new Path(Vec2(x, y), Vec2(x, y + 0.5)));
                }

                // For drawing resistors: -.╱
                else if ((grid(x, y) === '.') &&
                         (grid(x - 1, y) === '-') &&
                         (grid(x + 1, y) === '╱')) {
                    pathSet.insert(new Path(Vec2(x, y), Vec2(x + 0.5, y + 0.5)));
                }
                
                // For drawing resistors: ╱'-
                else if ((grid(x, y) === "'") &&
                         (grid(x + 1, y) === '-') &&
                         (grid(x - 1, y) === '╱')) {
                    pathSet.insert(new Path(Vec2(x, y), Vec2(x - 0.5, y - 0.5)));
                }

            } // y
        } // x
        
        // Find all solid horizontal lines 
        for (var y = 0; y < grid.height; ++y) {
            for (var x = 0; x < grid.width; ++x) {
                if (grid.isSolidHLineAt(x, y)) {
                    // Begins a line...find the end
                    var A = Vec2(x, y);
                    do { grid.setUsed(x, y); ++x; } while (grid.isSolidHLineAt(x, y));
                    var B = Vec2(x - 1, y);

                    // Detect adjacent box-drawing characters and lengthen the edge
                    if (grid(B.x + 1, B.y) === '┫') { B.x += 0.5; }
                    if (grid(A.x - 1, A.y) === '┣') { A.x -= 0.5; }

                    // Detect curves and shorten the edge
                    if ( ! isVertex(grid(A.x - 1, A.y)) && 
                         ((isTopVertex(grid(A)) && isSolidVLineOrJumpOrPoint(grid(A.x - 1, A.y + 1))) ||
                          (isBottomVertex(grid(A)) && isSolidVLineOrJumpOrPoint(grid(A.x - 1, A.y - 1))))) {
                        ++A.x;
                    }

                    if ( ! isVertex(grid(B.x + 1, B.y)) && 
                         ((isTopVertex(grid(B)) && isSolidVLineOrJumpOrPoint(grid(B.x + 1, B.y + 1))) ||
                          (isBottomVertex(grid(B)) && isSolidVLineOrJumpOrPoint(grid(B.x + 1, B.y - 1))))) {
                        --B.x;
                    }

                    // Only insert non-degenerate lines
                    if ((A.x !== B.x) || (A.y !== B.y)) {
                        pathSet.insert(new Path(A, B));
                    }
                    
                    // Continue the search from the end x+1
                }
            }
        } // y

        // Find all solid left-to-right downward diagonal lines (BACK DIAGONAL)
        for (var i = -grid.height; i < grid.width; ++i) {
            for (var x = i, y = 0; y < grid.height; ++y, ++x) {
                if (grid.isSolidBLineAt(x, y)) {
                    // Begins a line...find the end
                    var A = Vec2(x, y);
                    do { ++x; ++y; } while (grid.isSolidBLineAt(x, y));
                    var B = Vec2(x - 1, y - 1);

                    // Ensure that the entire line wasn't just vertices
                    if (lineContains(A, B, '\\')) {
                        for (var j = A.x; j <= B.x; ++j) {
                            grid.setUsed(j, A.y + (j - A.x)); 
                        }

                        var top = grid(A);
                        var up = grid(A.x, A.y - 1);
                        var uplt = grid(A.x - 1, A.y - 1);
                        if ((up === '/') || (uplt === '_') || (up === '_') || 
                            (! isVertex(top)  && 
                             (isSolidHLine(uplt) || isSolidVLine(uplt)))) {
                            // Continue half a cell more to connect for:
                            //  ___   ___
                            //  \        \    /      ----     |
                            //   \        \   \        ^      |^
                            A.x -= 0.5; A.y -= 0.5;
                        } else if (isPoint(uplt)) {
                            // Continue 1/4 cell more to connect for:
                            //
                            //  o
                            //   ^
                            //    \
                            A.x -= 0.25; A.y -= 0.25;
                        }
                        
                        var bottom = grid(B);
                        var dnrt = grid(B.x + 1, B.y + 1);
                        if ((grid(B.x, B.y + 1) === '/') || (grid(B.x + 1, B.y) === '_') || 
                            (grid(B.x - 1, B.y) === '_') || 
                            (! isVertex(grid(B)) &&
                             (isSolidHLine(dnrt) || isSolidVLine(dnrt)))) {
                            // Continue half a cell more to connect for:
                            //                       \      \ |
                            //  \       \     \       v      v|
                            //   \__   __\    /      ----     |
                            
                            B.x += 0.5; B.y += 0.5;
                        } else if (isPoint(dnrt)) {
                            // Continue 1/4 cell more to connect for:
                            //
                            //    \
                            //     v
                            //      o
                            
                            B.x += 0.25; B.y += 0.25;
                        }
                        
                        pathSet.insert(new Path(A, B));
                        // Continue the search from the end x+1,y+1
                    } // lineContains
                }
            }
        } // i


        // Find all solid left-to-right upward diagonal lines (DIAGONAL)
        for (var i = -grid.height; i < grid.width; ++i) {
            for (var x = i, y = grid.height - 1; y >= 0; --y, ++x) {
                if (grid.isSolidDLineAt(x, y)) {
                    // Begins a line...find the end
                    var A = Vec2(x, y);
                    do { ++x; --y; } while (grid.isSolidDLineAt(x, y));
                    var B = Vec2(x - 1, y + 1);

                    if (lineContains(A, B, '/')) {
                        // This is definitely a line. Commit the characters on it
                        for (var j = A.x; j <= B.x; ++j) {
                            grid.setUsed(j, A.y - (j - A.x)); 
                        }

                        var up = grid(B.x, B.y - 1);
                        var uprt = grid(B.x + 1, B.y - 1);
                        var bottom = grid(B);
                        if ((up === '\\') || (up === '_') || (uprt === '_') || 
                            (! isVertex(grid(B)) &&
                             (isSolidHLine(uprt) || isSolidVLine(uprt)))) {
                            
                            // Continue half a cell more to connect at:
                            //     __   __  ---     |
                            //    /      /   ^     ^|
                            //   /      /   /     / |
                            
                            B.x += 0.5; B.y -= 0.5;
                        } else if (isPoint(uprt)) {
                            
                            // Continue 1/4 cell more to connect at:
                            //
                            //       o
                            //      ^
                            //     /
                            
                            B.x += 0.25; B.y -= 0.25;
                        }
                        
                        var dnlt = grid(A.x - 1, A.y + 1);
                        var top = grid(A);
                        if ((grid(A.x, A.y + 1) === '\\') || (grid(A.x - 1, A.y) === '_') || (grid(A.x + 1, A.y) === '_') ||
                            (! isVertex(grid(A)) &&
                             (isSolidHLine(dnlt) || isSolidVLine(dnlt)))) {

                            // Continue half a cell more to connect at:
                            //               /     \ |
                            //    /  /      v       v|
                            // __/  /__   ----       | 
                            
                            A.x -= 0.5; A.y += 0.5;
                        } else if (isPoint(dnlt)) {
                            
                            // Continue 1/4 cell more to connect at:
                            //
                            //       /
                            //      v
                            //     o
                            
                            A.x -= 0.25; A.y += 0.25;
                        }
                        pathSet.insert(new Path(A, B));

                        // Continue the search from the end x+1,y-1
                    } // lineContains
                }
            }
        } // y
        
        
        // Now look for curved corners. The syntax constraints require
        // that these can always be identified by looking at three
        // horizontally-adjacent characters.
        for (var y = 0; y < grid.height; ++y) {
            for (var x = 0; x < grid.width; ++x) {
                var c = grid(x, y);

                // Note that because of undirected vertices, the
                // following cases are not exclusive
                if (isTopVertex(c)) {
                    // -.
                    //   |
                    if (isSolidHLine(grid(x - 1, y)) && isSolidVLine(grid(x + 1, y + 1))) {
                        grid.setUsed(x - 1, y); grid.setUsed(x, y); grid.setUsed(x + 1, y + 1);
                        pathSet.insert(new Path(Vec2(x - 1, y), Vec2(x + 1, y + 1), 
                                                Vec2(x + 1.1, y), Vec2(x + 1, y + 1)));
                    }

                    //  .-
                    // |
                    if (isSolidHLine(grid(x + 1, y)) && isSolidVLine(grid(x - 1, y + 1))) {
                        grid.setUsed(x - 1, y + 1); grid.setUsed(x, y); grid.setUsed(x + 1, y);
                        pathSet.insert(new Path(Vec2(x + 1, y), Vec2(x - 1, y + 1), 
                                                Vec2(x - 1.1, y), Vec2(x - 1, y + 1)));
                    }
                }
                
                // Special case patterns:
                //   .  .   .  .    
                //  (  o     )  o
                //   '  .   '  '
                if (((c === ')') || isPoint(c)) && (grid(x - 1, y - 1) === '.') && (grid(x - 1, y + 1) === "\'")) {
                    grid.setUsed(x, y); grid.setUsed(x - 1, y - 1); grid.setUsed(x - 1, y + 1);
                    pathSet.insert(new Path(Vec2(x - 2, y - 1), Vec2(x - 2, y + 1), 
                                            Vec2(x + 0.6, y - 1), Vec2(x + 0.6, y + 1)));
                }

                if (((c === '(') || isPoint(c)) && (grid(x + 1, y - 1) === '.') && (grid(x + 1, y + 1) === "\'")) {
                    grid.setUsed(x, y); grid.setUsed(x + 1, y - 1); grid.setUsed(x + 1, y + 1);
                    pathSet.insert(new Path(Vec2(x + 2, y - 1), Vec2(x + 2, y + 1), 
                                            Vec2(x - 0.6, y - 1), Vec2(x - 0.6, y + 1)));
                }

                if (isBottomVertex(c)) {
                    //   |
                    // -' 
                    if (isSolidHLine(grid(x - 1, y)) && isSolidVLine(grid(x + 1, y - 1))) {
                        grid.setUsed(x - 1, y); grid.setUsed(x, y); grid.setUsed(x + 1, y - 1);
                        pathSet.insert(new Path(Vec2(x - 1, y), Vec2(x + 1, y - 1), 
                                                Vec2(x + 1.1, y), Vec2(x + 1, y - 1)));
                    }

                    // | 
                    //  '-
                    if (isSolidHLine(grid(x + 1, y)) && isSolidVLine(grid(x - 1, y - 1))) {
                        grid.setUsed(x - 1, y - 1); grid.setUsed(x, y); grid.setUsed(x + 1, y);
                        pathSet.insert(new Path(Vec2(x + 1, y), Vec2(x - 1, y - 1),
                                                Vec2(x - 1.1, y), Vec2(x - 1, y - 1)));
                    }
                }
               
            } // for x
        } // for y

        // Find low horizontal lines marked with underscores. These
        // are so simple compared to the other cases that we process
        // them directly here without a helper function. Process these
        // from top to bottom and left to right so that we can read
        // them in a single sweep.
        // 
        // Exclude the special case of double underscores going right
        // into an ASCII character, which could be a source code
        // identifier such as __FILE__ embedded in the diagram.
        for (var y = 0; y < grid.height; ++y) {
            for (var x = 0; x < grid.width - 2; ++x) {
                var lt = grid(x - 1, y);

                if ((grid(x, y) === '_') && (grid(x + 1, y) === '_') && 
                    (! isASCIILetter(grid(x + 2, y)) || (lt === '_')) && 
                    (! isASCIILetter(lt) || (grid(x + 2, y) === '_'))) {

                    var ltlt = grid(x - 2, y);
                    var A = Vec2(x - 0.5, y + 0.5);

                    if ((lt === '|') || (grid(x - 1, y + 1) === '|') ||
                        (lt === '.') || (grid(x - 1, y + 1) === "'")) {
                        // Extend to meet adjacent vertical
                        A.x -= 0.5;

                        // Very special case of overrunning into the side of a curve,
                        // needed for logic gate diagrams
                        if ((lt === '.') && 
                            ((ltlt === '-') ||
                             (ltlt === '.')) &&
                            (grid(x - 2, y + 1) === '(')) {
                            A.x -= 0.5;
                        }
                    } else if (lt === '/') {
                        A.x -= 1.0;
                    }

                    // Detect overrun of a tight double curve
                    if ((lt === '(') && (ltlt === '(') &&
                        (grid(x, y + 1) === "'") && (grid(x, y - 1) === '.')) {
                        A.x += 0.5;
                    }
                    lt = ltlt = undefined;

                    do { grid.setUsed(x, y); ++x; } while (grid(x, y) === '_');

                    var B = Vec2(x - 0.5, y + 0.5);
                    var c = grid(x, y);
                    var rt = grid(x + 1, y);
                    var dn = grid(x, y + 1);

                    if ((c === '|') || (dn === '|') || (c === '.') || (dn === "'")) {
                        // Extend to meet adjacent vertical
                        B.x += 0.5;

                        // Very special case of overrunning into the side of a curve,
                        // needed for logic gate diagrams
                        if ((c === '.') && 
                            ((rt === '-') || (rt === '.')) &&
                            (grid(x + 1, y + 1) === ')')) {
                            B.x += 0.5;
                        }
                    } else if ((c === '\\')) {
                        B.x += 1.0;
                    }

                    // Detect overrun of a tight double curve
                    if ((c === ')') && (rt === ')') && (grid(x - 1, y + 1) === "'") && (grid(x - 1, y - 1) === '.')) {
                        B.x += -0.5;
                    }

                    pathSet.insert(new Path(A, B));
                }
            } // for x
        } // for y
    } // findPaths


    function findDecorations(grid, pathSet, decorationSet) {
        function isEmptyOrVertex(c) { return (c === ' ') || /[^a-zA-Z0-9]|[ov]/.test(c); }
        function isLetter(c) { var x = c.toUpperCase().charCodeAt(0); return (x > 64) && (x < 91); }
                    
        /** Is the point in the center of these values on a line? Allow points that are vertically
            adjacent but not horizontally--they wouldn't fit anyway, and might be text. */
        function onLine(up, dn, lt, rt) {
            return ((isEmptyOrVertex(dn) || isPoint(dn)) &&
                    (isEmptyOrVertex(up) || isPoint(up)) &&
                    isEmptyOrVertex(rt) &&
                    isEmptyOrVertex(lt));
        }

        for (var x = 0; x < grid.width; ++x) {
            for (var j = 0; j < grid.height; ++j) {
                var c = grid(x, j);
                var y = j;

                if (isJump(c)) {

                    // Ensure that this is really a jump and not a stray character
                    if (pathSet.downEndsAt(x, y - 0.5) &&
                        pathSet.upEndsAt(x, y + 0.5)) {
                        decorationSet.insert(x, y, c);
                        grid.setUsed(x, y);
                    }

                } else if (isPoint(c)) {
                    var up = grid(x, y - 1);
                    var dn = grid(x, y + 1);
                    var lt = grid(x - 1, y);
                    var rt = grid(x + 1, y);
                    var llt = grid(x - 2, y);
                    var rrt = grid(x + 2, y);

                    if (pathSet.rightEndsAt(x - 1, y) ||   // Must be at the end of a line...
                        pathSet.leftEndsAt(x + 1, y) ||    // or completely isolated NSEW
                        pathSet.downEndsAt(x, y - 1) ||
                        pathSet.upEndsAt(x, y + 1) ||

                        pathSet.upEndsAt(x, y) ||    // For points on vertical lines 
                        pathSet.downEndsAt(x, y) ||  // that are surrounded by other characters
                        
                        onLine(up, dn, lt, rt)) {

                        decorationSet.insert(x, y, c);
                        grid.setUsed(x, y);
                    }
                } else if (isGray(c)) {
                    decorationSet.insert(x, y, c);
                    grid.setUsed(x, y);
                } else if (isTri(c)) {
                    decorationSet.insert(x, y, c);
                    grid.setUsed(x, y);
                } else { // Arrow heads

                    // If we find one, ensure that it is really an
                    // arrow head and not a stray character by looking
                    // for a connecting line.
                    var dx = 0;
                    if ((c === '>') && (pathSet.rightEndsAt(x, y) || 
                                        pathSet.horizontalPassesThrough(x, y))) {
                        if (isPoint(grid(x + 1, y))) {
                            // Back up if connecting to a point so as to not
                            // overlap it
                            dx = -0.5;
                        }
                        decorationSet.insert(x + dx, y, '>', 0);
                        grid.setUsed(x, y);
                    } else if ((c === '<') && (pathSet.leftEndsAt(x, y) ||
                                               pathSet.horizontalPassesThrough(x, y))) {
                        if (isPoint(grid(x - 1, y))) {
                            // Back up if connecting to a point so as to not
                            // overlap it
                            dx = 0.5;
                        }
                        decorationSet.insert(x + dx, y, '>', 180); 
                        grid.setUsed(x, y);
                    } else if (c === '^') {
                        // Because of the aspect ratio, we need to look
                        // in two slots for the end of the previous line
                        if (pathSet.upEndsAt(x, y - 0.5)) {
                            decorationSet.insert(x, y - 0.5, '>', 270); 
                            grid.setUsed(x, y);
                        } else if (pathSet.upEndsAt(x, y)) {
                            decorationSet.insert(x, y, '>', 270);
                            grid.setUsed(x, y);
                        } else if (pathSet.diagonalUpEndsAt(x + 0.5, y - 0.5)) {
                            decorationSet.insert(x + 0.5, y - 0.5, '>', 270 + DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.diagonalUpEndsAt(x + 0.25, y - 0.25)) {
                            decorationSet.insert(x + 0.25, y - 0.25, '>', 270 + DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.diagonalUpEndsAt(x, y)) {
                            decorationSet.insert(x, y, '>', 270 + DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.backDiagonalUpEndsAt(x, y)) {
                            decorationSet.insert(x, y, c, 270 - DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.backDiagonalUpEndsAt(x - 0.5, y - 0.5)) {
                            decorationSet.insert(x - 0.5, y - 0.5, c, 270 - DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.backDiagonalUpEndsAt(x - 0.25, y - 0.25)) {
                            decorationSet.insert(x - 0.25, y - 0.25, c, 270 - DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.verticalPassesThrough(x, y)) {
                            // Only try this if all others failed
                            decorationSet.insert(x, y - 0.5, '>', 270); 
                            grid.setUsed(x, y);
                        }
                    } else if (c === 'v') {
                        if (pathSet.downEndsAt(x, y + 0.5)) {
                            decorationSet.insert(x, y + 0.5, '>', 90); 
                            grid.setUsed(x, y);
                        } else if (pathSet.downEndsAt(x, y)) {
                            decorationSet.insert(x, y, '>', 90);
                            grid.setUsed(x, y);
                        } else if (pathSet.diagonalDownEndsAt(x, y)) {
                            decorationSet.insert(x, y, '>', 90 + DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.diagonalDownEndsAt(x - 0.5, y + 0.5)) {
                            decorationSet.insert(x - 0.5, y + 0.5, '>', 90 + DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.diagonalDownEndsAt(x - 0.25, y + 0.25)) {
                            decorationSet.insert(x - 0.25, y + 0.25, '>', 90 + DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.backDiagonalDownEndsAt(x, y)) {
                            decorationSet.insert(x, y, '>', 90 - DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.backDiagonalDownEndsAt(x + 0.5, y + 0.5)) {
                            decorationSet.insert(x + 0.5, y + 0.5, '>', 90 - DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.backDiagonalDownEndsAt(x + 0.25, y + 0.25)) {
                            decorationSet.insert(x + 0.25, y + 0.25, '>', 90 - DIAGONAL_ANGLE);
                            grid.setUsed(x, y);
                        } else if (pathSet.verticalPassesThrough(x, y)) {
                            // Only try this if all others failed
                            decorationSet.insert(x, y + 0.5, '>', 90); 
                            grid.setUsed(x, y);
                        }
                    } // arrow heads
                } // decoration type
            } // y
        } // x
    } // findArrowHeads

    // Cases where we want to redraw at graphical unicode character
    // to adjust its weight or shape for a conventional application
    // in constructing a diagram.
    function findReplacementCharacters(grid, pathSet) {
        for (var x = 0; x < grid.width; ++x) {
            for (var y = 0; y < grid.height; ++y) {
                if (grid.isUsed(x, y)) continue;
                var c = grid(x, y);
                switch (c) {
                case '╱':
                    pathSet.insert(new Path(Vec2(x - 0.5, y + 0.5), Vec2(x + 0.5, y - 0.5)));
                    grid.setUsed(x, y);
                    break;
                case '╲':
                    pathSet.insert(new Path(Vec2(x - 0.5, y - 0.5), Vec2(x + 0.5, y + 0.5)));
                    grid.setUsed(x, y);
                    break;
                }
            }
        }
    } // findReplacementCharacters

    var grid = makeGrid(diagramString);

    var pathSet = new PathSet();
    var decorationSet = new DecorationSet();

    findPaths(grid, pathSet);
    findReplacementCharacters(grid, pathSet);
    findDecorations(grid, pathSet, decorationSet);

    var svg = '<svg class="diagram" xmlns="http://www.w3.org/2000/svg" version="1.1" height="' + 
        ((grid.height + 1) * SCALE * ASPECT) + '" width="' + ((grid.width + 1) * SCALE) + '"';

    if (alignmentHint === 'floatleft') {
        svg += ' style="float:left;margin:15px 30px 15px 0;"';
    } else if (alignmentHint === 'floatright') {
        svg += ' style="float:right;margin:15px 0 15px 30px;"';
    } else if (alignmentHint === 'center') {
        svg += ' style="margin:0 auto 0 auto;"';
    }

    svg += '><g transform="translate(' + Vec2(1, 1) + ')">\n';

    if (DEBUG_SHOW_GRID) {
        svg += '<g style="opacity:0.1">\n';
        for (var x = 0; x < grid.width; ++x) {
            for (var y = 0; y < grid.height; ++y) {
                svg += '<rect x="' + ((x - 0.5) * SCALE + 1) + '" + y="' + ((y - 0.5) * SCALE * ASPECT + 2) + '" width="' + (SCALE - 2) + '" height="' + (SCALE * ASPECT - 2) + '" style="fill:';
                if (grid.isUsed(x, y)) {
                    svg += 'red;';
                } else if (grid(x, y) === ' ') {
                    svg += 'gray;opacity:0.05';
                } else {
                    svg += 'blue;';
                }
                svg += '"/>\n';
            }
        }
        svg += '</g>\n';
    }
    
    svg += pathSet.toSVG();
    svg += decorationSet.toSVG();

    // Convert any remaining characters
    if (! DEBUG_HIDE_PASSTHROUGH) {
        svg += '<g transform="translate(0,0)">';
        for (var y = 0; y < grid.height; ++y) {
            for (var x = 0; x < grid.width; ++x) {
                var c = grid(x, y);
                if (/[\u2B22\u2B21]/.test(c)) {
                    // Enlarge hexagons so that they fill a grid
                    svg += '<text text-anchor="middle" x="' + (x * SCALE) + '" y="' + (4 + y * SCALE * ASPECT) + '" style="font-size:20.5px">' + escapeHTMLEntities(c) +  '</text>';
                } else if ((c !== ' ') && ! grid.isUsed(x, y)) {
                    svg += '<text text-anchor="middle" x="' + (x * SCALE) + '" y="' + (4 + y * SCALE * ASPECT) + '">' + escapeHTMLEntities(c) +  '</text>';
                } // if
            } // y
        } // x
        svg += '</g>';
    }

    if (DEBUG_SHOW_SOURCE) {
        // Offset the characters a little for easier viewing
        svg += '<g transform="translate(2,2)">\n';
        for (var x = 0; x < grid.width; ++x) {
            for (var y = 0; y < grid.height; ++y) {
                var c = grid(x, y);
                if (c !== ' ') {
                    svg += '<text text-anchor="middle" x="' + (x * SCALE) + '" y="' + (4 + y * SCALE * ASPECT) + '" style="fill:#F00;font-family:Menlo,monospace;font-size:12px;text-align:center">' + escapeHTMLEntities(c) +  '</text>';
                } // if
            } // y
        } // x
        svg += '</g>';
    } // if

    svg += '</g></svg>';

    svg = svg.rp(new RegExp(HIDE_O, 'g'), 'o');


    return svg;
}


////////////////////////// Processing for INSERT HERE
//
// Insert command processing modifies the entire document and potentially
// delays further processing, so it is handled specially and runs the main
// markdeep processing as a callback
//
// node: the node being processed for markdeep. This is document.body
// in markdeep mode, but may be another node in html or script mode.
//
// processMarkdeepCallback: function to run when insert is complete
// to evaluate markdeep 
function processInsertCommands(nodeArray, sourceArray, insertDoneCallback) {
    var myURLParse = /([^?]+)(?:\?id=(inc\d+)&p=([^&]+))?/.exec(location.href);

    var myBase = removeFilename(myURLParse[1]);
    var myID = myURLParse[2];
    var parentBase = removeFilename(myURLParse[3] && decodeURIComponent(myURLParse[3]));
    var childFrameStyle = 'display:none';
    var includeCounter = 0;
    var IAmAChild = myID; // !== undefined
    var IAmAParent = false;
    var numIncludeChildrenLeft = 0;
    
    // Helper function for use by children
    function sendContentsToMyParent() {
        var body = document.body.innerHTML;

        // Fix relative URLs within the body
        var baseref;
        if (document.baseURI !== undefined) {
            baseref = document.baseURI.rp(/\/[^/]+$/, '/');
        } else {
            // IE11
            // Return location from BASE tag.
            //   https://developer.mozilla.org/en-US/docs/Web/HTML/Element/base
            var base = document.getElementsByTagName('base');
            baseref = (base.length > 0) ? base[0].href : document.URL;
        }

        var serverref;
        if (/^file:\/\//.test(baseref)) {
            serverref = 'file://';
        } else {
            serverref = baseref.match(/[^:/]{3,6}:\/\/[^/]*\//)[0];
        }

        // Cases where URLs appear:
        //
        // ![](...)
        // [](...)
        // [link]: ...
        // <img src="...">
        // <script src="...">
        // <a href="...">
        // <link href="...">
        //
        // A url is relative if it does not begin with '^[a-z]{3,6}://|^#'

        // Protect code fences
        // TODO

        function makeAbsoluteURL(url) {
            return (/^[a-z]{3,6}:\/\//.test(url)) ?
                url :
                (url[0] === '/') ?
                // Make relative to server
                serverref + url.ss(1) :
                // Make relative to source document
                baseref + url;
        }

        // Unquoted images and links
        body = body.rp(/\]\([ \t]*([^#")][^ "\)]+)([ \t\)])/g, function (match, url, suffix) {
            return '](' + makeAbsoluteURL(url) + suffix;
        });
        
        // Quoted images and links
        body = body.rp(/\]\([ \t]*"([^#"][^"]+)"([ \t\)])/g, function (match, url, suffix) {
            return ']("' + makeAbsoluteURL(url) + '"' + suffix;
        });

        // Raw HTML
        body = body.rp(/(src|href)=(["'])([^#>][^"'\n>]+)\2/g, function (match, type, quot, url) {
            return type + '=' + quot + makeAbsoluteURL(url) + quot;
        });

        // Reference links
        body = body.rp(/(\n\[[^\]>\n \t]:[ \t]*)([^# \t][^ \t]+)"/g, function (match, prefix, url) {
            return prefix + makeAbsoluteURL(url);
        });

        // Unprotect code fences
        // TODO
        
        // console.log(location.pathname + " sent message to parent");
        // Send the document contents after the childFrame replaced itself
        // (not the source variable captured when this function was defined!)
        parent.postMessage([myID, '=', body].join(''), '*');
    }

    // Strip the filename from the url, if there is one (and it is a string)
    function removeFilename(url) {
        return url && url.ss(0, url.lastIndexOf('/') + 1);
    }

    // Called when this entire document is ready for either markdeep
    // processing or sending to its parent for markdeep processing.
    //
    // IAmAChild: Truish if this document is a child
    //
    // sourceArray: If known, source is the code for the nodes. If it was modified, it is not provided
    function documentReady(IAmAChild, nodeArray, sourceArray) {
        if (IAmAChild) {
            // I'm a child and not waiting for my own children, so trigger the send now. My parent will
            // do the processing.
            
            // console.log("Leaf node " + location.pathname + " sending to parent");
            sendContentsToMyParent();
        } else {
            // No includes. Run markdeep processing after the rest of this file parses
            
            // console.log("non-parent, non-child Parent scheduling markdeepProcessor");
            setTimeout(function () { insertDoneCallback(nodeArray, sourceArray) }, 1);
        }
    }
     
     function messageCallback(event) {
         // Parse the message. Ensure that it is for the Markdeep/include.js system.
         var childID = false;
         var childBody = event.data.substring && event.data.replace(/^(inc\d+)=/, function (match, a) {
             childID = a;
             return '';
         });
         
         if (childID) {
             // This message event was for the Markdeep/include.js system
             
             //console.log(location.href + ' received a message from child ' + childID);

             // Replace the corresponding node's contents
             var childFrame = document.getElementById(childID);
             childFrame.outerHTML = '\n' + childBody + '\n';

             --numIncludeChildrenLeft;

             //console.log(window.location.pathname, 'numIncludeChildrenLeft = ' + numIncludeChildrenLeft);
             
             if (numIncludeChildrenLeft <= 0) {
                 // This was the last child
                 documentReady(IAmAChild, nodeArray);
             }
         }
     };

     var isFirefox = navigator.userAgent.indexOf('Firefox') !== -1 && navigator.userAgent.indexOf('Seamonkey') === -1;
    
     // Find all insert or embed statements in all nodes and replace them
     for (var i = 0; i < sourceArray.length; ++i) {
         sourceArray[i] = sourceArray[i].rp(/(?:^|\s)\((insert|embed)[ \t]+(\S+\.\S*)[ \t]+(height=[a-zA-Z0-9.]+[ \t]+)?here\)\s/g, function(match, type, src, params) {
             var childID = 'inc' + (++includeCounter);
             var isHTML = src.toLowerCase().rp(/\?.*$/,'').endsWith('.html');
             if (type === 'embed' || ! isHTML) {
                 // This is not embedding another Markdeep file. Instead it is embedding
                 // some other kind of document.
                 var tag = 'iframe', url='src';
                 var style = params ? ' style="' + params.rp(/=/g, ':') + '"' : '';
                 
                 if (isFirefox && ! isHTML) {
                     // Firefox doesn't handle embedding other non-html documents in iframes
                     // correctly (it tries to download them!), so we switch to an object
                     // tag--which seems to work identically to the embed tag on this browser.                     
                     tag = 'object'; url = 'data';

                     // Firefox can be confused on a server (but not
                     // locally) by misconfigured MIME types and show
                     // nothing.  But if we know that we're on a
                     // server, we can go ahead and make an
                     // XMLHttpRequest() for the underlying document
                     // directly. Replace the insert in this case.
                     if (location.protocol.startsWith('http')) {
                         var req = new XMLHttpRequest();
                         (function (childID, style) {
                             req.addEventListener("load", function () {
                                 document.getElementById(childID).outerHTML =
                                     entag('iframe', '', 'class="textinsert" srcdoc="<pre>' + this.responseText.replace(/"/g, '&quot;') + '</pre>"' + style);
                             });
                             req.overrideMimeType("text/plain; charset=x-user-defined");
                             req.open("GET", src); 
                             req.send();
                         })(childID, style);
                     }
                 }

                 return entag(tag, '', 'class="textinsert" id="' + childID + '" ' + url + '="' + src + '"' + style);
             }
             
             if (numIncludeChildrenLeft === 0) {
                 // This is the first child observed. Prepare to receive messages from the
                 // embedded children.
                 IAmAParent = true;
                 addEventListener("message", messageCallback);
             }
             
             ++numIncludeChildrenLeft;
             //console.log(window.location.pathname, 'numIncludeChildrenLeft = ' + numIncludeChildrenLeft);
             
             // Replace this tag with a frame that loads the document.  Once loaded, it will
             // send a message with its contents for use as a replacement.
             return '<iframe src="' + src + '?id=' + childID + '&p=' + encodeURIComponent(myBase) + 
                 '" id="' + childID + '" style="' + childFrameStyle + '" content="text/html;charset=UTF-8"></iframe>';
         });
     }

     // console.log('after insert: ' + source);

     // Process all nodes
     if (IAmAParent) {
         // I'm waiting on children, so don't run the full processor
         // yet, but do substitute the iframe code so that it can
         // launch. I may be a child as well...this will be determined
         // when numIncludeChildren hits zero.

         for (var i = 0; i < sourceArray.length; ++i) {
             nodeArray[i].innerHTML = sourceArray[i];
         }
     } else {
         // The source was not modified
         documentReady(IAmAChild, nodeArray, sourceArray);
     }
} // function processInsertCommands()

 
/* xcode.min.js modified */
var HIGHLIGHT_STYLESHEET =
        "<style>.hljs{display:block;overflow-x:auto;padding:0.5em;background:#fff;color:#000;-webkit-text-size-adjust:none}"+
        ".hljs-comment{color:#006a00}" +
        ".hljs-keyword{color:#02E}" +
        ".hljs-literal,.nginx .hljs-title{color:#aa0d91}" + 
        ".method,.hljs-list .hljs-title,.hljs-tag .hljs-title,.setting .hljs-value,.hljs-winutils,.tex .hljs-command,.http .hljs-title,.hljs-request,.hljs-status,.hljs-name{color:#008}" + 
        ".hljs-envvar,.tex .hljs-special{color:#660}" + 
        ".hljs-string{color:#c41a16}" +
        ".hljs-tag .hljs-value,.hljs-cdata,.hljs-filter .hljs-argument,.hljs-attr_selector,.apache .hljs-cbracket,.hljs-date,.hljs-regexp{color:#080}" + 
        ".hljs-sub .hljs-identifier,.hljs-pi,.hljs-tag,.hljs-tag .hljs-keyword,.hljs-decorator,.ini .hljs-title,.hljs-shebang,.hljs-prompt,.hljs-hexcolor,.hljs-rule .hljs-value,.hljs-symbol,.hljs-symbol .hljs-string,.hljs-number,.css .hljs-function,.hljs-function .hljs-title,.coffeescript .hljs-attribute{color:#A0C}" +
        ".hljs-function .hljs-title{font-weight:bold;color:#000}" + 
        ".hljs-class .hljs-title,.smalltalk .hljs-class,.hljs-type,.hljs-typename,.hljs-tag .hljs-attribute,.hljs-doctype,.hljs-class .hljs-id,.hljs-built_in,.setting,.hljs-params,.clojure .hljs-attribute{color:#5c2699}" +
        ".hljs-variable{color:#3f6e74}" +
        ".css .hljs-tag,.hljs-rule .hljs-property,.hljs-pseudo,.hljs-subst{color:#000}" + 
        ".css .hljs-class,.css .hljs-id{color:#9b703f}" +
        ".hljs-value .hljs-important{color:#ff7700;font-weight:bold}" +
        ".hljs-rule .hljs-keyword{color:#c5af75}" +
        ".hljs-annotation,.apache .hljs-sqbracket,.nginx .hljs-built_in{color:#9b859d}" +
        ".hljs-preprocessor,.hljs-preprocessor *,.hljs-pragma{color:#643820}" +
        ".tex .hljs-formula{background-color:#eee;font-style:italic}" +
        ".diff .hljs-header,.hljs-chunk{color:#808080;font-weight:bold}" +
        ".diff .hljs-change{background-color:#bccff9}" +
        ".hljs-addition{background-color:#baeeba}" +
        ".hljs-deletion{background-color:#ffc8bd}" +
        ".hljs-comment .hljs-doctag{font-weight:bold}" +
        ".method .hljs-id{color:#000}</style>";

function isMarkdeepScriptName(str) { return str.search(/markdeep\S*?\.js$/i) !== -1; }
function toArray(list) { return Array.prototype.slice.call(list); }

// Intentionally uninitialized global variable used to detect
// recursive invocations
if (! window.alreadyProcessedMarkdeep) {
    window.alreadyProcessedMarkdeep = true;

    // Detect the noformat argument to the URL
    var noformat = (window.location.href.search(/\?.*noformat.*/i) !== -1);

    // Export relevant methods
    window.markdeep = Object.freeze({ 
        format:               markdeepToHTML,
        formatDiagram:        diagramToSVG,
        stylesheet:           function() {
            return STYLESHEET + sectionNumberingStylesheet() + HIGHLIGHT_STYLESHEET;
        }
    });

    // Not needed: jax: ["input/TeX", "output/SVG"], 
    var MATHJAX_CONFIG ='<script type="text/x-mathjax-config">MathJax.Hub.Config({ TeX: { equationNumbers: {autoNumber: "AMS"} } });</script>' +
        '<span style="display:none">' +
        // Custom definitions (NC == \newcommand)
        '$$NC{\\n}{\\hat{n}}NC{\\thetai}{\\theta_\\mathrm{i}}NC{\\thetao}{\\theta_\\mathrm{o}}NC{\\d}[1]{\\mathrm{d}#1}NC{\\w}{\\hat{\\omega}}NC{\\wi}{\\w_\\mathrm{i}}NC{\\wo}{\\w_\\mathrm{o}}NC{\\wh}{\\w_\\mathrm{h}}NC{\\Li}{L_\\mathrm{i}}NC{\\Lo}{L_\\mathrm{o}}NC{\\Le}{L_\\mathrm{e}}NC{\\Lr}{L_\\mathrm{r}}NC{\\Lt}{L_\\mathrm{t}}NC{\\O}{\\mathrm{O}}NC{\\degrees}{{^{\\large\\circ}}}NC{\\T}{\\mathsf{T}}NC{\\mathset}[1]{\\mathbb{#1}}NC{\\Real}{\\mathset{R}}NC{\\Integer}{\\mathset{Z}}NC{\\Boolean}{\\mathset{B}}NC{\\Complex}{\\mathset{C}}NC{\\un}[1]{\\,\\mathrm{#1}}$$\n'.rp(/NC/g, '\\newcommand') +
        '</span>\n';

    // The following option forces better rendering on some browsers, but also makes it impossible to copy-paste text with
    // inline equations:
    //
    // 'config=TeX-MML-AM_SVG'
    var MATHJAX_URL = 'https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.6/MathJax.js?config=TeX-AMS-MML_HTMLorMML';

    var loadMathJax = function() {
        // Dynamically load mathjax
        var script = document.createElement("script");
        script.type = "text/javascript";
        script.src = MATHJAX_URL;
        document.getElementsByTagName("head")[0].appendChild(script);
    }

    var needsMathJax= function(html) {
        // Need MathJax if $$ ... $$, \( ... \), or \begin{
        return option('detectMath') &&
            ((html.search(/(?:\$\$[\s\S]+\$\$)|(?:\\begin{)/m) !== -1) || 
             (html.search(/\\\(.*\\\)/) !== -1));
    }

    var mode = option('mode');
    switch (mode) {
    case 'script':
        // Nothing to do
        return;

    case 'html':
    case 'doxygen':
        // Process explicit diagram tags by themselves
        toArray(document.getElementsByClassName('diagram')).concat(toArray(document.getElementsByTagName('diagram'))).forEach(
            function (element) {
                var src = unescapeHTMLEntities(element.innerHTML);
                // Remove the first and last string (which probably
                // had the pre or diagram tag as part of them) if they are 
                // empty except for whitespace.
                src = src.rp(/(:?^[ \t]*\n)|(:?\n[ \t]*)$/g, '');

                if (mode === 'doxygen') {
                    // Undo Doxygen's &ndash and &mdash, which are impossible to 
                    // detect once the browser has parsed the document
                    src = src.rp(new RegExp('\u2013', 'g'), '--');
                    src = src.rp(new RegExp('\u2014', 'g'), '---');
                    
                    // Undo Doxygen's links within the diagram because they throw off spacing
                    src = src.rp(/<a class="el" .*>(.*)<\/a>/g, '$1');
                }
                element.outerHTML = '<center class="md">' + diagramToSVG(removeLeadingSpace(src), '') + '</center>';
            });

        // Collect all nodes that will receive markdeep processing
        var markdeepNodeArray = toArray(document.getElementsByClassName('markdeep')).concat(toArray(document.getElementsByTagName('markdeep')));

        // Extract the source code of markeep nodes
        var sourceArray = markdeepNodeArray.map(function (node) {
            return removeLeadingSpace(unescapeHTMLEntities(node.innerHTML));
        });

        // Process insert commands and then trigger markdeep processing
        processInsertCommands(markdeepNodeArray, sourceArray, function (nodeArray, sourceArray) {
            // Update sourceArray if needed because the source code was mutated
            // by insert processing
            sourceArray = sourceArray || nodeArray.map(function (node) {
                return removeLeadingSpace(unescapeHTMLEntities(node.innerHTML));
            });
            
            // Process all nodes, replacing them as we progress
            var anyNeedsMathJax = false;
            for (var i = 0; i < markdeepNodeArray.length; ++i) {
                var oldNode = markdeepNodeArray[i];
                var newNode = document.createElement('div');
                var source = removeLeadingSpace(unescapeHTMLEntities(oldNode.innerHTML));
                var html = markdeepToHTML(source, true);
                anyNeedsMathJax = anyNeedsMathJax || needsMathJax(html);
                newNode.innerHTML = html;
                oldNode.parentNode.replaceChild(newNode, oldNode);
            }

            if (anyNeedsMathJax) { loadMathJax(); }

            // Include our stylesheet even if there are no MARKDEEP tags, but do not include the BODY_STYLESHEET.
            document.head.innerHTML = window.markdeep.stylesheet() + document.head.innerHTML + (anyNeedsMathJax ? MATHJAX_CONFIG : '');

            // Remove fallback nodes
            var fallbackNodes = document.getElementsByClassName('fallback');
            for (var i = 0; i < fallbackNodes.length; ++i) {
                fallbackNodes[i].remove();
            }

        });

        window.alreadyProcessedMarkdeep = true;

        return;
    }
    
    // The following is Morgan's massive hack for allowing browsers to
    // directly parse Markdown from what appears to be a text file, but is
    // actually an intentionally malformed HTML file.
    
    // In order to be able to show what source files look like, the
    // noformat argument may be supplied.
    
    if (! noformat) {
        // Remove any recursive references to this script so that we don't trigger the cost of
        // recursive *loading*. (The alreadyProcessedMarkdeep variable will prevent recursive
        // *execution*.) We allow other scripts to pass through.
        toArray(document.getElementsByTagName('script')).forEach(function(node) {
            if (isMarkdeepScriptName(node.src)) {
                node.parentNode.removeChild(node);
            }
        });
        
        // Add an event handler for scrolling
        var scrollThreshold = parseInt(option('scrollThreshold'));
        document.addEventListener('scroll', function () {
            var b = document.body, c = b.classList, s = 'scrolled';
            if (b.scrollTop > scrollThreshold) c.add(s); else c.remove(s);
        });
        
        // Hide the body while formatting
        if (document.body) {
            document.body.style.visibility = 'hidden';
        }
    }
      
    var source = nodeToMarkdeepSource(document.body);

    if (noformat) { 
        // Abort processing. 
        source = source.rp(/<!-- Markdeep:.+$/gm, '') + MARKDEEP_LINE;
    
        // Escape the <> (not ampersand) that we just added
        source = source.rp(/</g, '&lt;').rp(/>/g, '&gt;');

        // Replace the Markdeep line itself so that ?noformat examples have a valid line to copy
        document.body.innerHTML = entag('pre', source);

        var fallbackNodes = document.getElementsByClassName('fallback');
        for (var i = 0; i < fallbackNodes.length; ++i) {
            fallbackNodes[i].remove();
        }

        return;
    }

    // In the common case of no INSERT commands, source is the original source
    // passed to avoid reparsing.
    var markdeepProcessor = function (source) {
        // Recompute the source text from the current version of the document
        // if it was unmodified
        source = source || nodeToMarkdeepSource(document.body);
        var markdeepHTML = markdeepToHTML(source, false);

        // console.log(markdeepHTML); // Final processed source 

        /////////////////////////////////////////////////////////////
        // Add the section header event handlers

        var onContextMenu = function (event) {
            var menu = null;
            try {
                // Test for whether the click was on a header
                var match = event.target.tagName.match(/^H(\d)$/);
                if (! match) { return; }

                // The event target is a header...ensure that it is a Markdeep header
                // (we could be in HTML or Doxygen mode and have non-.md content in the
                // same document)
                var node = event.target;
                while (node) {
                    if (node.classList.contains('md')) { break } else { node = node.parentElement; }
                }
                if (! node) {
                    // never found .md
                    return;
                }
                    
                // We are on a header
                var level = parseInt(match[1]) || 1;
                
                // Show the headerMenu
                menu = document.getElementById('mdContextMenu');
                if (! menu) { return; }
                
                var sectionType = ['Section', 'Subsection'][Math.min(level - 1, 1)];
                // Search backwards two siblings to grab the URL generated
                var anchorNode = event.target.previousElementSibling.previousElementSibling;
                
                var sectionName = event.target.innerText.trim();
                var sectionLabel = sectionName.toLowerCase();
                var anchor = anchorNode.name;
                var url = '' + location.origin + location.pathname + '#' + anchor;

                var shortUrl = url;
                if (shortUrl.length > 17) {
                    shortUrl = url.ss(0, 7) + '&hellip;' + location.pathname.ss(location.pathname.length - 8) + '#' + anchor;
                }
                
                var s = entag('div', 'Visit URL &ldquo;' + shortUrl + '&rdquo;',
                              'onclick="(location=&quot;' + url + '&quot;)"');
                
                s += entag('div', 'Copy URL &ldquo;' + shortUrl + '&rdquo;',
                           'onclick="navigator.clipboard.writeText(&quot;' + url + '&quot)&&(document.getElementById(\'mdContextMenu\').style.visibility=\'hidden\')"');
                
                s += entag('div', 'Copy Markdeep &ldquo;' + sectionName + ' ' + sectionType.toLowerCase() + '&rdquo;',
                           'onclick="navigator.clipboard.writeText(\'' + sectionName + ' ' + sectionType.toLowerCase() + '\')&&(document.getElementById(\'mdContextMenu\').style.visibility=\'hidden\')"');
                
                s += entag('div', 'Copy Markdeep &ldquo;' + sectionType + ' [' + sectionLabel + ']&rdquo;',
                           'onclick="navigator.clipboard.writeText(\'' + sectionType + ' [' + sectionLabel + ']\')&&(document.getElementById(\'mdContextMenu\').style.visibility=\'hidden\')"');
                
                s += entag('div', 'Copy HTML &ldquo;&lt;a href=&hellip;&gt;&rdquo;',
                           'onclick="navigator.clipboard.writeText(\'&lt;a href=&quot;' + url + '&quot;&gt;' + sectionName + '&lt;/a&gt;\')&&(document.getElementById(\'mdContextMenu\').style.visibility=\'hidden\')"');
                
                menu.innerHTML = s;
                menu.style.visibility = 'visible';
                menu.style.left = event.pageX + 'px';
                menu.style.top = event.pageY + 'px';
                
                event.preventDefault();
                return false;
            } catch (e) {
                // Something went wrong
                console.log(e);
                if (menu) { menu.style.visibility = 'hidden'; }
            }
        }

        markdeepHTML += '<div id="mdContextMenu" style="visibility:hidden"></div>';
        
        document.addEventListener('contextmenu', onContextMenu, false);
        document.addEventListener('mousedown', function (event) {
            var menu = document.getElementById('mdContextMenu');
            if (menu) {
                for (var node = event.target; node; node = node.parentElement) {
                    if (node === menu) { return; }
                }
                // Clicked off menu, so close it
                menu.style.visibility = 'hidden';
            }
        });
        document.addEventListener('keydown', function (event) {
            if (event.keyCode === 27) {
                var menu = document.getElementById('mdContextMenu');
                if (menu) { menu.style.visibility = 'hidden'; }
            }
        });
        
        
        /////////////////////////////////////////////////////////////
        
        var needMathJax = needsMathJax(markdeepHTML);
        if (needMathJax) {
            markdeepHTML = MATHJAX_CONFIG + markdeepHTML; 
        }
        
        markdeepHTML += MARKDEEP_FOOTER;
        
        // Replace the document. If using MathJax, include the custom Markdeep definitions
        var longDocument = source.length > 1000;
        
        // Setting "width" equal to 640 seems to give the best results on 
        // mobile devices in portrait mode. Setting "width=device-width" can cause markdeep
        // to appear exceedingly narrow on phones in the Chrome mobile preview.
        // https://developer.mozilla.org/en-US/docs/Mozilla/Mobile/Viewport_meta_tag
        var META = '<meta charset="UTF-8"><meta http-equiv="content-type" content="text/html;charset=UTF-8"><meta name="viewport" content="width=600, initial-scale=1">';
        var head = META + BODY_STYLESHEET + STYLESHEET + sectionNumberingStylesheet() + HIGHLIGHT_STYLESHEET;
        if (longDocument) {
            // Add more spacing before the title in a long document
            head += entag('style', 'div.title { padding-top: 40px; } div.afterTitles { height: 15px; }');
        }

        if (window.location.href.search(/\?.*export.*/i) !== -1) {
            // Export mode
            var text = head + document.head.innerHTML + markdeepHTML;
            if (needMathJax) {
                // Dynamically load mathjax
                text += '<script src="' + MATHJAX_URL +'"></script>';
            }
            document.body.innerHTML = entag('pre', escapeHTMLEntities(text));
        } else {
            document.head.innerHTML = head + document.head.innerHTML;
            document.body.innerHTML = markdeepHTML;
            if (needMathJax) { loadMathJax(); }            
        }

        // Change the ID of the body, so that CSS can distinguish Markdeep
        // controlling a whole document from Markdeep embedded within
        // a document in HTML mode.
        document.body.id = 'md';
        document.body.style.visibility = 'visible';

        var hashIndex = window.location.href.indexOf('#');
        if (hashIndex > -1) {
            // Scroll to the target; needed when loading is too fast (ironically)
            setTimeout(function () {
                var anchor = document.getElementsByName(window.location.href.substring(hashIndex + 1));
                if (anchor.length > 0) { anchor[0].scrollIntoView(); }
                if (window.markdeepOptions) (window.markdeepOptions.onLoad || Math.cos)();
            }, 100);
        } else if (window.markdeepOptions && window.markdeepOptions.onLoad) {
            // Wait for the DOM to update
            setTimeout(window.markdeepOptions.onLoad, 100);
        }
           
    };
    
    // Process insert commands, and then run the markdeepProcessor on the document
    processInsertCommands([document.body], [source], function (nodeArray, sourceArray) {
        markdeepProcessor(sourceArray && sourceArray[0]);
    });
}
    
})();

/*
  Highlight.js 10.5.0
  License: BSD-3-Clause
  Copyright (c) 2006-2020, Ivan Sagalaev

  This contains regexps with character groups that do not escape
  '['. For example, /[a-z[\]]/. That is technically legal in most
  situations but confuses syntax highlighting of this source 
  itself in emacs and sometimes also fails at runtime.
*/
function _createForOfIteratorHelper(o,allowArrayLike){var it;if(typeof Symbol==="undefined"||o[Symbol.iterator]==null){if(Array.isArray(o)||(it=_unsupportedIterableToArray(o))||allowArrayLike&&o&&typeof o.length==="number"){if(it)o=it;var i=0;var F=function F(){};return{s:F,n:function n(){if(i>=o.length)return{done:true};return{done:false,value:o[i++]}},e:function e(_e8){throw _e8},f:F}}throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}var normalCompletion=true,didErr=false,err;return{s:function s(){it=o[Symbol.iterator]()},n:function n(){var step=it.next();normalCompletion=step.done;return step},e:function e(_e9){didErr=true;err=_e9},f:function f(){try{if(!normalCompletion&&it["return"]!=null)it["return"]()}finally{if(didErr)throw err}}}}function _slicedToArray(arr,i){return _arrayWithHoles(arr)||_iterableToArrayLimit(arr,i)||_unsupportedIterableToArray(arr,i)||_nonIterableRest()}function _nonIterableRest(){throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}function _iterableToArrayLimit(arr,i){if(typeof Symbol==="undefined"||!(Symbol.iterator in Object(arr)))return;var _arr=[];var _n=true;var _d=false;var _e=undefined;try{for(var _i=arr[Symbol.iterator](),_s;!(_n=(_s=_i.next()).done);_n=true){_arr.push(_s.value);if(i&&_arr.length===i)break}}catch(err){_d=true;_e=err}finally{try{if(!_n&&_i["return"]!=null)_i["return"]()}finally{if(_d)throw _e}}return _arr}function _arrayWithHoles(arr){if(Array.isArray(arr))return arr}function _toConsumableArray(arr){return _arrayWithoutHoles(arr)||_iterableToArray(arr)||_unsupportedIterableToArray(arr)||_nonIterableSpread()}function _nonIterableSpread(){throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.")}function _unsupportedIterableToArray(o,minLen){if(!o)return;if(typeof o==="string")return _arrayLikeToArray(o,minLen);var n=Object.prototype.toString.call(o).slice(8,-1);if(n==="Object"&&o.constructor)n=o.constructor.name;if(n==="Map"||n==="Set")return Array.from(o);if(n==="Arguments"||/^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n))return _arrayLikeToArray(o,minLen)}function _iterableToArray(iter){if(typeof Symbol!=="undefined"&&Symbol.iterator in Object(iter))return Array.from(iter)}function _arrayWithoutHoles(arr){if(Array.isArray(arr))return _arrayLikeToArray(arr)}function _arrayLikeToArray(arr,len){if(len==null||len>arr.length)len=arr.length;for(var i=0,arr2=new Array(len);i<len;i++){arr2[i]=arr[i]}return arr2}function _inherits(subClass,superClass){if(typeof superClass!=="function"&&superClass!==null){throw new TypeError("Super expression must either be null or a function")}subClass.prototype=Object.create(superClass&&superClass.prototype,{constructor:{value:subClass,writable:true,configurable:true}});if(superClass)_setPrototypeOf(subClass,superClass)}function _setPrototypeOf(o,p){_setPrototypeOf=Object.setPrototypeOf||function _setPrototypeOf(o,p){o.__proto__=p;return o};return _setPrototypeOf(o,p)}function _createSuper(Derived){var hasNativeReflectConstruct=_isNativeReflectConstruct();return function _createSuperInternal(){var Super=_getPrototypeOf(Derived),result;if(hasNativeReflectConstruct){var NewTarget=_getPrototypeOf(this).constructor;result=Reflect.construct(Super,arguments,NewTarget)}else{result=Super.apply(this,arguments)}return _possibleConstructorReturn(this,result)}}function _possibleConstructorReturn(self,call){if(call&&(_typeof(call)==="object"||typeof call==="function")){return call}return _assertThisInitialized(self)}function _assertThisInitialized(self){if(self===void 0){throw new ReferenceError("this hasn't been initialised - super() hasn't been called")}return self}function _isNativeReflectConstruct(){if(typeof Reflect==="undefined"||!Reflect.construct)return false;if(Reflect.construct.sham)return false;if(typeof Proxy==="function")return true;try{Date.prototype.toString.call(Reflect.construct(Date,[],function(){}));return true}catch(e){return false}}function _getPrototypeOf(o){_getPrototypeOf=Object.setPrototypeOf?Object.getPrototypeOf:function _getPrototypeOf(o){return o.__proto__||Object.getPrototypeOf(o)};return _getPrototypeOf(o)}function _classCallCheck(instance,Constructor){if(!(instance instanceof Constructor)){throw new TypeError("Cannot call a class as a function")}}function _defineProperties(target,props){for(var i=0;i<props.length;i++){var descriptor=props[i];descriptor.enumerable=descriptor.enumerable||false;descriptor.configurable=true;if("value"in descriptor)descriptor.writable=true;Object.defineProperty(target,descriptor.key,descriptor)}}function _createClass(Constructor,protoProps,staticProps){if(protoProps)_defineProperties(Constructor.prototype,protoProps);if(staticProps)_defineProperties(Constructor,staticProps);return Constructor}function _typeof(obj){"@babel/helpers - typeof";if(typeof Symbol==="function"&&typeof Symbol.iterator==="symbol"){_typeof=function _typeof(obj){return typeof obj}}else{_typeof=function _typeof(obj){return obj&&typeof Symbol==="function"&&obj.constructor===Symbol&&obj!==Symbol.prototype?"symbol":typeof obj}}return _typeof(obj)}var hljs=function(){"use strict";function e(t){return t instanceof Map?t.clear=t["delete"]=t.set=function(){throw Error("map is read-only")}:t instanceof Set&&(t.add=t.clear=t["delete"]=function(){throw Error("set is read-only")}),Object.freeze(t),Object.getOwnPropertyNames(t).forEach(function(n){var s=t[n];"object"!=_typeof(s)||Object.isFrozen(s)||e(s)}),t}var t=e,n=e;t["default"]=n;var s=function(){function s(e){_classCallCheck(this,s);void 0===e.data&&(e.data={}),this.data=e.data}_createClass(s,[{key:"ignoreMatch",value:function ignoreMatch(){this.ignore=!0}}]);return s}();function r(e){return e.replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;").replace(/'/g,"&#x27;")}function a(e){var n=Object.create(null);for(var _t in e){n[_t]=e[_t]}for(var _len=arguments.length,t=new Array(_len>1?_len-1:0),_key=1;_key<_len;_key++){t[_key-1]=arguments[_key]}return t.forEach(function(e){for(var _t2 in e){n[_t2]=e[_t2]}}),n}var i=function i(e){return!!e.kind};var o=function(){function o(e,t){_classCallCheck(this,o);this.buffer="",this.classPrefix=t.classPrefix,e.walk(this)}_createClass(o,[{key:"addText",value:function addText(e){this.buffer+=r(e)}},{key:"openNode",value:function openNode(e){if(!i(e))return;var t=e.kind;e.sublanguage||(t="".concat(this.classPrefix).concat(t)),this.span(t)}},{key:"closeNode",value:function closeNode(e){i(e)&&(this.buffer+="</span>")}},{key:"value",value:function value(){return this.buffer}},{key:"span",value:function span(e){this.buffer+='<span class="'.concat(e,'">')}}]);return o}();var l=function(){function l(){_classCallCheck(this,l);this.rootNode={children:[]},this.stack=[this.rootNode]}_createClass(l,[{key:"add",value:function add(e){this.top.children.push(e)}},{key:"openNode",value:function openNode(e){var t={kind:e,children:[]};this.add(t),this.stack.push(t)}},{key:"closeNode",value:function closeNode(){if(this.stack.length>1)return this.stack.pop()}},{key:"closeAllNodes",value:function closeAllNodes(){for(;this.closeNode();){}}},{key:"toJSON",value:function toJSON(){return JSON.stringify(this.rootNode,null,4)}},{key:"walk",value:function walk(e){return this.constructor._walk(e,this.rootNode)}},{key:"top",get:function get(){return this.stack[this.stack.length-1]}},{key:"root",get:function get(){return this.rootNode}}],[{key:"_walk",value:function _walk(e,t){var _this=this;return"string"==typeof t?e.addText(t):t.children&&(e.openNode(t),t.children.forEach(function(t){return _this._walk(e,t)}),e.closeNode(t)),e}},{key:"_collapse",value:function _collapse(e){"string"!=typeof e&&e.children&&(e.children.every(function(e){return"string"==typeof e})?e.children=[e.children.join("")]:e.children.forEach(function(e){l._collapse(e)}))}}]);return l}();var c=function(_l){_inherits(c,_l);var _super=_createSuper(c);function c(e){var _this2;_classCallCheck(this,c);_this2=_super.call(this),_this2.options=e;return _this2}_createClass(c,[{key:"addKeyword",value:function addKeyword(e,t){""!==e&&(this.openNode(t),this.addText(e),this.closeNode())}},{key:"addText",value:function addText(e){""!==e&&this.add(e)}},{key:"addSublanguage",value:function addSublanguage(e,t){var n=e.root;n.kind=t,n.sublanguage=!0,this.add(n)}},{key:"toHTML",value:function toHTML(){return new o(this,this.options).value()}},{key:"finalize",value:function finalize(){return!0}}]);return c}(l);function u(e){return e?"string"==typeof e?e:e.source:null}var g="[a-zA-Z]\\w*",d="[a-zA-Z_]\\w*",h="\\b\\d+(\\.\\d+)?",f="(-?)(\\b0[xX][a-fA-F0-9]+|(\\b\\d+(\\.\\d*)?|\\.\\d+)([eE][-+]?\\d+)?)",p="\\b(0b[01]+)",m={begin:"\\\\[\\s\\S]",relevance:0},b={className:"string",begin:"'",end:"'",illegal:"\\n",contains:[m]},x={className:"string",begin:'"',end:'"',illegal:"\\n",contains:[m]},E={begin:/\b(a|an|the|are|I'm|isn't|don't|doesn't|won't|but|just|should|pretty|simply|enough|gonna|going|wtf|so|such|will|you|your|they|like|more)\b/},v=function v(e,t){var n=arguments.length>2&&arguments[2]!==undefined?arguments[2]:{};var s=a({className:"comment",begin:e,end:t,contains:[]},n);return s.contains.push(E),s.contains.push({className:"doctag",begin:"(?:TODO|FIXME|NOTE|BUG|OPTIMIZE|HACK|XXX):",relevance:0}),s},N=v("//","$"),w=v("/\\*","\\*/"),R=v("#","$");var y=Object.freeze({__proto__:null,IDENT_RE:g,UNDERSCORE_IDENT_RE:d,NUMBER_RE:h,C_NUMBER_RE:f,BINARY_NUMBER_RE:p,RE_STARTERS_RE:"!|!=|!==|%|%=|&|&&|&=|\\*|\\*=|\\+|\\+=|,|-|-=|/=|/|:|;|<<|<<=|<=|<|===|==|=|>>>=|>>=|>=|>>>|>>|>|\\?|\\[|\\{|\\(|\\^|\\^=|\\||\\|=|\\|\\||~",SHEBANG:function SHEBANG(){var e=arguments.length>0&&arguments[0]!==undefined?arguments[0]:{};var t=/^#![ ]*\//;return e.binary&&(e.begin=function(){for(var _len2=arguments.length,e=new Array(_len2),_key2=0;_key2<_len2;_key2++){e[_key2]=arguments[_key2]}return e.map(function(e){return u(e)}).join("")}(t,/.*\b/,e.binary,/\b.*/)),a({className:"meta",begin:t,end:/$/,relevance:0,"on:begin":function onBegin(e,t){0!==e.index&&t.ignoreMatch()}},e)},BACKSLASH_ESCAPE:m,APOS_STRING_MODE:b,QUOTE_STRING_MODE:x,PHRASAL_WORDS_MODE:E,COMMENT:v,C_LINE_COMMENT_MODE:N,C_BLOCK_COMMENT_MODE:w,HASH_COMMENT_MODE:R,NUMBER_MODE:{className:"number",begin:h,relevance:0},C_NUMBER_MODE:{className:"number",begin:f,relevance:0},BINARY_NUMBER_MODE:{className:"number",begin:p,relevance:0},CSS_NUMBER_MODE:{className:"number",begin:h+"(%|em|ex|ch|rem|vw|vh|vmin|vmax|cm|mm|in|pt|pc|px|deg|grad|rad|turn|s|ms|Hz|kHz|dpi|dpcm|dppx)?",relevance:0},REGEXP_MODE:{begin:/(?=\/[^/\n]*\/)/,contains:[{className:"regexp",begin:/\//,end:/\/[gimuy]*/,illegal:/\n/,contains:[m,{begin:/\[/,end:/\]/,relevance:0,contains:[m]}]}]},TITLE_MODE:{className:"title",begin:g,relevance:0},UNDERSCORE_TITLE_MODE:{className:"title",begin:d,relevance:0},METHOD_GUARD:{begin:"\\.\\s*[a-zA-Z_]\\w*",relevance:0},END_SAME_AS_BEGIN:function END_SAME_AS_BEGIN(e){return Object.assign(e,{"on:begin":function onBegin(e,t){t.data._beginMatch=e[1]},"on:end":function onEnd(e,t){t.data._beginMatch!==e[1]&&t.ignoreMatch()}})}});function _(e,t){"."===e.input[e.index-1]&&t.ignoreMatch()}function k(e,t){t&&e.beginKeywords&&(e.begin="\\b("+e.beginKeywords.split(" ").join("|")+")(?!\\.)(?=\\b|\\s)",e.__beforeBegin=_,e.keywords=e.keywords||e.beginKeywords,delete e.beginKeywords)}function M(e,t){Array.isArray(e.illegal)&&(e.illegal=function(){for(var _len3=arguments.length,e=new Array(_len3),_key3=0;_key3<_len3;_key3++){e[_key3]=arguments[_key3]}return"("+e.map(function(e){return u(e)}).join("|")+")"}.apply(void 0,_toConsumableArray(e.illegal)))}function O(e,t){if(e.match){if(e.begin||e.end)throw Error("begin & end are not supported with match");e.begin=e.match,delete e.match}}function A(e,t){void 0===e.relevance&&(e.relevance=1)}var L=["of","and","for","in","not","or","if","then","parent","list","value"];function B(e,t){return t?Number(t):function(e){return L.includes(e.toLowerCase())}(e)?0:1}function I(e,_ref){var t=_ref.plugins;function n(t,n){return RegExp(u(t),"m"+(e.case_insensitive?"i":"")+(n?"g":""))}var s=function(){function s(){_classCallCheck(this,s);this.matchIndexes={},this.regexes=[],this.matchAt=1,this.position=0}_createClass(s,[{key:"addRule",value:function addRule(e,t){t.position=this.position++,this.matchIndexes[this.matchAt]=t,this.regexes.push([t,e]),this.matchAt+=function(e){return RegExp(e.toString()+"|").exec("").length-1}(e)+1}},{key:"compile",value:function compile(){0===this.regexes.length&&(this.exec=function(){return null});var e=this.regexes.map(function(e){return e[1]});this.matcherRe=n(function(e){var t=arguments.length>1&&arguments[1]!==undefined?arguments[1]:"|";var n=/\[(?:[^\\\]]|\\.)*\]|\(\??|\\([1-9][0-9]*)|\\./;var s=0,r="";for(var _a=0;_a<e.length;_a++){s+=1;var _i=s;var _o=u(e[_a]);for(_a>0&&(r+=t),r+="(";_o.length>0;){var _e=n.exec(_o);if(null==_e){r+=_o;break}r+=_o.substring(0,_e.index),_o=_o.substring(_e.index+_e[0].length),"\\"===_e[0][0]&&_e[1]?r+="\\"+(Number(_e[1])+_i):(r+=_e[0],"("===_e[0]&&s++)}r+=")"}return r}(e),!0),this.lastIndex=0}},{key:"exec",value:function exec(e){this.matcherRe.lastIndex=this.lastIndex;var t=this.matcherRe.exec(e);if(!t)return null;var n=t.findIndex(function(e,t){return t>0&&void 0!==e}),s=this.matchIndexes[n];return t.splice(0,n),Object.assign(t,s)}}]);return s}();var r=function(){function r(){_classCallCheck(this,r);this.rules=[],this.multiRegexes=[],this.count=0,this.lastIndex=0,this.regexIndex=0}_createClass(r,[{key:"getMatcher",value:function getMatcher(e){if(this.multiRegexes[e])return this.multiRegexes[e];var t=new s;return this.rules.slice(e).forEach(function(_ref2){var _ref3=_slicedToArray(_ref2,2),e=_ref3[0],n=_ref3[1];return t.addRule(e,n)}),t.compile(),this.multiRegexes[e]=t,t}},{key:"resumingScanAtSamePosition",value:function resumingScanAtSamePosition(){return 0!==this.regexIndex}},{key:"considerAll",value:function considerAll(){this.regexIndex=0}},{key:"addRule",value:function addRule(e,t){this.rules.push([e,t]),"begin"===t.type&&this.count++}},{key:"exec",value:function exec(e){var t=this.getMatcher(this.regexIndex);t.lastIndex=this.lastIndex;var n=t.exec(e);if(this.resumingScanAtSamePosition())if(n&&n.index===this.lastIndex);else{var _t3=this.getMatcher(0);_t3.lastIndex=this.lastIndex+1,n=_t3.exec(e)}return n&&(this.regexIndex+=n.position+1,this.regexIndex===this.count&&this.considerAll()),n}}]);return r}();if(e.compilerExtensions||(e.compilerExtensions=[]),e.contains&&e.contains.includes("self"))throw Error("ERR: contains `self` is not supported at the top-level of a language.  See documentation.");return e.classNameAliases=a(e.classNameAliases||{}),function t(s,i){var _ref4;var o=s;if(s.compiled)return o;[O].forEach(function(e){return e(s,i)}),e.compilerExtensions.forEach(function(e){return e(s,i)}),s.__beforeBegin=null,[k,M,A].forEach(function(e){return e(s,i)}),s.compiled=!0;var l=null;if("object"==_typeof(s.keywords)&&(l=s.keywords.$pattern,delete s.keywords.$pattern),s.keywords&&(s.keywords=function(e,t){var n={};return"string"==typeof e?s("keyword",e):Object.keys(e).forEach(function(t){s(t,e[t])}),n;function s(e,s){t&&(s=s.toLowerCase()),s.split(" ").forEach(function(t){var s=t.split("|");n[s[0]]=[e,B(s[0],s[1])]})}}(s.keywords,e.case_insensitive)),s.lexemes&&l)throw Error("ERR: Prefer `keywords.$pattern` to `mode.lexemes`, BOTH are not allowed. (see mode reference) ");return l=l||s.lexemes||/\w+/,o.keywordPatternRe=n(l,!0),i&&(s.begin||(s.begin=/\B|\b/),o.beginRe=n(s.begin),s.endSameAsBegin&&(s.end=s.begin),s.end||s.endsWithParent||(s.end=/\B|\b/),s.end&&(o.endRe=n(s.end)),o.terminatorEnd=u(s.end)||"",s.endsWithParent&&i.terminatorEnd&&(o.terminatorEnd+=(s.end?"|":"")+i.terminatorEnd)),s.illegal&&(o.illegalRe=n(s.illegal)),s.contains||(s.contains=[]),s.contains=(_ref4=[]).concat.apply(_ref4,_toConsumableArray(s.contains.map(function(e){return function(e){return e.variants&&!e.cachedVariants&&(e.cachedVariants=e.variants.map(function(t){return a(e,{variants:null},t)})),e.cachedVariants?e.cachedVariants:T(e)?a(e,{starts:e.starts?a(e.starts):null}):Object.isFrozen(e)?a(e):e}("self"===e?s:e)}))),s.contains.forEach(function(e){t(e,o)}),s.starts&&t(s.starts,i),o.matcher=function(e){var t=new r;return e.contains.forEach(function(e){return t.addRule(e.begin,{rule:e,type:"begin"})}),e.terminatorEnd&&t.addRule(e.terminatorEnd,{type:"end"}),e.illegal&&t.addRule(e.illegal,{type:"illegal"}),t}(o),o}(e)}function T(e){return!!e&&(e.endsWithParent||T(e.starts))}function j(e){var t={props:["language","code","autodetect"],data:function data(){return{detectedLanguage:"",unknownLanguage:!1}},computed:{className:function className(){return this.unknownLanguage?"":"hljs "+this.detectedLanguage},highlighted:function highlighted(){if(!this.autoDetect&&!e.getLanguage(this.language))return console.warn('The language "'.concat(this.language,'" you specified could not be found.')),this.unknownLanguage=!0,r(this.code);var t={};return this.autoDetect?(t=e.highlightAuto(this.code),this.detectedLanguage=t.language):(t=e.highlight(this.language,this.code,this.ignoreIllegals),this.detectedLanguage=this.language),t.value},autoDetect:function autoDetect(){return!(this.language&&(e=this.autodetect,!e&&""!==e));var e},ignoreIllegals:function ignoreIllegals(){return!0}},render:function render(e){return e("pre",{},[e("code",{class:this.className,domProps:{innerHTML:this.highlighted}})])}};return{Component:t,VuePlugin:{install:function install(e){e.component("highlightjs",t)}}}}var S={"after:highlightBlock":function afterHighlightBlock(_ref5){var e=_ref5.block,t=_ref5.result,n=_ref5.text;var s=D(e);if(!s.length)return;var a=document.createElement("div");a.innerHTML=t.value,t.value=function(e,t,n){var s=0,a="";var i=[];function o(){return e.length&&t.length?e[0].offset!==t[0].offset?e[0].offset<t[0].offset?e:t:"start"===t[0].event?e:t:e.length?e:t}function l(e){a+="<"+P(e)+[].map.call(e.attributes,function(e){return" "+e.nodeName+'="'+r(e.value)+'"'}).join("")+">"}function c(e){a+="</"+P(e)+">"}function u(e){("start"===e.event?l:c)(e.node)}for(;e.length||t.length;){var _t4=o();if(a+=r(n.substring(s,_t4[0].offset)),s=_t4[0].offset,_t4===e){i.reverse().forEach(c);do{u(_t4.splice(0,1)[0]),_t4=o()}while(_t4===e&&_t4.length&&_t4[0].offset===s);i.reverse().forEach(l)}else"start"===_t4[0].event?i.push(_t4[0].node):i.pop(),u(_t4.splice(0,1)[0])}return a+r(n.substr(s))}(s,D(a),n)}};function P(e){return e.nodeName.toLowerCase()}function D(e){var t=[];return function e(n,s){for(var _r=n.firstChild;_r;_r=_r.nextSibling){3===_r.nodeType?s+=_r.nodeValue.length:1===_r.nodeType&&(t.push({event:"start",offset:s,node:_r}),s=e(_r,s),P(_r).match(/br|hr|img|input/)||t.push({event:"stop",offset:s,node:_r}))}return s}(e,0),t}var C=function C(e){console.error(e)},H=function H(e){var _console;for(var _len4=arguments.length,t=new Array(_len4>1?_len4-1:0),_key4=1;_key4<_len4;_key4++){t[_key4-1]=arguments[_key4]}(_console=console).log.apply(_console,["WARN: "+e].concat(t))},$=function $(e,t){console.log("Deprecated as of ".concat(e,". ").concat(t))},U=r,z=a,K=Symbol("nomatch");return function(e){var n=Object.create(null),r=Object.create(null),a=[];var i=!0;var o=/(^(<[^>]+>|\t|)+|\n)/gm,l="Could not find the language '{}', did you forget to load/include a language module?",u={disableAutodetect:!0,name:"Plain text",contains:[]};var g={noHighlightRe:/^(no-?highlight)$/i,languageDetectRe:/\blang(?:uage)?-([\w-]+)\b/i,classPrefix:"hljs-",tabReplace:null,useBR:!1,languages:null,__emitter:c};function d(e){return g.noHighlightRe.test(e)}function h(e,t,n,s){var r={code:t,language:e};_("before:highlight",r);var a=r.result?r.result:f(r.language,r.code,n,s);return a.code=r.code,_("after:highlight",a),a}function f(e,t,r,o){var c=t;function u(e,t){var n=w.case_insensitive?t[0].toLowerCase():t[0];return Object.prototype.hasOwnProperty.call(e.keywords,n)&&e.keywords[n]}function d(){null!=_.subLanguage?function(){if(""===O)return;var e=null;if("string"==typeof _.subLanguage){if(!n[_.subLanguage])return void M.addText(O);e=f(_.subLanguage,O,!0,k[_.subLanguage]),k[_.subLanguage]=e.top}else e=p(O,_.subLanguage.length?_.subLanguage:null);_.relevance>0&&(A+=e.relevance),M.addSublanguage(e.emitter,e.language)}():function(){if(!_.keywords)return void M.addText(O);var e=0;_.keywordPatternRe.lastIndex=0;var t=_.keywordPatternRe.exec(O),n="";for(;t;){n+=O.substring(e,t.index);var _s2=u(_,t);if(_s2){var _s3=_slicedToArray(_s2,2),_e2=_s3[0],_r2=_s3[1];M.addText(n),n="",A+=_r2;var _a2=w.classNameAliases[_e2]||_e2;M.addKeyword(t[0],_a2)}else n+=t[0];e=_.keywordPatternRe.lastIndex,t=_.keywordPatternRe.exec(O)}n+=O.substr(e),M.addText(n)}(),O=""}function h(e){return e.className&&M.openNode(w.classNameAliases[e.className]||e.className),_=Object.create(e,{parent:{value:_}}),_}function m(e,t,n){var r=function(e,t){var n=e&&e.exec(t);return n&&0===n.index}(e.endRe,n);if(r){if(e["on:end"]){var _n2=new s(e);e["on:end"](t,_n2),_n2.ignore&&(r=!1)}if(r){for(;e.endsParent&&e.parent;){e=e.parent}return e}}if(e.endsWithParent)return m(e.parent,t,n)}function b(e){return 0===_.matcher.regexIndex?(O+=e[0],1):(T=!0,0)}function x(e){var t=e[0],n=c.substr(e.index),s=m(_,e,n);if(!s)return K;var r=_;r.skip?O+=t:(r.returnEnd||r.excludeEnd||(O+=t),d(),r.excludeEnd&&(O=t));do{_.className&&M.closeNode(),_.skip||_.subLanguage||(A+=_.relevance),_=_.parent}while(_!==s.parent);return s.starts&&(s.endSameAsBegin&&(s.starts.endRe=s.endRe),h(s.starts)),r.returnEnd?0:t.length}var E={};function v(t,n){var a=n&&n[0];if(O+=t,null==a)return d(),0;if("begin"===E.type&&"end"===n.type&&E.index===n.index&&""===a){if(O+=c.slice(n.index,n.index+1),!i){var _t5=Error("0 width match regex");throw _t5.languageName=e,_t5.badRule=E.rule,_t5}return 1}if(E=n,"begin"===n.type)return function(e){var t=e[0],n=e.rule,r=new s(n),a=[n.__beforeBegin,n["on:begin"]];for(var _i2=0,_a3=a;_i2<_a3.length;_i2++){var _n3=_a3[_i2];if(_n3&&(_n3(e,r),r.ignore))return b(t)}return n&&n.endSameAsBegin&&(n.endRe=RegExp(t.replace(/[-/\\^$*+?.()|\[\]{}]/g,"\\$&"),"m")),n.skip?O+=t:(n.excludeBegin&&(O+=t),d(),n.returnBegin||n.excludeBegin||(O=t)),h(n),n.returnBegin?0:t.length}(n);if("illegal"===n.type&&!r){var _e3=Error('Illegal lexeme "'+a+'" for mode "'+(_.className||"<unnamed>")+'"');throw _e3.mode=_,_e3}if("end"===n.type){var _e4=x(n);if(_e4!==K)return _e4}if("illegal"===n.type&&""===a)return 1;if(B>1e5&&B>3*n.index)throw Error("potential infinite loop, way more iterations than matches");return O+=a,a.length}var w=N(e);if(!w)throw C(l.replace("{}",e)),Error('Unknown language: "'+e+'"');var R=I(w,{plugins:a});var y="",_=o||R;var k={},M=new g.__emitter(g);(function(){var e=[];for(var _t6=_;_t6!==w;_t6=_t6.parent){_t6.className&&e.unshift(_t6.className)}e.forEach(function(e){return M.openNode(e)})})();var O="",A=0,L=0,B=0,T=!1;try{for(_.matcher.considerAll();;){B++,T?T=!1:_.matcher.considerAll(),_.matcher.lastIndex=L;var _e5=_.matcher.exec(c);if(!_e5)break;var _t7=v(c.substring(L,_e5.index),_e5);L=_e5.index+_t7}return v(c.substr(L)),M.closeAllNodes(),M.finalize(),y=M.toHTML(),{relevance:A,value:y,language:e,illegal:!1,emitter:M,top:_}}catch(t){if(t.message&&t.message.includes("Illegal"))return{illegal:!0,illegalBy:{msg:t.message,context:c.slice(L-100,L+100),mode:t.mode},sofar:y,relevance:0,value:U(c),emitter:M};if(i)return{illegal:!1,relevance:0,value:U(c),emitter:M,language:e,top:_,errorRaised:t};throw t}}function p(e,t){t=t||g.languages||Object.keys(n);var s=function(e){var t={relevance:0,emitter:new g.__emitter(g),value:U(e),illegal:!1,top:u};return t.emitter.addText(e),t}(e),r=t.filter(N).filter(R).map(function(t){return f(t,e,!1)});r.unshift(s);var a=r.sort(function(e,t){if(e.relevance!==t.relevance)return t.relevance-e.relevance;if(e.language&&t.language){if(N(e.language).supersetOf===t.language)return 1;if(N(t.language).supersetOf===e.language)return-1}return 0}),_a4=_slicedToArray(a,2),i=_a4[0],o=_a4[1],l=i;return l.second_best=o,l}var m={"before:highlightBlock":function beforeHighlightBlock(_ref6){var e=_ref6.block;g.useBR&&(e.innerHTML=e.innerHTML.replace(/\n/g,"").replace(/<br[ /]*>/g,"\n"))},"after:highlightBlock":function afterHighlightBlock(_ref7){var e=_ref7.result;g.useBR&&(e.value=e.value.replace(/\n/g,"<br>"))}},b=/^(<[^>]+>|\t)+/gm,x={"after:highlightBlock":function afterHighlightBlock(_ref8){var e=_ref8.result;g.tabReplace&&(e.value=e.value.replace(b,function(e){return e.replace(/\t/g,g.tabReplace)}))}};function E(e){var t=null;var n=function(e){var t=e.className+" ";t+=e.parentNode?e.parentNode.className:"";var n=g.languageDetectRe.exec(t);if(n){var _t8=N(n[1]);return _t8||(H(l.replace("{}",n[1])),H("Falling back to no-highlight mode for this block.",e)),_t8?n[1]:"no-highlight"}return t.split(/\s+/).find(function(e){return d(e)||N(e)})}(e);if(d(n))return;_("before:highlightBlock",{block:e,language:n}),t=e;var s=t.textContent,a=n?h(n,s,!0):p(s);_("after:highlightBlock",{block:e,result:a,text:s}),e.innerHTML=a.value,function(e,t,n){var s=t?r[t]:n;e.classList.add("hljs"),s&&e.classList.add(s)}(e,n,a.language),e.result={language:a.language,re:a.relevance,relavance:a.relevance},a.second_best&&(e.second_best={language:a.second_best.language,re:a.second_best.relevance,relavance:a.second_best.relevance})}var v=function v(){v.called||(v.called=!0,document.querySelectorAll("pre code").forEach(E))};function N(e){return e=(e||"").toLowerCase(),n[e]||n[r[e]]}function w(e,_ref9){var t=_ref9.languageName;"string"==typeof e&&(e=[e]),e.forEach(function(e){r[e]=t})}function R(e){var t=N(e);return t&&!t.disableAutodetect}function _(e,t){var n=e;a.forEach(function(e){e[n]&&e[n](t)})}Object.assign(e,{highlight:h,highlightAuto:p,fixMarkup:function fixMarkup(e){return $("10.2.0","fixMarkup will be removed entirely in v11.0"),$("10.2.0","Please see https://github.com/highlightjs/highlight.js/issues/2534"),t=e,g.tabReplace||g.useBR?t.replace(o,function(e){return"\n"===e?g.useBR?"<br>":e:g.tabReplace?e.replace(/\t/g,g.tabReplace):e}):t;var t},highlightBlock:E,configure:function configure(e){e.useBR&&($("10.3.0","'useBR' will be removed entirely in v11.0"),$("10.3.0","Please see https://github.com/highlightjs/highlight.js/issues/2559")),g=z(g,e)},initHighlighting:v,initHighlightingOnLoad:function initHighlightingOnLoad(){window.addEventListener("DOMContentLoaded",v,!1)},registerLanguage:function registerLanguage(t,s){var r=null;try{r=s(e)}catch(e){if(C("Language definition for '{}' could not be registered.".replace("{}",t)),!i)throw e;C(e),r=u}r.name||(r.name=t),n[t]=r,r.rawDefinition=s.bind(null,e),r.aliases&&w(r.aliases,{languageName:t})},listLanguages:function listLanguages(){return Object.keys(n)},getLanguage:N,registerAliases:w,requireLanguage:function requireLanguage(e){$("10.4.0","requireLanguage will be removed entirely in v11."),$("10.4.0","Please see https://github.com/highlightjs/highlight.js/pull/2844");var t=N(e);if(t)return t;throw Error("The '{}' language is required, but not loaded.".replace("{}",e))},autoDetection:R,inherit:z,addPlugin:function addPlugin(e){a.push(e)},vuePlugin:j(e).VuePlugin}),e.debugMode=function(){i=!1},e.safeMode=function(){i=!0},e.versionString="10.5.0";for(var _e6 in y){"object"==_typeof(y[_e6])&&t(y[_e6])}return Object.assign(e,y),e.addPlugin(m),e.addPlugin(S),e.addPlugin(x),e}({})}();"object"==(typeof exports==="undefined"?"undefined":_typeof(exports))&&"undefined"!=typeof module&&(module.exports=hljs);hljs.registerLanguage("bash",function(){"use strict";function e(){for(var _len5=arguments.length,e=new Array(_len5),_key5=0;_key5<_len5;_key5++){e[_key5]=arguments[_key5]}return e.map(function(e){return(s=e)?"string"==typeof s?s:s.source:null;var s}).join("")}return function(s){var n={},t={begin:/\$\{/,end:/\}/,contains:["self",{begin:/:-/,contains:[n]}]};Object.assign(n,{className:"variable",variants:[{begin:e(/\$[\w\d#@][\w\d_]*/,"(?![\\w\\d])(?![$])")},t]});var a={className:"subst",begin:/\$\(/,end:/\)/,contains:[s.BACKSLASH_ESCAPE]},i={begin:/<<-?\s*(?=\w+)/,starts:{contains:[s.END_SAME_AS_BEGIN({begin:/(\w+)/,end:/(\w+)/,className:"string"})]}},c={className:"string",begin:/"/,end:/"/,contains:[s.BACKSLASH_ESCAPE,n,a]};a.contains.push(c);var o={begin:/\$\(\(/,end:/\)\)/,contains:[{begin:/\d+#[0-9a-f]+/,className:"number"},s.NUMBER_MODE,n]},r=s.SHEBANG({binary:"(fish|bash|zsh|sh|csh|ksh|tcsh|dash|scsh)",relevance:10}),l={className:"function",begin:/\w[\w\d_]*\s*\(\s*\)\s*\{/,returnBegin:!0,contains:[s.inherit(s.TITLE_MODE,{begin:/\w[\w\d_]*/})],relevance:0};return{name:"Bash",aliases:["sh","zsh"],keywords:{$pattern:/\b[a-z._-]+\b/,keyword:"if then else elif fi for while in do done case esac function",literal:"true false",built_in:"break cd continue eval exec exit export getopts hash pwd readonly return shift test times trap umask unset alias bind builtin caller command declare echo enable help let local logout mapfile printf read readarray source type typeset ulimit unalias set shopt autoload bg bindkey bye cap chdir clone comparguments compcall compctl compdescribe compfiles compgroups compquote comptags comptry compvalues dirs disable disown echotc echoti emulate fc fg float functions getcap getln history integer jobs kill limit log noglob popd print pushd pushln rehash sched setcap setopt stat suspend ttyctl unfunction unhash unlimit unsetopt vared wait whence where which zcompile zformat zftp zle zmodload zparseopts zprof zpty zregexparse zsocket zstyle ztcp"},contains:[r,s.SHEBANG(),l,o,s.HASH_COMMENT_MODE,i,c,{className:"",begin:/\\"/},{className:"string",begin:/'/,end:/'/},n]}}}());hljs.registerLanguage("csharp",function(){"use strict";return function(e){var n={keyword:["abstract","as","base","break","case","class","const","continue","do","else","event","explicit","extern","finally","fixed","for","foreach","goto","if","implicit","in","interface","internal","is","lock","namespace","new","operator","out","override","params","private","protected","public","readonly","record","ref","return","sealed","sizeof","stackalloc","static","struct","switch","this","throw","try","typeof","unchecked","unsafe","using","virtual","void","volatile","while"].concat(["add","alias","and","ascending","async","await","by","descending","equals","from","get","global","group","init","into","join","let","nameof","not","notnull","on","or","orderby","partial","remove","select","set","unmanaged","value|0","var","when","where","with","yield"]).join(" "),built_in:"bool byte char decimal delegate double dynamic enum float int long nint nuint object sbyte short string ulong unit ushort",literal:"default false null true"},a=e.inherit(e.TITLE_MODE,{begin:"[a-zA-Z](\\.?\\w)*"}),i={className:"number",variants:[{begin:"\\b(0b[01']+)"},{begin:"(-?)\\b([\\d']+(\\.[\\d']*)?|\\.[\\d']+)(u|U|l|L|ul|UL|f|F|b|B)"},{begin:"(-?)(\\b0[xX][a-fA-F0-9']+|(\\b[\\d']+(\\.[\\d']*)?|\\.[\\d']+)([eE][-+]?[\\d']+)?)"}],relevance:0},s={className:"string",begin:'@"',end:'"',contains:[{begin:'""'}]},t=e.inherit(s,{illegal:/\n/}),r={className:"subst",begin:/\{/,end:/\}/,keywords:n},l=e.inherit(r,{illegal:/\n/}),c={className:"string",begin:/\$"/,end:'"',illegal:/\n/,contains:[{begin:/\{\{/},{begin:/\}\}/},e.BACKSLASH_ESCAPE,l]},o={className:"string",begin:/\$@"/,end:'"',contains:[{begin:/\{\{/},{begin:/\}\}/},{begin:'""'},r]},d=e.inherit(o,{illegal:/\n/,contains:[{begin:/\{\{/},{begin:/\}\}/},{begin:'""'},l]});r.contains=[o,c,s,e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,i,e.C_BLOCK_COMMENT_MODE],l.contains=[d,c,t,e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,i,e.inherit(e.C_BLOCK_COMMENT_MODE,{illegal:/\n/})];var g={variants:[o,c,s,e.APOS_STRING_MODE,e.QUOTE_STRING_MODE]},E={begin:"<",end:">",contains:[{beginKeywords:"in out"},a]},_=e.IDENT_RE+"(<"+e.IDENT_RE+"(\\s*,\\s*"+e.IDENT_RE+")*>)?(\\[\\])?",b={begin:"@"+e.IDENT_RE,relevance:0};return{name:"C#",aliases:["cs","c#"],keywords:n,illegal:/::/,contains:[e.COMMENT("///","$",{returnBegin:!0,contains:[{className:"doctag",variants:[{begin:"///",relevance:0},{begin:"\x3c!--|--\x3e"},{begin:"</?",end:">"}]}]}),e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE,{className:"meta",begin:"#",end:"$",keywords:{"meta-keyword":"if else elif endif define undef warning error line region endregion pragma checksum"}},g,i,{beginKeywords:"class interface",relevance:0,end:/[{;=]/,illegal:/[^\s:,]/,contains:[{beginKeywords:"where class"},a,E,e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},{beginKeywords:"namespace",relevance:0,end:/[{;=]/,illegal:/[^\s:]/,contains:[a,e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},{beginKeywords:"record",relevance:0,end:/[{;=]/,illegal:/[^\s:]/,contains:[a,E,e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},{className:"meta",begin:"^\\s*\\[",excludeBegin:!0,end:"\\]",excludeEnd:!0,contains:[{className:"meta-string",begin:/"/,end:/"/}]},{beginKeywords:"new return throw await else",relevance:0},{className:"function",begin:"("+_+"\\s+)+"+e.IDENT_RE+"\\s*(<.+>\\s*)?\\(",returnBegin:!0,end:/\s*[{;=]/,excludeEnd:!0,keywords:n,contains:[{beginKeywords:"public private protected static internal protected abstract async extern override unsafe virtual new sealed partial",relevance:0},{begin:e.IDENT_RE+"\\s*(<.+>\\s*)?\\(",returnBegin:!0,contains:[e.TITLE_MODE,E],relevance:0},{className:"params",begin:/\(/,end:/\)/,excludeBegin:!0,excludeEnd:!0,keywords:n,relevance:0,contains:[g,i,e.C_BLOCK_COMMENT_MODE]},e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},b]}}}());hljs.registerLanguage("c-like",function(){"use strict";function e(e){return function(){for(var _len6=arguments.length,e=new Array(_len6),_key6=0;_key6<_len6;_key6++){e[_key6]=arguments[_key6]}return e.map(function(e){return function(e){return e?"string"==typeof e?e:e.source:null}(e)}).join("")}("(",e,")?")}return function(t){var n=t.COMMENT("//","$",{contains:[{begin:/\\\n/}]}),r="[a-zA-Z_]\\w*::",a="(decltype\\(auto\\)|"+e(r)+"[a-zA-Z_]\\w*"+e("<[^<>]+>")+")",i={className:"keyword",begin:"\\b[a-z\\d_]*_t\\b"},s={className:"string",variants:[{begin:'(u8?|U|L)?"',end:'"',illegal:"\\n",contains:[t.BACKSLASH_ESCAPE]},{begin:"(u8?|U|L)?'(\\\\(x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4,8}|[0-7]{3}|\\S)|.)",end:"'",illegal:"."},t.END_SAME_AS_BEGIN({begin:/(?:u8?|U|L)?R"([^()\\ ]{0,16})\(/,end:/\)([^()\\ ]{0,16})"/})]},o={className:"number",variants:[{begin:"\\b(0b[01']+)"},{begin:"(-?)\\b([\\d']+(\\.[\\d']*)?|\\.[\\d']+)((ll|LL|l|L)(u|U)?|(u|U)(ll|LL|l|L)?|f|F|b|B)"},{begin:"(-?)(\\b0[xX][a-fA-F0-9']+|(\\b[\\d']+(\\.[\\d']*)?|\\.[\\d']+)([eE][-+]?[\\d']+)?)"}],relevance:0},c={className:"meta",begin:/#\s*[a-z]+\b/,end:/$/,keywords:{"meta-keyword":"if else elif endif define undef warning error line pragma _Pragma ifdef ifndef include"},contains:[{begin:/\\\n/,relevance:0},t.inherit(s,{className:"meta-string"}),{className:"meta-string",begin:/<.*?>/,end:/$/,illegal:"\\n"},n,t.C_BLOCK_COMMENT_MODE]},l={className:"title",begin:e(r)+t.IDENT_RE,relevance:0},d=e(r)+t.IDENT_RE+"\\s*\\(",u={keyword:"int float while private char char8_t char16_t char32_t catch import module export virtual operator sizeof dynamic_cast|10 typedef const_cast|10 const for static_cast|10 union namespace unsigned long volatile static protected bool template mutable if public friend do goto auto void enum else break extern using asm case typeid wchar_t short reinterpret_cast|10 default double register explicit signed typename try this switch continue inline delete alignas alignof constexpr consteval constinit decltype concept co_await co_return co_yield requires noexcept static_assert thread_local restrict final override atomic_bool atomic_char atomic_schar atomic_uchar atomic_short atomic_ushort atomic_int atomic_uint atomic_long atomic_ulong atomic_llong atomic_ullong new throw return and and_eq bitand bitor compl not not_eq or or_eq xor xor_eq",built_in:"std string wstring cin cout cerr clog stdin stdout stderr stringstream istringstream ostringstream auto_ptr deque list queue stack vector map set pair bitset multiset multimap unordered_set unordered_map unordered_multiset unordered_multimap priority_queue make_pair array shared_ptr abort terminate abs acos asin atan2 atan calloc ceil cosh cos exit exp fabs floor fmod fprintf fputs free frexp fscanf future isalnum isalpha iscntrl isdigit isgraph islower isprint ispunct isspace isupper isxdigit tolower toupper labs ldexp log10 log malloc realloc memchr memcmp memcpy memset modf pow printf putchar puts scanf sinh sin snprintf sprintf sqrt sscanf strcat strchr strcmp strcpy strcspn strlen strncat strncmp strncpy strpbrk strrchr strspn strstr tanh tan vfprintf vprintf vsprintf endl initializer_list unique_ptr _Bool complex _Complex imaginary _Imaginary",literal:"true false nullptr NULL"},m=[c,i,n,t.C_BLOCK_COMMENT_MODE,o,s],p={variants:[{begin:/=/,end:/;/},{begin:/\(/,end:/\)/},{beginKeywords:"new throw return else",end:/;/}],keywords:u,contains:m.concat([{begin:/\(/,end:/\)/,keywords:u,contains:m.concat(["self"]),relevance:0}]),relevance:0},_={className:"function",begin:"("+a+"[\\*&\\s]+)+"+d,returnBegin:!0,end:/[{;=]/,excludeEnd:!0,keywords:u,illegal:/[^\w\s\*&:<>.]/,contains:[{begin:"decltype\\(auto\\)",keywords:u,relevance:0},{begin:d,returnBegin:!0,contains:[l],relevance:0},{className:"params",begin:/\(/,end:/\)/,keywords:u,relevance:0,contains:[n,t.C_BLOCK_COMMENT_MODE,s,o,i,{begin:/\(/,end:/\)/,keywords:u,relevance:0,contains:["self",n,t.C_BLOCK_COMMENT_MODE,s,o,i]}]},i,n,t.C_BLOCK_COMMENT_MODE,c]};return{aliases:["c","cc","h","c++","h++","hpp","hh","hxx","cxx"],keywords:u,disableAutodetect:!0,illegal:"</",contains:[].concat(p,_,m,[c,{begin:"\\b(deque|list|queue|priority_queue|pair|stack|vector|map|set|bitset|multiset|multimap|unordered_map|unordered_set|unordered_multiset|unordered_multimap|array)\\s*<",end:">",keywords:u,contains:["self",i]},{begin:t.IDENT_RE+"::",keywords:u},{className:"class",beginKeywords:"enum class struct union",end:/[{;:<>=]/,contains:[{beginKeywords:"final class struct"},t.TITLE_MODE]}]),exports:{preprocessor:c,strings:s,keywords:u}}}}());hljs.registerLanguage("cpp",function(){"use strict";function e(e){return function(){for(var _len7=arguments.length,e=new Array(_len7),_key7=0;_key7<_len7;_key7++){e[_key7]=arguments[_key7]}return e.map(function(e){return function(e){return e?"string"==typeof e?e:e.source:null}(e)}).join("")}("(",e,")?")}return function(t){var n=function(t){var n=t.COMMENT("//","$",{contains:[{begin:/\\\n/}]}),r="[a-zA-Z_]\\w*::",a="(decltype\\(auto\\)|"+e(r)+"[a-zA-Z_]\\w*"+e("<[^<>]+>")+")",s={className:"keyword",begin:"\\b[a-z\\d_]*_t\\b"},i={className:"string",variants:[{begin:'(u8?|U|L)?"',end:'"',illegal:"\\n",contains:[t.BACKSLASH_ESCAPE]},{begin:"(u8?|U|L)?'(\\\\(x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4,8}|[0-7]{3}|\\S)|.)",end:"'",illegal:"."},t.END_SAME_AS_BEGIN({begin:/(?:u8?|U|L)?R"([^()\\ ]{0,16})\(/,end:/\)([^()\\ ]{0,16})"/})]},c={className:"number",variants:[{begin:"\\b(0b[01']+)"},{begin:"(-?)\\b([\\d']+(\\.[\\d']*)?|\\.[\\d']+)((ll|LL|l|L)(u|U)?|(u|U)(ll|LL|l|L)?|f|F|b|B)"},{begin:"(-?)(\\b0[xX][a-fA-F0-9']+|(\\b[\\d']+(\\.[\\d']*)?|\\.[\\d']+)([eE][-+]?[\\d']+)?)"}],relevance:0},o={className:"meta",begin:/#\s*[a-z]+\b/,end:/$/,keywords:{"meta-keyword":"if else elif endif define undef warning error line pragma _Pragma ifdef ifndef include"},contains:[{begin:/\\\n/,relevance:0},t.inherit(i,{className:"meta-string"}),{className:"meta-string",begin:/<.*?>/,end:/$/,illegal:"\\n"},n,t.C_BLOCK_COMMENT_MODE]},l={className:"title",begin:e(r)+t.IDENT_RE,relevance:0},d=e(r)+t.IDENT_RE+"\\s*\\(",u={keyword:"int float while private char char8_t char16_t char32_t catch import module export virtual operator sizeof dynamic_cast|10 typedef const_cast|10 const for static_cast|10 union namespace unsigned long volatile static protected bool template mutable if public friend do goto auto void enum else break extern using asm case typeid wchar_t short reinterpret_cast|10 default double register explicit signed typename try this switch continue inline delete alignas alignof constexpr consteval constinit decltype concept co_await co_return co_yield requires noexcept static_assert thread_local restrict final override atomic_bool atomic_char atomic_schar atomic_uchar atomic_short atomic_ushort atomic_int atomic_uint atomic_long atomic_ulong atomic_llong atomic_ullong new throw return and and_eq bitand bitor compl not not_eq or or_eq xor xor_eq",built_in:"std string wstring cin cout cerr clog stdin stdout stderr stringstream istringstream ostringstream auto_ptr deque list queue stack vector map set pair bitset multiset multimap unordered_set unordered_map unordered_multiset unordered_multimap priority_queue make_pair array shared_ptr abort terminate abs acos asin atan2 atan calloc ceil cosh cos exit exp fabs floor fmod fprintf fputs free frexp fscanf future isalnum isalpha iscntrl isdigit isgraph islower isprint ispunct isspace isupper isxdigit tolower toupper labs ldexp log10 log malloc realloc memchr memcmp memcpy memset modf pow printf putchar puts scanf sinh sin snprintf sprintf sqrt sscanf strcat strchr strcmp strcpy strcspn strlen strncat strncmp strncpy strpbrk strrchr strspn strstr tanh tan vfprintf vprintf vsprintf endl initializer_list unique_ptr _Bool complex _Complex imaginary _Imaginary",literal:"true false nullptr NULL"},p=[o,s,n,t.C_BLOCK_COMMENT_MODE,c,i],m={variants:[{begin:/=/,end:/;/},{begin:/\(/,end:/\)/},{beginKeywords:"new throw return else",end:/;/}],keywords:u,contains:p.concat([{begin:/\(/,end:/\)/,keywords:u,contains:p.concat(["self"]),relevance:0}]),relevance:0},_={className:"function",begin:"("+a+"[\\*&\\s]+)+"+d,returnBegin:!0,end:/[{;=]/,excludeEnd:!0,keywords:u,illegal:/[^\w\s\*&:<>.]/,contains:[{begin:"decltype\\(auto\\)",keywords:u,relevance:0},{begin:d,returnBegin:!0,contains:[l],relevance:0},{className:"params",begin:/\(/,end:/\)/,keywords:u,relevance:0,contains:[n,t.C_BLOCK_COMMENT_MODE,i,c,s,{begin:/\(/,end:/\)/,keywords:u,relevance:0,contains:["self",n,t.C_BLOCK_COMMENT_MODE,i,c,s]}]},s,n,t.C_BLOCK_COMMENT_MODE,o]};return{aliases:["c","cc","h","c++","h++","hpp","hh","hxx","cxx"],keywords:u,disableAutodetect:!0,illegal:"</",contains:[].concat(m,_,p,[o,{begin:"\\b(deque|list|queue|priority_queue|pair|stack|vector|map|set|bitset|multiset|multimap|unordered_map|unordered_set|unordered_multiset|unordered_multimap|array)\\s*<",end:">",keywords:u,contains:["self",s]},{begin:t.IDENT_RE+"::",keywords:u},{className:"class",beginKeywords:"enum class struct union",end:/[{;:<>=]/,contains:[{beginKeywords:"final class struct"},t.TITLE_MODE]}]),exports:{preprocessor:o,strings:i,keywords:u}}}(t);return n.disableAutodetect=!1,n.name="C++",n.aliases=["cc","c++","h++","hpp","hh","hxx","cxx"],n}}());hljs.registerLanguage("css",function(){"use strict";return function(e){var n="[a-zA-Z-][a-zA-Z0-9_-]*",a={begin:/([*]\s?)?(?:[A-Z_.\-\\]+|--[a-zA-Z0-9_-]+)\s*(\/\*\*\/)?:/,returnBegin:!0,end:";",endsWithParent:!0,contains:[{begin:/-(webkit|moz|ms|o)-/},{className:"attribute",begin:/\S/,end:":",excludeEnd:!0,starts:{endsWithParent:!0,excludeEnd:!0,contains:[{begin:/[\w-]+\(/,returnBegin:!0,contains:[{className:"built_in",begin:/[\w-]+/},{begin:/\(/,end:/\)/,contains:[e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,e.CSS_NUMBER_MODE]}]},e.CSS_NUMBER_MODE,e.QUOTE_STRING_MODE,e.APOS_STRING_MODE,e.C_BLOCK_COMMENT_MODE,{className:"number",begin:"#[0-9A-Fa-f]+"},{className:"meta",begin:"!important"}]}}]};return{name:"CSS",case_insensitive:!0,illegal:/[=|'\$]/,contains:[e.C_BLOCK_COMMENT_MODE,{className:"selector-id",begin:/#[A-Za-z0-9_-]+/},{className:"selector-class",begin:"\\."+n},{className:"selector-attr",begin:/\[/,end:/\]/,illegal:"$",contains:[e.APOS_STRING_MODE,e.QUOTE_STRING_MODE]},{className:"selector-pseudo",begin:/:(:)?[a-zA-Z0-9_+"'.-]+/},{begin:/\(/,end:/\)/,contains:[e.CSS_NUMBER_MODE]},{begin:"@(page|font-face)",lexemes:"@[a-z-]+",keywords:"@page @font-face"},{begin:"@",end:"[{;]",illegal:/:/,returnBegin:!0,contains:[{className:"keyword",begin:/@-?\w[\w]*(-\w+)*/},{begin:/\s/,endsWithParent:!0,excludeEnd:!0,relevance:0,keywords:"and or not only",contains:[{begin:/[a-z-]+:/,className:"attribute"},e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,e.CSS_NUMBER_MODE]}]},{className:"selector-tag",begin:n,relevance:0},{begin:/\{/,end:/\}/,illegal:/\S/,contains:[e.C_BLOCK_COMMENT_MODE,{begin:/;/},a]}]}}}());hljs.registerLanguage("coffeescript",function(){"use strict";var e=["as","in","of","if","for","while","finally","var","new","function","do","return","void","else","break","catch","instanceof","with","throw","case","default","try","switch","continue","typeof","delete","let","yield","const","class","debugger","async","await","static","import","from","export","extends"],n=["true","false","null","undefined","NaN","Infinity"],a=[].concat(["setInterval","setTimeout","clearInterval","clearTimeout","require","exports","eval","isFinite","isNaN","parseFloat","parseInt","decodeURI","decodeURIComponent","encodeURI","encodeURIComponent","escape","unescape"],["arguments","this","super","console","window","document","localStorage","module","global"],["Intl","DataView","Number","Math","Date","String","RegExp","Object","Function","Boolean","Error","Symbol","Set","Map","WeakSet","WeakMap","Proxy","Reflect","JSON","Promise","Float64Array","Int16Array","Int32Array","Int8Array","Uint16Array","Uint32Array","Float32Array","Array","Uint8Array","Uint8ClampedArray","ArrayBuffer"],["EvalError","InternalError","RangeError","ReferenceError","SyntaxError","TypeError","URIError"]);return function(r){var t={keyword:e.concat(["then","unless","until","loop","by","when","and","or","is","isnt","not"]).filter((i=["var","const","let","function","static"],function(e){return!i.includes(e)})).join(" "),literal:n.concat(["yes","no","on","off"]).join(" "),built_in:a.concat(["npm","print"]).join(" ")};var i;var s="[A-Za-z$_][0-9A-Za-z$_]*",o={className:"subst",begin:/#\{/,end:/\}/,keywords:t},c=[r.BINARY_NUMBER_MODE,r.inherit(r.C_NUMBER_MODE,{starts:{end:"(\\s*/)?",relevance:0}}),{className:"string",variants:[{begin:/'''/,end:/'''/,contains:[r.BACKSLASH_ESCAPE]},{begin:/'/,end:/'/,contains:[r.BACKSLASH_ESCAPE]},{begin:/"""/,end:/"""/,contains:[r.BACKSLASH_ESCAPE,o]},{begin:/"/,end:/"/,contains:[r.BACKSLASH_ESCAPE,o]}]},{className:"regexp",variants:[{begin:"///",end:"///",contains:[o,r.HASH_COMMENT_MODE]},{begin:"//[gim]{0,3}(?=\\W)",relevance:0},{begin:/\/(?![ *]).*?(?![\\]).\/[gim]{0,3}(?=\W)/}]},{begin:"@"+s},{subLanguage:"javascript",excludeBegin:!0,excludeEnd:!0,variants:[{begin:"```",end:"```"},{begin:"`",end:"`"}]}];o.contains=c;var l=r.inherit(r.TITLE_MODE,{begin:s}),d="(\\(.*\\)\\s*)?\\B[-=]>",g={className:"params",begin:"\\([^\\(]",returnBegin:!0,contains:[{begin:/\(/,end:/\)/,keywords:t,contains:["self"].concat(c)}]};return{name:"CoffeeScript",aliases:["coffee","cson","iced"],keywords:t,illegal:/\/\*/,contains:c.concat([r.COMMENT("###","###"),r.HASH_COMMENT_MODE,{className:"function",begin:"^\\s*"+s+"\\s*=\\s*"+d,end:"[-=]>",returnBegin:!0,contains:[l,g]},{begin:/[:\(,=]\s*/,relevance:0,contains:[{className:"function",begin:d,end:"[-=]>",returnBegin:!0,contains:[g]}]},{className:"class",beginKeywords:"class",end:"$",illegal:/[:="\[\]]/,contains:[{beginKeywords:"extends",endsWithParent:!0,illegal:/[:="\[\]]/,contains:[l]},l]},{begin:s+":",end:":",returnBegin:!0,returnEnd:!0,relevance:0}])}}}());hljs.registerLanguage("xml",function(){"use strict";function e(e){return e?"string"==typeof e?e:e.source:null}function n(e){return a("(?=",e,")")}function a(){for(var _len8=arguments.length,n=new Array(_len8),_key8=0;_key8<_len8;_key8++){n[_key8]=arguments[_key8]}return n.map(function(n){return e(n)}).join("")}function s(){for(var _len9=arguments.length,n=new Array(_len9),_key9=0;_key9<_len9;_key9++){n[_key9]=arguments[_key9]}return"("+n.map(function(n){return e(n)}).join("|")+")"}return function(e){var t=a(/[A-Z_]/,a("(",/[A-Z0-9_.-]+:/,")?"),/[A-Z0-9_.-]*/),i={className:"symbol",begin:/&[a-z]+;|&#[0-9]+;|&#x[a-f0-9]+;/},r={begin:/\s/,contains:[{className:"meta-keyword",begin:/#?[a-z_][a-z1-9_-]+/,illegal:/\n/}]},c=e.inherit(r,{begin:/\(/,end:/\)/}),l=e.inherit(e.APOS_STRING_MODE,{className:"meta-string"}),g=e.inherit(e.QUOTE_STRING_MODE,{className:"meta-string"}),m={endsWithParent:!0,illegal:/</,relevance:0,contains:[{className:"attr",begin:/[A-Za-z0-9._:-]+/,relevance:0},{begin:/=\s*/,relevance:0,contains:[{className:"string",endsParent:!0,variants:[{begin:/"/,end:/"/,contains:[i]},{begin:/'/,end:/'/,contains:[i]},{begin:/[^\s"'=<>`]+/}]}]}]};return{name:"HTML, XML",aliases:["html","xhtml","rss","atom","xjb","xsd","xsl","plist","wsf","svg"],case_insensitive:!0,contains:[{className:"meta",begin:/<![a-z]/,end:/>/,relevance:10,contains:[r,g,l,c,{begin:/\[/,end:/\]/,contains:[{className:"meta",begin:/<![a-z]/,end:/>/,contains:[r,c,g,l]}]}]},e.COMMENT(/<!--/,/-->/,{relevance:10}),{begin:/<!\[CDATA\[/,end:/\]\]>/,relevance:10},i,{className:"meta",begin:/<\?xml/,end:/\?>/,relevance:10},{className:"tag",begin:/<style(?=\s|>)/,end:/>/,keywords:{name:"style"},contains:[m],starts:{end:/<\/style>/,returnEnd:!0,subLanguage:["css","xml"]}},{className:"tag",begin:/<script(?=\s|>)/,end:/>/,keywords:{name:"script"},contains:[m],starts:{end:/<\/script>/,returnEnd:!0,subLanguage:["javascript","handlebars","xml"]}},{className:"tag",begin:/<>|<\/>/},{className:"tag",begin:a(/</,n(a(t,s(/\/>/,/>/,/\s/)))),end:/\/?>/,contains:[{className:"name",begin:t,relevance:0,starts:m}]},{className:"tag",begin:a(/<\//,n(a(t,/>/))),contains:[{className:"name",begin:t,relevance:0},{begin:/>/,relevance:0}]}]}}}());hljs.registerLanguage("http",function(){"use strict";function e(){for(var _len10=arguments.length,e=new Array(_len10),_key10=0;_key10<_len10;_key10++){e[_key10]=arguments[_key10]}return e.map(function(e){return(n=e)?"string"==typeof n?n:n.source:null;var n}).join("")}return function(n){var a="HTTP/(2|1\\.[01])",s=[{className:"attribute",begin:e("^",/[A-Za-z][A-Za-z0-9-]*/,"(?=\\:\\s)"),starts:{contains:[{className:"punctuation",begin:/: /,relevance:0,starts:{end:"$",relevance:0}}]}},{begin:"\\n\\n",starts:{subLanguage:[],endsWithParent:!0}}];return{name:"HTTP",aliases:["https"],illegal:/\S/,contains:[{begin:"^(?="+a+" \\d{3})",end:/$/,contains:[{className:"meta",begin:a},{className:"number",begin:"\\b\\d{3}\\b"}],starts:{end:/\b\B/,illegal:/\S/,contains:s}},{begin:"(?=^[A-Z]+ (.*?) "+a+"$)",end:/$/,contains:[{className:"string",begin:" ",end:" ",excludeBegin:!0,excludeEnd:!0},{className:"meta",begin:a},{className:"keyword",begin:"[A-Z]+"}],starts:{end:/\b\B/,illegal:/\S/,contains:s}}]}}}());hljs.registerLanguage("json",function(){"use strict";return function(n){var e={literal:"true false null"},i=[n.C_LINE_COMMENT_MODE,n.C_BLOCK_COMMENT_MODE],a=[n.QUOTE_STRING_MODE,n.C_NUMBER_MODE],l={end:",",endsWithParent:!0,excludeEnd:!0,contains:a,keywords:e},t={begin:/\{/,end:/\}/,contains:[{className:"attr",begin:/"/,end:/"/,contains:[n.BACKSLASH_ESCAPE],illegal:"\\n"},n.inherit(l,{begin:/:/})].concat(i),illegal:"\\S"},s={begin:"\\[",end:"\\]",contains:[n.inherit(l)],illegal:"\\S"};return a.push(t,s),i.forEach(function(n){a.push(n)}),{name:"JSON",contains:a,keywords:e,illegal:"\\S"}}}());hljs.registerLanguage("java",function(){"use strict";var e="\\.([0-9](_*[0-9])*)",n="[0-9a-fA-F](_*[0-9a-fA-F])*",a={className:"number",variants:[{begin:"(\\b([0-9](_*[0-9])*)((".concat(e,")|\\.)?|(").concat(e,"))[eE][+-]?([0-9](_*[0-9])*)[fFdD]?\\b")},{begin:"\\b([0-9](_*[0-9])*)((".concat(e,")[fFdD]?\\b|\\.([fFdD]\\b)?)")},{begin:"(".concat(e,")[fFdD]?\\b")},{begin:"\\b([0-9](_*[0-9])*)[fFdD]\\b"},{begin:"\\b0[xX]((".concat(n,")\\.?|(").concat(n,")?\\.(").concat(n,"))[pP][+-]?([0-9](_*[0-9])*)[fFdD]?\\b")},{begin:"\\b(0|[1-9](_*[0-9])*)[lL]?\\b"},{begin:"\\b0[xX](".concat(n,")[lL]?\\b")},{begin:"\\b0(_*[0-7])*[lL]?\\b"},{begin:"\\b0[bB][01](_*[01])*[lL]?\\b"}],relevance:0};return function(e){var n="false synchronized int abstract float private char boolean var static null if const for true while long strictfp finally protected import native final void enum else break transient catch instanceof byte super volatile case assert short package default double public try this switch continue throws protected public private module requires exports do",s={className:"meta",begin:"@[À-ʸa-zA-Z_$][À-ʸa-zA-Z_$0-9]*",contains:[{begin:/\(/,end:/\)/,contains:["self"]}]};var r=a;return{name:"Java",aliases:["jsp"],keywords:n,illegal:/<\/|#/,contains:[e.COMMENT("/\\*\\*","\\*/",{relevance:0,contains:[{begin:/\w+@/,relevance:0},{className:"doctag",begin:"@[A-Za-z]+"}]}),{begin:/import java\.[a-z]+\./,keywords:"import",relevance:2},e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE,e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,{className:"class",beginKeywords:"class interface enum",end:/[{;=]/,excludeEnd:!0,keywords:"class interface enum",illegal:/[:"\[\]]/,contains:[{beginKeywords:"extends implements"},e.UNDERSCORE_TITLE_MODE]},{beginKeywords:"new throw return else",relevance:0},{className:"class",begin:"record\\s+"+e.UNDERSCORE_IDENT_RE+"\\s*\\(",returnBegin:!0,excludeEnd:!0,end:/[{;=]/,keywords:n,contains:[{beginKeywords:"record"},{begin:e.UNDERSCORE_IDENT_RE+"\\s*\\(",returnBegin:!0,relevance:0,contains:[e.UNDERSCORE_TITLE_MODE]},{className:"params",begin:/\(/,end:/\)/,keywords:n,relevance:0,contains:[e.C_BLOCK_COMMENT_MODE]},e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},{className:"function",begin:"([À-ʸa-zA-Z_$][À-ʸa-zA-Z_$0-9]*(<[À-ʸa-zA-Z_$][À-ʸa-zA-Z_$0-9]*(\\s*,\\s*[À-ʸa-zA-Z_$][À-ʸa-zA-Z_$0-9]*)*>)?\\s+)+"+e.UNDERSCORE_IDENT_RE+"\\s*\\(",returnBegin:!0,end:/[{;=]/,excludeEnd:!0,keywords:n,contains:[{begin:e.UNDERSCORE_IDENT_RE+"\\s*\\(",returnBegin:!0,relevance:0,contains:[e.UNDERSCORE_TITLE_MODE]},{className:"params",begin:/\(/,end:/\)/,keywords:n,relevance:0,contains:[s,e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,r,e.C_BLOCK_COMMENT_MODE]},e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},r,s]}}}());hljs.registerLanguage("javascript",function(){"use strict";var e="[A-Za-z$_][0-9A-Za-z$_]*",n=["as","in","of","if","for","while","finally","var","new","function","do","return","void","else","break","catch","instanceof","with","throw","case","default","try","switch","continue","typeof","delete","let","yield","const","class","debugger","async","await","static","import","from","export","extends"],a=["true","false","null","undefined","NaN","Infinity"],s=[].concat(["setInterval","setTimeout","clearInterval","clearTimeout","require","exports","eval","isFinite","isNaN","parseFloat","parseInt","decodeURI","decodeURIComponent","encodeURI","encodeURIComponent","escape","unescape"],["arguments","this","super","console","window","document","localStorage","module","global"],["Intl","DataView","Number","Math","Date","String","RegExp","Object","Function","Boolean","Error","Symbol","Set","Map","WeakSet","WeakMap","Proxy","Reflect","JSON","Promise","Float64Array","Int16Array","Int32Array","Int8Array","Uint16Array","Uint32Array","Float32Array","Array","Uint8Array","Uint8ClampedArray","ArrayBuffer"],["EvalError","InternalError","RangeError","ReferenceError","SyntaxError","TypeError","URIError"]);function r(e){return i("(?=",e,")")}function i(){for(var _len11=arguments.length,e=new Array(_len11),_key11=0;_key11<_len11;_key11++){e[_key11]=arguments[_key11]}return e.map(function(e){return(n=e)?"string"==typeof n?n:n.source:null;var n}).join("")}return function(t){var c=e,o={begin:/<[A-Za-z0-9\\._:-]+/,end:/\/[A-Za-z0-9\\._:-]+>|\/>/,isTrulyOpeningTag:function isTrulyOpeningTag(e,n){var a=e[0].length+e.index,s=e.input[a];"<"!==s?">"===s&&(function(e,_ref10){var n=_ref10.after;var a="</"+e[0].slice(1);return-1!==e.input.indexOf(a,n)}(e,{after:a})||n.ignoreMatch()):n.ignoreMatch()}},l={$pattern:e,keyword:n.join(" "),literal:a.join(" "),built_in:s.join(" ")},b="\\.([0-9](_?[0-9])*)",g="0|[1-9](_?[0-9])*|0[0-7]*[89][0-9]*",d={className:"number",variants:[{begin:"(\\b(".concat(g,")((").concat(b,")|\\.)?|(").concat(b,"))[eE][+-]?([0-9](_?[0-9])*)\\b")},{begin:"\\b(".concat(g,")\\b((").concat(b,")\\b|\\.)?|(").concat(b,")\\b")},{begin:"\\b(0|[1-9](_?[0-9])*)n\\b"},{begin:"\\b0[xX][0-9a-fA-F](_?[0-9a-fA-F])*n?\\b"},{begin:"\\b0[bB][0-1](_?[0-1])*n?\\b"},{begin:"\\b0[oO][0-7](_?[0-7])*n?\\b"},{begin:"\\b0[0-7]+n?\\b"}],relevance:0},E={className:"subst",begin:"\\$\\{",end:"\\}",keywords:l,contains:[]},u={begin:"html`",end:"",starts:{end:"`",returnEnd:!1,contains:[t.BACKSLASH_ESCAPE,E],subLanguage:"xml"}},_={begin:"css`",end:"",starts:{end:"`",returnEnd:!1,contains:[t.BACKSLASH_ESCAPE,E],subLanguage:"css"}},m={className:"string",begin:"`",end:"`",contains:[t.BACKSLASH_ESCAPE,E]},N={className:"comment",variants:[t.COMMENT(/\/\*\*(?!\/)/,"\\*/",{relevance:0,contains:[{className:"doctag",begin:"@[A-Za-z]+",contains:[{className:"type",begin:"\\{",end:"\\}",relevance:0},{className:"variable",begin:c+"(?=\\s*(-)|$)",endsParent:!0,relevance:0},{begin:/(?=[^\n])\s/,relevance:0}]}]}),t.C_BLOCK_COMMENT_MODE,t.C_LINE_COMMENT_MODE]},y=[t.APOS_STRING_MODE,t.QUOTE_STRING_MODE,u,_,m,d,t.REGEXP_MODE];E.contains=y.concat({begin:/\{/,end:/\}/,keywords:l,contains:["self"].concat(y)});var f=[].concat(N,E.contains),A=f.concat([{begin:/\(/,end:/\)/,keywords:l,contains:["self"].concat(f)}]),p={className:"params",begin:/\(/,end:/\)/,excludeBegin:!0,excludeEnd:!0,keywords:l,contains:A};return{name:"Javascript",aliases:["js","jsx","mjs","cjs"],keywords:l,exports:{PARAMS_CONTAINS:A},illegal:/#(?![$_A-z])/,contains:[t.SHEBANG({label:"shebang",binary:"node",relevance:5}),{label:"use_strict",className:"meta",relevance:10,begin:/^\s*['"]use (strict|asm)['"]/},t.APOS_STRING_MODE,t.QUOTE_STRING_MODE,u,_,m,N,d,{begin:i(/[{,\n]\s*/,r(i(/(((\/\/.*$)|(\/\*(\*[^/]|[^*])*\*\/))\s*)*/,c+"\\s*:"))),relevance:0,contains:[{className:"attr",begin:c+r("\\s*:"),relevance:0}]},{begin:"("+t.RE_STARTERS_RE+"|\\b(case|return|throw)\\b)\\s*",keywords:"return throw case",contains:[N,t.REGEXP_MODE,{className:"function",begin:"(\\([^()]*(\\([^()]*(\\([^()]*\\)[^()]*)*\\)[^()]*)*\\)|"+t.UNDERSCORE_IDENT_RE+")\\s*=>",returnBegin:!0,end:"\\s*=>",contains:[{className:"params",variants:[{begin:t.UNDERSCORE_IDENT_RE,relevance:0},{className:null,begin:/\(\s*\)/,skip:!0},{begin:/\(/,end:/\)/,excludeBegin:!0,excludeEnd:!0,keywords:l,contains:A}]}]},{begin:/,/,relevance:0},{className:"",begin:/\s/,end:/\s*/,skip:!0},{variants:[{begin:"<>",end:"</>"},{begin:o.begin,"on:begin":o.isTrulyOpeningTag,end:o.end}],subLanguage:"xml",contains:[{begin:o.begin,end:o.end,skip:!0,contains:["self"]}]}],relevance:0},{className:"function",beginKeywords:"function",end:/[{;]/,excludeEnd:!0,keywords:l,contains:["self",t.inherit(t.TITLE_MODE,{begin:c}),p],illegal:/%/},{beginKeywords:"while if switch catch for"},{className:"function",begin:t.UNDERSCORE_IDENT_RE+"\\([^()]*(\\([^()]*(\\([^()]*\\)[^()]*)*\\)[^()]*)*\\)\\s*\\{",returnBegin:!0,contains:[p,t.inherit(t.TITLE_MODE,{begin:c})]},{variants:[{begin:"\\."+c},{begin:"\\$"+c}],relevance:0},{className:"class",beginKeywords:"class",end:/[{;=]/,excludeEnd:!0,illegal:/[:"[\]]/,contains:[{beginKeywords:"extends"},t.UNDERSCORE_TITLE_MODE]},{begin:/\b(?=constructor)/,end:/[{;]/,excludeEnd:!0,contains:[t.inherit(t.TITLE_MODE,{begin:c}),"self",p]},{begin:"(get|set)\\s+(?="+c+"\\()",end:/\{/,keywords:"get set",contains:[t.inherit(t.TITLE_MODE,{begin:c}),{begin:/\(\)/},p]},{begin:/\$[(.]/}]}}}());hljs.registerLanguage("makefile",function(){"use strict";return function(e){var i={className:"variable",variants:[{begin:"\\$\\("+e.UNDERSCORE_IDENT_RE+"\\)",contains:[e.BACKSLASH_ESCAPE]},{begin:/\$[@%<?\^\+\*]/}]},a={className:"string",begin:/"/,end:/"/,contains:[e.BACKSLASH_ESCAPE,i]},n={className:"variable",begin:/\$\([\w-]+\s/,end:/\)/,keywords:{built_in:"subst patsubst strip findstring filter filter-out sort word wordlist firstword lastword dir notdir suffix basename addsuffix addprefix join wildcard realpath abspath error warning shell origin flavor foreach if or and call eval file value"},contains:[i]},s={begin:"^"+e.UNDERSCORE_IDENT_RE+"\\s*(?=[:+?]?=)"},r={className:"section",begin:/^[^\s]+:/,end:/$/,contains:[i]};return{name:"Makefile",aliases:["mk","mak","make"],keywords:{$pattern:/[\w-]+/,keyword:"define endef undefine ifdef ifndef ifeq ifneq else endif include -include sinclude override export unexport private vpath"},contains:[e.HASH_COMMENT_MODE,i,a,n,s,{className:"meta",begin:/^\.PHONY:/,end:/$/,keywords:{$pattern:/[\.\w]+/,"meta-keyword":".PHONY"}},r]}}}());hljs.registerLanguage("markdown",function(){"use strict";function n(){for(var _len12=arguments.length,n=new Array(_len12),_key12=0;_key12<_len12;_key12++){n[_key12]=arguments[_key12]}return n.map(function(n){return(e=n)?"string"==typeof e?e:e.source:null;var e}).join("")}return function(e){var a={begin:/<\/?[A-Za-z_]/,end:">",subLanguage:"xml",relevance:0},i={variants:[{begin:/\[.+?\]\[.*?\]/,relevance:0},{begin:/\[.+?\]\(((data|javascript|mailto):|(?:http|ftp)s?:\/\/).*?\)/,relevance:2},{begin:n(/\[.+?\]\(/,/[A-Za-z][A-Za-z0-9+.-]*/,/:\/\/.*?\)/),relevance:2},{begin:/\[.+?\]\([./?&#].*?\)/,relevance:1},{begin:/\[.+?\]\(.*?\)/,relevance:0}],returnBegin:!0,contains:[{className:"string",relevance:0,begin:"\\[",end:"\\]",excludeBegin:!0,returnEnd:!0},{className:"link",relevance:0,begin:"\\]\\(",end:"\\)",excludeBegin:!0,excludeEnd:!0},{className:"symbol",relevance:0,begin:"\\]\\[",end:"\\]",excludeBegin:!0,excludeEnd:!0}]},s={className:"strong",contains:[],variants:[{begin:/_{2}/,end:/_{2}/},{begin:/\*{2}/,end:/\*{2}/}]},c={className:"emphasis",contains:[],variants:[{begin:/\*(?!\*)/,end:/\*/},{begin:/_(?!_)/,end:/_/,relevance:0}]};s.contains.push(c),c.contains.push(s);var t=[a,i];return s.contains=s.contains.concat(t),c.contains=c.contains.concat(t),t=t.concat(s,c),{name:"Markdown",aliases:["md","mkdown","mkd"],contains:[{className:"section",variants:[{begin:"^#{1,6}",end:"$",contains:t},{begin:"(?=^.+?\\n[=-]{2,}$)",contains:[{begin:"^[=-]*$"},{begin:"^",end:"\\n",contains:t}]}]},a,{className:"bullet",begin:"^[ \t]*([*+-]|(\\d+\\.))(?=\\s+)",end:"\\s+",excludeEnd:!0},s,c,{className:"quote",begin:"^>\\s+",contains:t,end:"$"},{className:"code",variants:[{begin:"(`{3,})[^`](.|\\n)*?\\1`*[ ]*"},{begin:"(~{3,})[^~](.|\\n)*?\\1~*[ ]*"},{begin:"```",end:"```+[ ]*$"},{begin:"~~~",end:"~~~+[ ]*$"},{begin:"`.+?`"},{begin:"(?=^( {4}|\\t))",contains:[{begin:"^( {4}|\\t)",end:"(\\n)$"}],relevance:0}]},{begin:"^[-\\*]{3,}",end:"$"},i,{begin:/^\[[^\n]+\]:/,returnBegin:!0,contains:[{className:"symbol",begin:/\[/,end:/\]/,excludeBegin:!0,excludeEnd:!0},{className:"link",begin:/:\s*/,end:/$/,excludeBegin:!0}]}]}}}());hljs.registerLanguage("objectivec",function(){"use strict";return function(e){var n=/[a-zA-Z@][a-zA-Z0-9_]*/,_={$pattern:n,keyword:"@interface @class @protocol @implementation"};return{name:"Objective-C",aliases:["mm","objc","obj-c","obj-c++","objective-c++"],keywords:{$pattern:n,keyword:"int float while char export sizeof typedef const struct for union unsigned long volatile static bool mutable if do return goto void enum else break extern asm case short default double register explicit signed typename this switch continue wchar_t inline readonly assign readwrite self @synchronized id typeof nonatomic super unichar IBOutlet IBAction strong weak copy in out inout bycopy byref oneway __strong __weak __block __autoreleasing @private @protected @public @try @property @end @throw @catch @finally @autoreleasepool @synthesize @dynamic @selector @optional @required @encode @package @import @defs @compatibility_alias __bridge __bridge_transfer __bridge_retained __bridge_retain __covariant __contravariant __kindof _Nonnull _Nullable _Null_unspecified __FUNCTION__ __PRETTY_FUNCTION__ __attribute__ getter setter retain unsafe_unretained nonnull nullable null_unspecified null_resettable class instancetype NS_DESIGNATED_INITIALIZER NS_UNAVAILABLE NS_REQUIRES_SUPER NS_RETURNS_INNER_POINTER NS_INLINE NS_AVAILABLE NS_DEPRECATED NS_ENUM NS_OPTIONS NS_SWIFT_UNAVAILABLE NS_ASSUME_NONNULL_BEGIN NS_ASSUME_NONNULL_END NS_REFINED_FOR_SWIFT NS_SWIFT_NAME NS_SWIFT_NOTHROW NS_DURING NS_HANDLER NS_ENDHANDLER NS_VALUERETURN NS_VOIDRETURN",literal:"false true FALSE TRUE nil YES NO NULL",built_in:"BOOL dispatch_once_t dispatch_queue_t dispatch_sync dispatch_async dispatch_once"},illegal:"</",contains:[{className:"built_in",begin:"\\b(AV|CA|CF|CG|CI|CL|CM|CN|CT|MK|MP|MTK|MTL|NS|SCN|SK|UI|WK|XC)\\w+"},e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE,e.C_NUMBER_MODE,e.QUOTE_STRING_MODE,e.APOS_STRING_MODE,{className:"string",variants:[{begin:'@"',end:'"',illegal:"\\n",contains:[e.BACKSLASH_ESCAPE]}]},{className:"meta",begin:/#\s*[a-z]+\b/,end:/$/,keywords:{"meta-keyword":"if else elif endif define undef warning error line pragma ifdef ifndef include"},contains:[{begin:/\\\n/,relevance:0},e.inherit(e.QUOTE_STRING_MODE,{className:"meta-string"}),{className:"meta-string",begin:/<.*?>/,end:/$/,illegal:"\\n"},e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE]},{className:"class",begin:"("+_.keyword.split(" ").join("|")+")\\b",end:/(\{|$)/,excludeEnd:!0,keywords:_,contains:[e.UNDERSCORE_TITLE_MODE]},{begin:"\\."+e.UNDERSCORE_IDENT_RE,relevance:0}]}}}());hljs.registerLanguage("php",function(){"use strict";return function(e){var r={className:"variable",begin:"\\$+[a-zA-Z_-ÿ][a-zA-Z0-9_-ÿ]*(?![A-Za-z0-9])(?![$])"},t={className:"meta",variants:[{begin:/<\?php/,relevance:10},{begin:/<\?[=]?/},{begin:/\?>/}]},a={className:"subst",variants:[{begin:/\$\w+/},{begin:/\{\$/,end:/\}/}]},n=e.inherit(e.APOS_STRING_MODE,{illegal:null}),i=e.inherit(e.QUOTE_STRING_MODE,{illegal:null,contains:e.QUOTE_STRING_MODE.contains.concat(a)}),o=e.END_SAME_AS_BEGIN({begin:/<<<[ \t]*(\w+)\n/,end:/[ \t]*(\w+)\b/,contains:e.QUOTE_STRING_MODE.contains.concat(a)}),l={className:"string",contains:[e.BACKSLASH_ESCAPE,t],variants:[e.inherit(n,{begin:"b'",end:"'"}),e.inherit(i,{begin:'b"',end:'"'}),i,n,o]},c={variants:[e.BINARY_NUMBER_MODE,e.C_NUMBER_MODE]},s={keyword:"__CLASS__ __DIR__ __FILE__ __FUNCTION__ __LINE__ __METHOD__ __NAMESPACE__ __TRAIT__ die echo exit include include_once print require require_once array abstract and as binary bool boolean break callable case catch class clone const continue declare default do double else elseif empty enddeclare endfor endforeach endif endswitch endwhile eval extends final finally float for foreach from global goto if implements instanceof insteadof int integer interface isset iterable list match|0 new object or private protected public real return string switch throw trait try unset use var void while xor yield",literal:"false null true",built_in:"Error|0 AppendIterator ArgumentCountError ArithmeticError ArrayIterator ArrayObject AssertionError BadFunctionCallException BadMethodCallException CachingIterator CallbackFilterIterator CompileError Countable DirectoryIterator DivisionByZeroError DomainException EmptyIterator ErrorException Exception FilesystemIterator FilterIterator GlobIterator InfiniteIterator InvalidArgumentException IteratorIterator LengthException LimitIterator LogicException MultipleIterator NoRewindIterator OutOfBoundsException OutOfRangeException OuterIterator OverflowException ParentIterator ParseError RangeException RecursiveArrayIterator RecursiveCachingIterator RecursiveCallbackFilterIterator RecursiveDirectoryIterator RecursiveFilterIterator RecursiveIterator RecursiveIteratorIterator RecursiveRegexIterator RecursiveTreeIterator RegexIterator RuntimeException SeekableIterator SplDoublyLinkedList SplFileInfo SplFileObject SplFixedArray SplHeap SplMaxHeap SplMinHeap SplObjectStorage SplObserver SplObserver SplPriorityQueue SplQueue SplStack SplSubject SplSubject SplTempFileObject TypeError UnderflowException UnexpectedValueException ArrayAccess Closure Generator Iterator IteratorAggregate Serializable Throwable Traversable WeakReference Directory __PHP_Incomplete_Class parent php_user_filter self static stdClass"};return{aliases:["php","php3","php4","php5","php6","php7","php8"],case_insensitive:!0,keywords:s,contains:[e.HASH_COMMENT_MODE,e.COMMENT("//","$",{contains:[t]}),e.COMMENT("/\\*","\\*/",{contains:[{className:"doctag",begin:"@[A-Za-z]+"}]}),e.COMMENT("__halt_compiler.+?;",!1,{endsWithParent:!0,keywords:"__halt_compiler"}),t,{className:"keyword",begin:/\$this\b/},r,{begin:/(::|->)+[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*/},{className:"function",relevance:0,beginKeywords:"fn function",end:/[;{]/,excludeEnd:!0,illegal:"[$%\\[]",contains:[e.UNDERSCORE_TITLE_MODE,{begin:"=>"},{className:"params",begin:"\\(",end:"\\)",excludeBegin:!0,excludeEnd:!0,keywords:s,contains:["self",r,e.C_BLOCK_COMMENT_MODE,l,c]}]},{className:"class",beginKeywords:"class interface",relevance:0,end:/\{/,excludeEnd:!0,illegal:/[:($"]/,contains:[{beginKeywords:"extends implements"},e.UNDERSCORE_TITLE_MODE]},{beginKeywords:"namespace",relevance:0,end:";",illegal:/[.']/,contains:[e.UNDERSCORE_TITLE_MODE]},{beginKeywords:"use",relevance:0,end:";",contains:[e.UNDERSCORE_TITLE_MODE]},l,c]}}}());hljs.registerLanguage("perl",function(){"use strict";function e(){for(var _len13=arguments.length,e=new Array(_len13),_key13=0;_key13<_len13;_key13++){e[_key13]=arguments[_key13]}return e.map(function(e){return(n=e)?"string"==typeof n?n:n.source:null;var n}).join("")}return function(n){var t=/[dualxmsipn]{0,12}/,s={$pattern:/[\w.]+/,keyword:"getpwent getservent quotemeta msgrcv scalar kill dbmclose undef lc ma syswrite tr send umask sysopen shmwrite vec qx utime local oct semctl localtime readpipe do return format read sprintf dbmopen pop getpgrp not getpwnam rewinddir qq fileno qw endprotoent wait sethostent bless s|0 opendir continue each sleep endgrent shutdown dump chomp connect getsockname die socketpair close flock exists index shmget sub for endpwent redo lstat msgctl setpgrp abs exit select print ref gethostbyaddr unshift fcntl syscall goto getnetbyaddr join gmtime symlink semget splice x|0 getpeername recv log setsockopt cos last reverse gethostbyname getgrnam study formline endhostent times chop length gethostent getnetent pack getprotoent getservbyname rand mkdir pos chmod y|0 substr endnetent printf next open msgsnd readdir use unlink getsockopt getpriority rindex wantarray hex system getservbyport endservent int chr untie rmdir prototype tell listen fork shmread ucfirst setprotoent else sysseek link getgrgid shmctl waitpid unpack getnetbyname reset chdir grep split require caller lcfirst until warn while values shift telldir getpwuid my getprotobynumber delete and sort uc defined srand accept package seekdir getprotobyname semop our rename seek if q|0 chroot sysread setpwent no crypt getc chown sqrt write setnetent setpriority foreach tie sin msgget map stat getlogin unless elsif truncate exec keys glob tied closedir ioctl socket readlink eval xor readline binmode setservent eof ord bind alarm pipe atan2 getgrent exp time push setgrent gt lt or ne m|0 break given say state when"},r={className:"subst",begin:"[$@]\\{",end:"\\}",keywords:s},i={begin:/->\{/,end:/\}/},a={variants:[{begin:/\$\d/},{begin:e(/[$%@](\^\w\b|#\w+(::\w+)*|\{\w+\}|\w+(::\w*)*)/,"(?![A-Za-z])(?![@$%])")},{begin:/[$%@][^\s\w{]/,relevance:0}]},o=[n.BACKSLASH_ESCAPE,r,a],c=[a,n.HASH_COMMENT_MODE,n.COMMENT(/^=\w/,/=cut/,{endsWithParent:!0}),i,{className:"string",contains:o,variants:[{begin:"q[qwxr]?\\s*\\(",end:"\\)",relevance:5},{begin:"q[qwxr]?\\s*\\[",end:"\\]",relevance:5},{begin:"q[qwxr]?\\s*\\{",end:"\\}",relevance:5},{begin:"q[qwxr]?\\s*\\|",end:"\\|",relevance:5},{begin:"q[qwxr]?\\s*<",end:">",relevance:5},{begin:"qw\\s+q",end:"q",relevance:5},{begin:"'",end:"'",contains:[n.BACKSLASH_ESCAPE]},{begin:'"',end:'"'},{begin:"`",end:"`",contains:[n.BACKSLASH_ESCAPE]},{begin:/\{\w+\}/,contains:[],relevance:0},{begin:"-?\\w+\\s*=>",contains:[],relevance:0}]},{className:"number",begin:"(\\b0[0-7_]+)|(\\b0x[0-9a-fA-F_]+)|(\\b[1-9][0-9_]*(\\.[0-9_]+)?)|[0_]\\b",relevance:0},{begin:"(\\/\\/|"+n.RE_STARTERS_RE+"|\\b(split|return|print|reverse|grep)\\b)\\s*",keywords:"split return print reverse grep",relevance:0,contains:[n.HASH_COMMENT_MODE,{className:"regexp",begin:e(/(s|tr|y)/,/\//,/(\\.|[^\\\/])*/,/\//,/(\\.|[^\\\/])*/,/\//,t),relevance:10},{className:"regexp",begin:/(m|qr)?\//,end:e(/\//,t),contains:[n.BACKSLASH_ESCAPE],relevance:0}]},{className:"function",beginKeywords:"sub",end:"(\\s*\\(.*?\\))?[;{]",excludeEnd:!0,relevance:5,contains:[n.TITLE_MODE]},{begin:"-\\w\\b",relevance:0},{begin:"^__DATA__$",end:"^__END__$",subLanguage:"mojolicious",contains:[{begin:"^@@.*",end:"$",className:"comment"}]}];return r.contains=c,i.contains=c,{name:"Perl",aliases:["pl","pm"],keywords:s,contains:c}}}());hljs.registerLanguage("python",function(){"use strict";return function(e){var n={keyword:"and as assert async await break class continue def del elif else except finally for  from global if import in is lambda nonlocal|10 not or pass raise return try while with yield",built_in:"__import__ abs all any ascii bin bool breakpoint bytearray bytes callable chr classmethod compile complex delattr dict dir divmod enumerate eval exec filter float format frozenset getattr globals hasattr hash help hex id input int isinstance issubclass iter len list locals map max memoryview min next object oct open ord pow print property range repr reversed round set setattr slice sorted staticmethod str sum super tuple type vars zip",literal:"__debug__ Ellipsis False None NotImplemented True"},a={className:"meta",begin:/^(>>>|\.\.\.) /},s={className:"subst",begin:/\{/,end:/\}/,keywords:n,illegal:/#/},i={begin:/\{\{/,relevance:0},r={className:"string",contains:[e.BACKSLASH_ESCAPE],variants:[{begin:/([uU]|[bB]|[rR]|[bB][rR]|[rR][bB])?'''/,end:/'''/,contains:[e.BACKSLASH_ESCAPE,a],relevance:10},{begin:/([uU]|[bB]|[rR]|[bB][rR]|[rR][bB])?"""/,end:/"""/,contains:[e.BACKSLASH_ESCAPE,a],relevance:10},{begin:/([fF][rR]|[rR][fF]|[fF])'''/,end:/'''/,contains:[e.BACKSLASH_ESCAPE,a,i,s]},{begin:/([fF][rR]|[rR][fF]|[fF])"""/,end:/"""/,contains:[e.BACKSLASH_ESCAPE,a,i,s]},{begin:/([uU]|[rR])'/,end:/'/,relevance:10},{begin:/([uU]|[rR])"/,end:/"/,relevance:10},{begin:/([bB]|[bB][rR]|[rR][bB])'/,end:/'/},{begin:/([bB]|[bB][rR]|[rR][bB])"/,end:/"/},{begin:/([fF][rR]|[rR][fF]|[fF])'/,end:/'/,contains:[e.BACKSLASH_ESCAPE,i,s]},{begin:/([fF][rR]|[rR][fF]|[fF])"/,end:/"/,contains:[e.BACKSLASH_ESCAPE,i,s]},e.APOS_STRING_MODE,e.QUOTE_STRING_MODE]},t="[0-9](_?[0-9])*",l="(\\b(".concat(t,"))?\\.(").concat(t,")|\\b(").concat(t,")\\."),b={className:"number",relevance:0,variants:[{begin:"(\\b(".concat(t,")|(").concat(l,"))[eE][+-]?(").concat(t,")[jJ]?\\b")},{begin:"(".concat(l,")[jJ]?")},{begin:"\\b([1-9](_?[0-9])*|0+(_?0)*)[lLjJ]?\\b"},{begin:"\\b0[bB](_?[01])+[lL]?\\b"},{begin:"\\b0[oO](_?[0-7])+[lL]?\\b"},{begin:"\\b0[xX](_?[0-9a-fA-F])+[lL]?\\b"},{begin:"\\b(".concat(t,")[jJ]\\b")}]},o={className:"params",variants:[{begin:/\(\s*\)/,skip:!0,className:null},{begin:/\(/,end:/\)/,excludeBegin:!0,excludeEnd:!0,keywords:n,contains:["self",a,b,r,e.HASH_COMMENT_MODE]}]};return s.contains=[r,b,a],{name:"Python",aliases:["py","gyp","ipython"],keywords:n,illegal:/(<\/|->|\?)|=>/,contains:[a,b,{begin:/\bself\b/},{beginKeywords:"if",relevance:0},r,e.HASH_COMMENT_MODE,{variants:[{className:"function",beginKeywords:"def"},{className:"class",beginKeywords:"class"}],end:/:/,illegal:/[${=;\n,]/,contains:[e.UNDERSCORE_TITLE_MODE,o,{begin:/->/,endsWithParent:!0,keywords:"None"}]},{className:"meta",begin:/^[\t ]*@/,end:/(?=#)|$/,contains:[b,o,r]},{begin:/\b(print|exec)\(/}]}}}());hljs.registerLanguage("ruby",function(){"use strict";function e(){for(var _len14=arguments.length,e=new Array(_len14),_key14=0;_key14<_len14;_key14++){e[_key14]=arguments[_key14]}return e.map(function(e){return(n=e)?"string"==typeof n?n:n.source:null;var n}).join("")}return function(n){var a,i="([a-zA-Z_]\\w*[!?=]?|[-+~]@|<<|>>|=~|===?|<=>|[<>]=?|\\*\\*|[-/+%^&*~`|]|\\[\\]=?)",s={keyword:"and then defined module in return redo if BEGIN retry end for self when next until do begin unless END rescue else break undef not super class case require yield alias while ensure elsif or include attr_reader attr_writer attr_accessor __FILE__",built_in:"proc lambda",literal:"true false nil"},r={className:"doctag",begin:"@[A-Za-z]+"},b={begin:"#<",end:">"},t=[n.COMMENT("#","$",{contains:[r]}),n.COMMENT("^=begin","^=end",{contains:[r],relevance:10}),n.COMMENT("^__END__","\\n$")],c={className:"subst",begin:/#\{/,end:/\}/,keywords:s},d={className:"string",contains:[n.BACKSLASH_ESCAPE,c],variants:[{begin:/'/,end:/'/},{begin:/"/,end:/"/},{begin:/`/,end:/`/},{begin:/%[qQwWx]?\(/,end:/\)/},{begin:/%[qQwWx]?\[/,end:/\]/},{begin:/%[qQwWx]?\{/,end:/\}/},{begin:/%[qQwWx]?</,end:/>/},{begin:/%[qQwWx]?\//,end:/\//},{begin:/%[qQwWx]?%/,end:/%/},{begin:/%[qQwWx]?-/,end:/-/},{begin:/%[qQwWx]?\|/,end:/\|/},{begin:/\B\?(\\\d{1,3}|\\x[A-Fa-f0-9]{1,2}|\\u[A-Fa-f0-9]{4}|\\?\S)\b/},{begin:/<<[-~]?'?(\w+)\n(?:[^\n]*\n)*?\s*\1\b/,returnBegin:!0,contains:[{begin:/<<[-~]?'?/},n.END_SAME_AS_BEGIN({begin:/(\w+)/,end:/(\w+)/,contains:[n.BACKSLASH_ESCAPE,c]})]}]},g="[0-9](_?[0-9])*",l={className:"number",relevance:0,variants:[{begin:"\\b([1-9](_?[0-9])*|0)(\\.(".concat(g,"))?([eE][+-]?(").concat(g,")|r)?i?\\b")},{begin:"\\b0[dD][0-9](_?[0-9])*r?i?\\b"},{begin:"\\b0[bB][0-1](_?[0-1])*r?i?\\b"},{begin:"\\b0[oO][0-7](_?[0-7])*r?i?\\b"},{begin:"\\b0[xX][0-9a-fA-F](_?[0-9a-fA-F])*r?i?\\b"},{begin:"\\b0(_?[0-7])+r?i?\\b"}]},o={className:"params",begin:"\\(",end:"\\)",endsParent:!0,keywords:s},_=[d,{className:"class",beginKeywords:"class module",end:"$|;",illegal:/=/,contains:[n.inherit(n.TITLE_MODE,{begin:"[A-Za-z_]\\w*(::\\w+)*(\\?|!)?"}),{begin:"<\\s*",contains:[{begin:"("+n.IDENT_RE+"::)?"+n.IDENT_RE}]}].concat(t)},{className:"function",begin:e(/def\s*/,(a=i+"\\s*(\\(|;|$)",e("(?=",a,")"))),keywords:"def",end:"$|;",contains:[n.inherit(n.TITLE_MODE,{begin:i}),o].concat(t)},{begin:n.IDENT_RE+"::"},{className:"symbol",begin:n.UNDERSCORE_IDENT_RE+"(!|\\?)?:",relevance:0},{className:"symbol",begin:":(?!\\s)",contains:[d,{begin:i}],relevance:0},l,{className:"variable",begin:"(\\$\\W)|((\\$|@@?)(\\w+))(?=[^@$?])(?![A-Za-z])(?![@$?'])"},{className:"params",begin:/\|/,end:/\|/,relevance:0,keywords:s},{begin:"("+n.RE_STARTERS_RE+"|unless)\\s*",keywords:"unless",contains:[{className:"regexp",contains:[n.BACKSLASH_ESCAPE,c],illegal:/\n/,variants:[{begin:"/",end:"/[a-z]*"},{begin:/%r\{/,end:/\}[a-z]*/},{begin:"%r\\(",end:"\\)[a-z]*"},{begin:"%r!",end:"![a-z]*"},{begin:"%r\\[",end:"\\][a-z]*"}]}].concat(b,t),relevance:0}].concat(b,t);c.contains=_,o.contains=_;var E=[{begin:/^\s*=>/,starts:{end:"$",contains:_}},{className:"meta",begin:"^([>?]>|[\\w#]+\\(\\w+\\):\\d+:\\d+>|(\\w+-)?\\d+\\.\\d+\\.\\d+(p\\d+)?[^\\d][^>]+>)(?=[ ])",starts:{end:"$",contains:_}}];return t.unshift(b),{name:"Ruby",aliases:["rb","gemspec","podspec","thor","irb"],keywords:s,illegal:/\/\*/,contains:[n.SHEBANG({binary:"ruby"})].concat(E).concat(t).concat(_)}}}());hljs.registerLanguage("shell",function(){"use strict";return function(s){return{name:"Shell Session",aliases:["console"],contains:[{className:"meta",begin:/^\s{0,3}[/~\w\d[\]()@-]*[>%$#]/,starts:{end:/[^\\](?=\s*$)/,subLanguage:"bash"}}]}}}());hljs.registerLanguage("armasm",function(){"use strict";return function(s){var e={variants:[s.COMMENT("^[ \\t]*(?=#)","$",{relevance:0,excludeBegin:!0}),s.COMMENT("[;@]","$",{relevance:0}),s.C_LINE_COMMENT_MODE,s.C_BLOCK_COMMENT_MODE]};return{name:"ARM Assembly",case_insensitive:!0,aliases:["arm"],keywords:{$pattern:"\\.?"+s.IDENT_RE,meta:".2byte .4byte .align .ascii .asciz .balign .byte .code .data .else .end .endif .endm .endr .equ .err .exitm .extern .global .hword .if .ifdef .ifndef .include .irp .long .macro .rept .req .section .set .skip .space .text .word .arm .thumb .code16 .code32 .force_thumb .thumb_func .ltorg ALIAS ALIGN ARM AREA ASSERT ATTR CN CODE CODE16 CODE32 COMMON CP DATA DCB DCD DCDU DCDO DCFD DCFDU DCI DCQ DCQU DCW DCWU DN ELIF ELSE END ENDFUNC ENDIF ENDP ENTRY EQU EXPORT EXPORTAS EXTERN FIELD FILL FUNCTION GBLA GBLL GBLS GET GLOBAL IF IMPORT INCBIN INCLUDE INFO KEEP LCLA LCLL LCLS LTORG MACRO MAP MEND MEXIT NOFP OPT PRESERVE8 PROC QN READONLY RELOC REQUIRE REQUIRE8 RLIST FN ROUT SETA SETL SETS SN SPACE SUBT THUMB THUMBX TTL WHILE WEND ",built_in:"r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 pc lr sp ip sl sb fp a1 a2 a3 a4 v1 v2 v3 v4 v5 v6 v7 v8 f0 f1 f2 f3 f4 f5 f6 f7 p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 c10 c11 c12 c13 c14 c15 q0 q1 q2 q3 q4 q5 q6 q7 q8 q9 q10 q11 q12 q13 q14 q15 cpsr_c cpsr_x cpsr_s cpsr_f cpsr_cx cpsr_cxs cpsr_xs cpsr_xsf cpsr_sf cpsr_cxsf spsr_c spsr_x spsr_s spsr_f spsr_cx spsr_cxs spsr_xs spsr_xsf spsr_sf spsr_cxsf s0 s1 s2 s3 s4 s5 s6 s7 s8 s9 s10 s11 s12 s13 s14 s15 s16 s17 s18 s19 s20 s21 s22 s23 s24 s25 s26 s27 s28 s29 s30 s31 d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 d10 d11 d12 d13 d14 d15 d16 d17 d18 d19 d20 d21 d22 d23 d24 d25 d26 d27 d28 d29 d30 d31 {PC} {VAR} {TRUE} {FALSE} {OPT} {CONFIG} {ENDIAN} {CODESIZE} {CPU} {FPU} {ARCHITECTURE} {PCSTOREOFFSET} {ARMASM_VERSION} {INTER} {ROPI} {RWPI} {SWST} {NOSWST} . @"},contains:[{className:"keyword",begin:"\\b(adc|(qd?|sh?|u[qh]?)?add(8|16)?|usada?8|(q|sh?|u[qh]?)?(as|sa)x|and|adrl?|sbc|rs[bc]|asr|b[lx]?|blx|bxj|cbn?z|tb[bh]|bic|bfc|bfi|[su]bfx|bkpt|cdp2?|clz|clrex|cmp|cmn|cpsi[ed]|cps|setend|dbg|dmb|dsb|eor|isb|it[te]{0,3}|lsl|lsr|ror|rrx|ldm(([id][ab])|f[ds])?|ldr((s|ex)?[bhd])?|movt?|mvn|mra|mar|mul|[us]mull|smul[bwt][bt]|smu[as]d|smmul|smmla|mla|umlaal|smlal?([wbt][bt]|d)|mls|smlsl?[ds]|smc|svc|sev|mia([bt]{2}|ph)?|mrr?c2?|mcrr2?|mrs|msr|orr|orn|pkh(tb|bt)|rbit|rev(16|sh)?|sel|[su]sat(16)?|nop|pop|push|rfe([id][ab])?|stm([id][ab])?|str(ex)?[bhd]?|(qd?)?sub|(sh?|q|u[qh]?)?sub(8|16)|[su]xt(a?h|a?b(16)?)|srs([id][ab])?|swpb?|swi|smi|tst|teq|wfe|wfi|yield)(eq|ne|cs|cc|mi|pl|vs|vc|hi|ls|ge|lt|gt|le|al|hs|lo)?[sptrx]?(?=\\s)"},e,s.QUOTE_STRING_MODE,{className:"string",begin:"'",end:"[^\\\\]'",relevance:0},{className:"title",begin:"\\|",end:"\\|",illegal:"\\n",relevance:0},{className:"number",variants:[{begin:"[#$=]?0x[0-9a-f]+"},{begin:"[#$=]?0b[01]+"},{begin:"[#$=]\\d+"},{begin:"\\b\\d+"}],relevance:0},{className:"symbol",variants:[{begin:"^[ \\t]*[a-z_\\.\\$][a-z0-9_\\.\\$]+:"},{begin:"^[a-z_\\.\\$][a-z0-9_\\.\\$]+"},{begin:"[=#]\\w+"}],relevance:0}]}}}());hljs.registerLanguage("glsl",function(){"use strict";return function(e){return{name:"GLSL",keywords:{keyword:"break continue discard do else for if return while switch case default attribute binding buffer ccw centroid centroid varying coherent column_major const cw depth_any depth_greater depth_less depth_unchanged early_fragment_tests equal_spacing flat fractional_even_spacing fractional_odd_spacing highp in index inout invariant invocations isolines layout line_strip lines lines_adjacency local_size_x local_size_y local_size_z location lowp max_vertices mediump noperspective offset origin_upper_left out packed patch pixel_center_integer point_mode points precise precision quads r11f_g11f_b10f r16 r16_snorm r16f r16i r16ui r32f r32i r32ui r8 r8_snorm r8i r8ui readonly restrict rg16 rg16_snorm rg16f rg16i rg16ui rg32f rg32i rg32ui rg8 rg8_snorm rg8i rg8ui rgb10_a2 rgb10_a2ui rgba16 rgba16_snorm rgba16f rgba16i rgba16ui rgba32f rgba32i rgba32ui rgba8 rgba8_snorm rgba8i rgba8ui row_major sample shared smooth std140 std430 stream triangle_strip triangles triangles_adjacency uniform varying vertices volatile writeonly",type:"atomic_uint bool bvec2 bvec3 bvec4 dmat2 dmat2x2 dmat2x3 dmat2x4 dmat3 dmat3x2 dmat3x3 dmat3x4 dmat4 dmat4x2 dmat4x3 dmat4x4 double dvec2 dvec3 dvec4 float iimage1D iimage1DArray iimage2D iimage2DArray iimage2DMS iimage2DMSArray iimage2DRect iimage3D iimageBuffer iimageCube iimageCubeArray image1D image1DArray image2D image2DArray image2DMS image2DMSArray image2DRect image3D imageBuffer imageCube imageCubeArray int isampler1D isampler1DArray isampler2D isampler2DArray isampler2DMS isampler2DMSArray isampler2DRect isampler3D isamplerBuffer isamplerCube isamplerCubeArray ivec2 ivec3 ivec4 mat2 mat2x2 mat2x3 mat2x4 mat3 mat3x2 mat3x3 mat3x4 mat4 mat4x2 mat4x3 mat4x4 sampler1D sampler1DArray sampler1DArrayShadow sampler1DShadow sampler2D sampler2DArray sampler2DArrayShadow sampler2DMS sampler2DMSArray sampler2DRect sampler2DRectShadow sampler2DShadow sampler3D samplerBuffer samplerCube samplerCubeArray samplerCubeArrayShadow samplerCubeShadow image1D uimage1DArray uimage2D uimage2DArray uimage2DMS uimage2DMSArray uimage2DRect uimage3D uimageBuffer uimageCube uimageCubeArray uint usampler1D usampler1DArray usampler2D usampler2DArray usampler2DMS usampler2DMSArray usampler2DRect usampler3D samplerBuffer usamplerCube usamplerCubeArray uvec2 uvec3 uvec4 vec2 vec3 vec4 void",built_in:"gl_MaxAtomicCounterBindings gl_MaxAtomicCounterBufferSize gl_MaxClipDistances gl_MaxClipPlanes gl_MaxCombinedAtomicCounterBuffers gl_MaxCombinedAtomicCounters gl_MaxCombinedImageUniforms gl_MaxCombinedImageUnitsAndFragmentOutputs gl_MaxCombinedTextureImageUnits gl_MaxComputeAtomicCounterBuffers gl_MaxComputeAtomicCounters gl_MaxComputeImageUniforms gl_MaxComputeTextureImageUnits gl_MaxComputeUniformComponents gl_MaxComputeWorkGroupCount gl_MaxComputeWorkGroupSize gl_MaxDrawBuffers gl_MaxFragmentAtomicCounterBuffers gl_MaxFragmentAtomicCounters gl_MaxFragmentImageUniforms gl_MaxFragmentInputComponents gl_MaxFragmentInputVectors gl_MaxFragmentUniformComponents gl_MaxFragmentUniformVectors gl_MaxGeometryAtomicCounterBuffers gl_MaxGeometryAtomicCounters gl_MaxGeometryImageUniforms gl_MaxGeometryInputComponents gl_MaxGeometryOutputComponents gl_MaxGeometryOutputVertices gl_MaxGeometryTextureImageUnits gl_MaxGeometryTotalOutputComponents gl_MaxGeometryUniformComponents gl_MaxGeometryVaryingComponents gl_MaxImageSamples gl_MaxImageUnits gl_MaxLights gl_MaxPatchVertices gl_MaxProgramTexelOffset gl_MaxTessControlAtomicCounterBuffers gl_MaxTessControlAtomicCounters gl_MaxTessControlImageUniforms gl_MaxTessControlInputComponents gl_MaxTessControlOutputComponents gl_MaxTessControlTextureImageUnits gl_MaxTessControlTotalOutputComponents gl_MaxTessControlUniformComponents gl_MaxTessEvaluationAtomicCounterBuffers gl_MaxTessEvaluationAtomicCounters gl_MaxTessEvaluationImageUniforms gl_MaxTessEvaluationInputComponents gl_MaxTessEvaluationOutputComponents gl_MaxTessEvaluationTextureImageUnits gl_MaxTessEvaluationUniformComponents gl_MaxTessGenLevel gl_MaxTessPatchComponents gl_MaxTextureCoords gl_MaxTextureImageUnits gl_MaxTextureUnits gl_MaxVaryingComponents gl_MaxVaryingFloats gl_MaxVaryingVectors gl_MaxVertexAtomicCounterBuffers gl_MaxVertexAtomicCounters gl_MaxVertexAttribs gl_MaxVertexImageUniforms gl_MaxVertexOutputComponents gl_MaxVertexOutputVectors gl_MaxVertexTextureImageUnits gl_MaxVertexUniformComponents gl_MaxVertexUniformVectors gl_MaxViewports gl_MinProgramTexelOffset gl_BackColor gl_BackLightModelProduct gl_BackLightProduct gl_BackMaterial gl_BackSecondaryColor gl_ClipDistance gl_ClipPlane gl_ClipVertex gl_Color gl_DepthRange gl_EyePlaneQ gl_EyePlaneR gl_EyePlaneS gl_EyePlaneT gl_Fog gl_FogCoord gl_FogFragCoord gl_FragColor gl_FragCoord gl_FragData gl_FragDepth gl_FrontColor gl_FrontFacing gl_FrontLightModelProduct gl_FrontLightProduct gl_FrontMaterial gl_FrontSecondaryColor gl_GlobalInvocationID gl_InstanceID gl_InvocationID gl_Layer gl_LightModel gl_LightSource gl_LocalInvocationID gl_LocalInvocationIndex gl_ModelViewMatrix gl_ModelViewMatrixInverse gl_ModelViewMatrixInverseTranspose gl_ModelViewMatrixTranspose gl_ModelViewProjectionMatrix gl_ModelViewProjectionMatrixInverse gl_ModelViewProjectionMatrixInverseTranspose gl_ModelViewProjectionMatrixTranspose gl_MultiTexCoord0 gl_MultiTexCoord1 gl_MultiTexCoord2 gl_MultiTexCoord3 gl_MultiTexCoord4 gl_MultiTexCoord5 gl_MultiTexCoord6 gl_MultiTexCoord7 gl_Normal gl_NormalMatrix gl_NormalScale gl_NumSamples gl_NumWorkGroups gl_ObjectPlaneQ gl_ObjectPlaneR gl_ObjectPlaneS gl_ObjectPlaneT gl_PatchVerticesIn gl_Point gl_PointCoord gl_PointSize gl_Position gl_PrimitiveID gl_PrimitiveIDIn gl_ProjectionMatrix gl_ProjectionMatrixInverse gl_ProjectionMatrixInverseTranspose gl_ProjectionMatrixTranspose gl_SampleID gl_SampleMask gl_SampleMaskIn gl_SamplePosition gl_SecondaryColor gl_TessCoord gl_TessLevelInner gl_TessLevelOuter gl_TexCoord gl_TextureEnvColor gl_TextureMatrix gl_TextureMatrixInverse gl_TextureMatrixInverseTranspose gl_TextureMatrixTranspose gl_Vertex gl_VertexID gl_ViewportIndex gl_WorkGroupID gl_WorkGroupSize gl_in gl_out EmitStreamVertex EmitVertex EndPrimitive EndStreamPrimitive abs acos acosh all any asin asinh atan atanh atomicAdd atomicAnd atomicCompSwap atomicCounter atomicCounterDecrement atomicCounterIncrement atomicExchange atomicMax atomicMin atomicOr atomicXor barrier bitCount bitfieldExtract bitfieldInsert bitfieldReverse ceil clamp cos cosh cross dFdx dFdy degrees determinant distance dot equal exp exp2 faceforward findLSB findMSB floatBitsToInt floatBitsToUint floor fma fract frexp ftransform fwidth greaterThan greaterThanEqual groupMemoryBarrier imageAtomicAdd imageAtomicAnd imageAtomicCompSwap imageAtomicExchange imageAtomicMax imageAtomicMin imageAtomicOr imageAtomicXor imageLoad imageSize imageStore imulExtended intBitsToFloat interpolateAtCentroid interpolateAtOffset interpolateAtSample inverse inversesqrt isinf isnan ldexp length lessThan lessThanEqual log log2 matrixCompMult max memoryBarrier memoryBarrierAtomicCounter memoryBarrierBuffer memoryBarrierImage memoryBarrierShared min mix mod modf noise1 noise2 noise3 noise4 normalize not notEqual outerProduct packDouble2x32 packHalf2x16 packSnorm2x16 packSnorm4x8 packUnorm2x16 packUnorm4x8 pow radians reflect refract round roundEven shadow1D shadow1DLod shadow1DProj shadow1DProjLod shadow2D shadow2DLod shadow2DProj shadow2DProjLod sign sin sinh smoothstep sqrt step tan tanh texelFetch texelFetchOffset texture texture1D texture1DLod texture1DProj texture1DProjLod texture2D texture2DLod texture2DProj texture2DProjLod texture3D texture3DLod texture3DProj texture3DProjLod textureCube textureCubeLod textureGather textureGatherOffset textureGatherOffsets textureGrad textureGradOffset textureLod textureLodOffset textureOffset textureProj textureProjGrad textureProjGradOffset textureProjLod textureProjLodOffset textureProjOffset textureQueryLevels textureQueryLod textureSize transpose trunc uaddCarry uintBitsToFloat umulExtended unpackDouble2x32 unpackHalf2x16 unpackSnorm2x16 unpackSnorm4x8 unpackUnorm2x16 unpackUnorm4x8 usubBorrow",literal:"true false"},illegal:'"',contains:[e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE,e.C_NUMBER_MODE,{className:"meta",begin:"#",end:"$"}]}}}());hljs.registerLanguage("go",function(){"use strict";return function(e){var n={keyword:"break default func interface select case map struct chan else goto package switch const fallthrough if range type continue for import return var go defer bool byte complex64 complex128 float32 float64 int8 int16 int32 int64 string uint8 uint16 uint32 uint64 int uint uintptr rune",literal:"true false iota nil",built_in:"append cap close complex copy imag len make new panic print println real recover delete"};return{name:"Go",aliases:["golang"],keywords:n,illegal:"</",contains:[e.C_LINE_COMMENT_MODE,e.C_BLOCK_COMMENT_MODE,{className:"string",variants:[e.QUOTE_STRING_MODE,e.APOS_STRING_MODE,{begin:"`",end:"`"}]},{className:"number",variants:[{begin:e.C_NUMBER_RE+"[i]",relevance:1},e.C_NUMBER_MODE]},{begin:/:=/},{className:"function",beginKeywords:"func",end:"\\s*(\\{|$)",excludeEnd:!0,contains:[e.TITLE_MODE,{className:"params",begin:/\(/,end:/\)/,keywords:n,illegal:/["']/}]}]}}}());hljs.registerLanguage("haskell",function(){"use strict";return function(e){var n={variants:[e.COMMENT("--","$"),e.COMMENT(/\{-/,/-\}/,{contains:["self"]})]},i={className:"meta",begin:/\{-#/,end:/#-\}/},a={className:"meta",begin:"^#",end:"$"},s={className:"type",begin:"\\b[A-Z][\\w']*",relevance:0},l={begin:"\\(",end:"\\)",illegal:'"',contains:[i,a,{className:"type",begin:"\\b[A-Z][\\w]*(\\((\\.\\.|,|\\w+)\\))?"},e.inherit(e.TITLE_MODE,{begin:"[_a-z][\\w']*"}),n]};return{name:"Haskell",aliases:["hs"],keywords:"let in if then else case of where do module import hiding qualified type data newtype deriving class instance as default infix infixl infixr foreign export ccall stdcall cplusplus jvm dotnet safe unsafe family forall mdo proc rec",contains:[{beginKeywords:"module",end:"where",keywords:"module where",contains:[l,n],illegal:"\\W\\.|;"},{begin:"\\bimport\\b",end:"$",keywords:"import qualified as hiding",contains:[l,n],illegal:"\\W\\.|;"},{className:"class",begin:"^(\\s*)?(class|instance)\\b",end:"where",keywords:"class family instance where",contains:[s,l,n]},{className:"class",begin:"\\b(data|(new)?type)\\b",end:"$",keywords:"data family type newtype deriving",contains:[i,s,l,{begin:/\{/,end:/\}/,contains:l.contains},n]},{beginKeywords:"default",end:"$",contains:[s,l,n]},{beginKeywords:"infix infixl infixr",end:"$",contains:[e.C_NUMBER_MODE,n]},{begin:"\\bforeign\\b",end:"$",keywords:"foreign import export ccall stdcall cplusplus jvm dotnet safe unsafe",contains:[s,e.QUOTE_STRING_MODE,n]},{className:"meta",begin:"#!\\/usr\\/bin\\/env runhaskell",end:"$"},i,a,e.QUOTE_STRING_MODE,e.C_NUMBER_MODE,s,e.inherit(e.TITLE_MODE,{begin:"^[_a-z][\\w']*"}),n,{begin:"->|<-"}]}}}());hljs.registerLanguage("kotlin",function(){"use strict";var e="\\.([0-9](_*[0-9])*)",n="[0-9a-fA-F](_*[0-9a-fA-F])*",a={className:"number",variants:[{begin:"(\\b([0-9](_*[0-9])*)((".concat(e,")|\\.)?|(").concat(e,"))[eE][+-]?([0-9](_*[0-9])*)[fFdD]?\\b")},{begin:"\\b([0-9](_*[0-9])*)((".concat(e,")[fFdD]?\\b|\\.([fFdD]\\b)?)")},{begin:"(".concat(e,")[fFdD]?\\b")},{begin:"\\b([0-9](_*[0-9])*)[fFdD]\\b"},{begin:"\\b0[xX]((".concat(n,")\\.?|(").concat(n,")?\\.(").concat(n,"))[pP][+-]?([0-9](_*[0-9])*)[fFdD]?\\b")},{begin:"\\b(0|[1-9](_*[0-9])*)[lL]?\\b"},{begin:"\\b0[xX](".concat(n,")[lL]?\\b")},{begin:"\\b0(_*[0-7])*[lL]?\\b"},{begin:"\\b0[bB][01](_*[01])*[lL]?\\b"}],relevance:0};return function(e){var n={keyword:"abstract as val var vararg get set class object open private protected public noinline crossinline dynamic final enum if else do while for when throw try catch finally import package is in fun override companion reified inline lateinit init interface annotation data sealed internal infix operator out by constructor super tailrec where const inner suspend typealias external expect actual",built_in:"Byte Short Char Int Long Boolean Float Double Void Unit Nothing",literal:"true false null"},i={className:"symbol",begin:e.UNDERSCORE_IDENT_RE+"@"},s={className:"subst",begin:/\$\{/,end:/\}/,contains:[e.C_NUMBER_MODE]},t={className:"variable",begin:"\\$"+e.UNDERSCORE_IDENT_RE},r={className:"string",variants:[{begin:'"""',end:'"""(?=[^"])',contains:[t,s]},{begin:"'",end:"'",illegal:/\n/,contains:[e.BACKSLASH_ESCAPE]},{begin:'"',end:'"',illegal:/\n/,contains:[e.BACKSLASH_ESCAPE,t,s]}]};s.contains.push(r);var l={className:"meta",begin:"@(?:file|property|field|get|set|receiver|param|setparam|delegate)\\s*:(?:\\s*"+e.UNDERSCORE_IDENT_RE+")?"},c={className:"meta",begin:"@"+e.UNDERSCORE_IDENT_RE,contains:[{begin:/\(/,end:/\)/,contains:[e.inherit(r,{className:"meta-string"})]}]},o=a,b=e.COMMENT("/\\*","\\*/",{contains:[e.C_BLOCK_COMMENT_MODE]}),E={variants:[{className:"type",begin:e.UNDERSCORE_IDENT_RE},{begin:/\(/,end:/\)/,contains:[]}]},d=E;return d.variants[1].contains=[E],E.variants[1].contains=[d],{name:"Kotlin",aliases:["kt"],keywords:n,contains:[e.COMMENT("/\\*\\*","\\*/",{relevance:0,contains:[{className:"doctag",begin:"@[A-Za-z]+"}]}),e.C_LINE_COMMENT_MODE,b,{className:"keyword",begin:/\b(break|continue|return|this)\b/,starts:{contains:[{className:"symbol",begin:/@\w+/}]}},i,l,c,{className:"function",beginKeywords:"fun",end:"[(]|$",returnBegin:!0,excludeEnd:!0,keywords:n,relevance:5,contains:[{begin:e.UNDERSCORE_IDENT_RE+"\\s*\\(",returnBegin:!0,relevance:0,contains:[e.UNDERSCORE_TITLE_MODE]},{className:"type",begin:/</,end:/>/,keywords:"reified",relevance:0},{className:"params",begin:/\(/,end:/\)/,endsParent:!0,keywords:n,relevance:0,contains:[{begin:/:/,end:/[=,\/]/,endsWithParent:!0,contains:[E,e.C_LINE_COMMENT_MODE,b],relevance:0},e.C_LINE_COMMENT_MODE,b,l,c,r,e.C_NUMBER_MODE]},b]},{className:"class",beginKeywords:"class interface trait",end:/[:\{(]|$/,excludeEnd:!0,illegal:"extends implements",contains:[{beginKeywords:"public protected internal private constructor"},e.UNDERSCORE_TITLE_MODE,{className:"type",begin:/</,end:/>/,excludeBegin:!0,excludeEnd:!0,relevance:0},{className:"type",begin:/[,:]\s*/,end:/[<\(,]|$/,excludeBegin:!0,returnEnd:!0},l,c]},r,{className:"meta",begin:"^#!/usr/bin/env",end:"$",illegal:"\n"},o]}}}());hljs.registerLanguage("lisp",function(){"use strict";return function(e){var n="[a-zA-Z_\\-+\\*\\/<=>&#][a-zA-Z0-9_\\-+*\\/<=>&#!]*",a="\\|[^]*?\\|",i="(-|\\+)?\\d+(\\.\\d+|\\/\\d+)?((d|e|f|l|s|D|E|F|L|S)(\\+|-)?\\d+)?",s={className:"literal",begin:"\\b(t{1}|nil)\\b"},l={className:"number",variants:[{begin:i,relevance:0},{begin:"#(b|B)[0-1]+(/[0-1]+)?"},{begin:"#(o|O)[0-7]+(/[0-7]+)?"},{begin:"#(x|X)[0-9a-fA-F]+(/[0-9a-fA-F]+)?"},{begin:"#(c|C)\\("+i+" +"+i,end:"\\)"}]},b=e.inherit(e.QUOTE_STRING_MODE,{illegal:null}),g=e.COMMENT(";","$",{relevance:0}),r={begin:"\\*",end:"\\*"},t={className:"symbol",begin:"[:&]"+n},c={begin:n,relevance:0},d={begin:a},o={contains:[l,b,r,t,{begin:"\\(",end:"\\)",contains:["self",s,b,l,c]},c],variants:[{begin:"['`]\\(",end:"\\)"},{begin:"\\(quote ",end:"\\)",keywords:{name:"quote"}},{begin:"'"+a}]},v={variants:[{begin:"'"+n},{begin:"#'"+n+"(::"+n+")*"}]},m={begin:"\\(\\s*",end:"\\)"},u={endsWithParent:!0,relevance:0};return m.contains=[{className:"name",variants:[{begin:n,relevance:0},{begin:a}]},u],u.contains=[o,v,m,s,l,b,g,r,t,d,c],{name:"Lisp",illegal:/\S/,contains:[l,e.SHEBANG(),s,b,g,o,v,m,c]}}}());hljs.registerLanguage("lua",function(){"use strict";return function(e){var t="\\[=*\\[",a="\\]=*\\]",n={begin:t,end:a,contains:["self"]},o=[e.COMMENT("--(?!\\[=*\\[)","$"),e.COMMENT("--\\[=*\\[",a,{contains:[n],relevance:10})];return{name:"Lua",keywords:{$pattern:e.UNDERSCORE_IDENT_RE,literal:"true false nil",keyword:"and break do else elseif end for goto if in local not or repeat return then until while",built_in:"_G _ENV _VERSION __index __newindex __mode __call __metatable __tostring __len __gc __add __sub __mul __div __mod __pow __concat __unm __eq __lt __le assert collectgarbage dofile error getfenv getmetatable ipairs load loadfile loadstring module next pairs pcall print rawequal rawget rawset require select setfenv setmetatable tonumber tostring type unpack xpcall arg self coroutine resume yield status wrap create running debug getupvalue debug sethook getmetatable gethook setmetatable setlocal traceback setfenv getinfo setupvalue getlocal getregistry getfenv io lines write close flush open output type read stderr stdin input stdout popen tmpfile math log max acos huge ldexp pi cos tanh pow deg tan cosh sinh random randomseed frexp ceil floor rad abs sqrt modf asin min mod fmod log10 atan2 exp sin atan os exit setlocale date getenv difftime remove time clock tmpname rename execute package preload loadlib loaded loaders cpath config path seeall string sub upper len gfind rep find match char dump gmatch reverse byte format gsub lower table setn insert getn foreachi maxn foreach concat sort remove"},contains:o.concat([{className:"function",beginKeywords:"function",end:"\\)",contains:[e.inherit(e.TITLE_MODE,{begin:"([_a-zA-Z]\\w*\\.)*([_a-zA-Z]\\w*:)?[_a-zA-Z]\\w*"}),{className:"params",begin:"\\(",endsWithParent:!0,contains:o}].concat(o)},e.C_NUMBER_MODE,e.APOS_STRING_MODE,e.QUOTE_STRING_MODE,{className:"string",begin:t,end:a,contains:[n],relevance:5}])}}}());hljs.registerLanguage("matlab",function(){"use strict";return function(e){var a={relevance:0,contains:[{begin:"('|\\.')+"}]};return{name:"Matlab",keywords:{keyword:"arguments break case catch classdef continue else elseif end enumeration events for function global if methods otherwise parfor persistent properties return spmd switch try while",built_in:"sin sind sinh asin asind asinh cos cosd cosh acos acosd acosh tan tand tanh atan atand atan2 atanh sec secd sech asec asecd asech csc cscd csch acsc acscd acsch cot cotd coth acot acotd acoth hypot exp expm1 log log1p log10 log2 pow2 realpow reallog realsqrt sqrt nthroot nextpow2 abs angle complex conj imag real unwrap isreal cplxpair fix floor ceil round mod rem sign airy besselj bessely besselh besseli besselk beta betainc betaln ellipj ellipke erf erfc erfcx erfinv expint gamma gammainc gammaln psi legendre cross dot factor isprime primes gcd lcm rat rats perms nchoosek factorial cart2sph cart2pol pol2cart sph2cart hsv2rgb rgb2hsv zeros ones eye repmat rand randn linspace logspace freqspace meshgrid accumarray size length ndims numel disp isempty isequal isequalwithequalnans cat reshape diag blkdiag tril triu fliplr flipud flipdim rot90 find sub2ind ind2sub bsxfun ndgrid permute ipermute shiftdim circshift squeeze isscalar isvector ans eps realmax realmin pi i|0 inf nan isnan isinf isfinite j|0 why compan gallery hadamard hankel hilb invhilb magic pascal rosser toeplitz vander wilkinson max min nanmax nanmin mean nanmean type table readtable writetable sortrows sort figure plot plot3 scatter scatter3 cellfun legend intersect ismember procrustes hold num2cell "},illegal:'(//|"|#|/\\*|\\s+/\\w+)',contains:[{className:"function",beginKeywords:"function",end:"$",contains:[e.UNDERSCORE_TITLE_MODE,{className:"params",variants:[{begin:"\\(",end:"\\)"},{begin:"\\[",end:"\\]"}]}]},{className:"built_in",begin:/true|false/,relevance:0,starts:a},{begin:"[a-zA-Z][a-zA-Z_0-9]*('|\\.')+",relevance:0},{className:"number",begin:e.C_NUMBER_RE,relevance:0,starts:a},{className:"string",begin:"'",end:"'",contains:[e.BACKSLASH_ESCAPE,{begin:"''"}]},{begin:/\]|\}|\)/,relevance:0,starts:a},{className:"string",begin:'"',end:'"',contains:[e.BACKSLASH_ESCAPE,{begin:'""'}],starts:a},e.COMMENT("^\\s*%\\{\\s*$","^\\s*%\\}\\s*$"),e.COMMENT("%","$")]}}}());hljs.registerLanguage("r",function(){"use strict";function e(){for(var _len15=arguments.length,e=new Array(_len15),_key15=0;_key15<_len15;_key15++){e[_key15]=arguments[_key15]}return e.map(function(e){return(a=e)?"string"==typeof a?a:a.source:null;var a}).join("")}return function(a){var n=/(?:(?:[a-zA-Z]|\.[._a-zA-Z])[._a-zA-Z0-9]*)|\.(?!\d)/;return{name:"R",illegal:/->/,keywords:{$pattern:n,keyword:"function if in break next repeat else for while",literal:"NULL NA TRUE FALSE Inf NaN NA_integer_|10 NA_real_|10 NA_character_|10 NA_complex_|10",built_in:"LETTERS letters month.abb month.name pi T F abs acos acosh all any anyNA Arg as.call as.character as.complex as.double as.environment as.integer as.logical as.null.default as.numeric as.raw asin asinh atan atanh attr attributes baseenv browser c call ceiling class Conj cos cosh cospi cummax cummin cumprod cumsum digamma dim dimnames emptyenv exp expression floor forceAndCall gamma gc.time globalenv Im interactive invisible is.array is.atomic is.call is.character is.complex is.double is.environment is.expression is.finite is.function is.infinite is.integer is.language is.list is.logical is.matrix is.na is.name is.nan is.null is.numeric is.object is.pairlist is.raw is.recursive is.single is.symbol lazyLoadDBfetch length lgamma list log max min missing Mod names nargs nzchar oldClass on.exit pos.to.env proc.time prod quote range Re rep retracemem return round seq_along seq_len seq.int sign signif sin sinh sinpi sqrt standardGeneric substitute sum switch tan tanh tanpi tracemem trigamma trunc unclass untracemem UseMethod xtfrm"},compilerExtensions:[function(a,n){if(!a.beforeMatch)return;if(a.starts)throw Error("beforeMatch cannot be used with starts");var i=Object.assign({},a);Object.keys(a).forEach(function(e){delete a[e]}),a.begin=e(i.beforeMatch,e("(?=",i.begin,")")),a.starts={relevance:0,contains:[Object.assign(i,{endsParent:!0})]},a.relevance=0,delete i.beforeMatch}],contains:[a.COMMENT(/#'/,/$/,{contains:[{className:"doctag",begin:"@examples",starts:{contains:[{begin:/\n/},{begin:/#'\s*(?=@[a-zA-Z]+)/,endsParent:!0},{begin:/#'/,end:/$/,excludeBegin:!0}]}},{className:"doctag",begin:"@param",end:/$/,contains:[{className:"variable",variants:[{begin:n},{begin:/`(?:\\.|[^`\\])+`/}],endsParent:!0}]},{className:"doctag",begin:/@[a-zA-Z]+/},{className:"meta-keyword",begin:/\\[a-zA-Z]+/}]}),a.HASH_COMMENT_MODE,{className:"string",contains:[a.BACKSLASH_ESCAPE],variants:[a.END_SAME_AS_BEGIN({begin:/[rR]"(-*)\(/,end:/\)(-*)"/}),a.END_SAME_AS_BEGIN({begin:/[rR]"(-*)\{/,end:/\}(-*)"/}),a.END_SAME_AS_BEGIN({begin:/[rR]"(-*)\[/,end:/\](-*)"/}),a.END_SAME_AS_BEGIN({begin:/[rR]'(-*)\(/,end:/\)(-*)'/}),a.END_SAME_AS_BEGIN({begin:/[rR]'(-*)\{/,end:/\}(-*)'/}),a.END_SAME_AS_BEGIN({begin:/[rR]'(-*)\[/,end:/\](-*)'/}),{begin:'"',end:'"',relevance:0},{begin:"'",end:"'",relevance:0}]},{className:"number",relevance:0,beforeMatch:/([^a-zA-Z0-9._])/,variants:[{match:/0[xX][0-9a-fA-F]+\.[0-9a-fA-F]*[pP][+-]?\d+i?/},{match:/0[xX][0-9a-fA-F]+([pP][+-]?\d+)?[Li]?/},{match:/(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?[Li]?/}]},{begin:"%",end:"%"},{begin:e(/[a-zA-Z][a-zA-Z_0-9]*/,"\\s+<-\\s+")},{begin:"`",end:"`",contains:[{begin:/\\./}]}]}}}());hljs.registerLanguage("rust",function(){"use strict";return function(e){var n="([ui](8|16|32|64|128|size)|f(32|64))?",t="drop i8 i16 i32 i64 i128 isize u8 u16 u32 u64 u128 usize f32 f64 str char bool Box Option Result String Vec Copy Send Sized Sync Drop Fn FnMut FnOnce ToOwned Clone Debug PartialEq PartialOrd Eq Ord AsRef AsMut Into From Default Iterator Extend IntoIterator DoubleEndedIterator ExactSizeIterator SliceConcatExt ToString assert! assert_eq! bitflags! bytes! cfg! col! concat! concat_idents! debug_assert! debug_assert_eq! env! panic! file! format! format_args! include_bin! include_str! line! local_data_key! module_path! option_env! print! println! select! stringify! try! unimplemented! unreachable! vec! write! writeln! macro_rules! assert_ne! debug_assert_ne!";return{name:"Rust",aliases:["rs"],keywords:{$pattern:e.IDENT_RE+"!?",keyword:"abstract as async await become box break const continue crate do dyn else enum extern false final fn for if impl in let loop macro match mod move mut override priv pub ref return self Self static struct super trait true try type typeof unsafe unsized use virtual where while yield",literal:"true false Some None Ok Err",built_in:t},illegal:"</",contains:[e.C_LINE_COMMENT_MODE,e.COMMENT("/\\*","\\*/",{contains:["self"]}),e.inherit(e.QUOTE_STRING_MODE,{begin:/b?"/,illegal:null}),{className:"string",variants:[{begin:/r(#*)"(.|\n)*?"\1(?!#)/},{begin:/b?'\\?(x\w{2}|u\w{4}|U\w{8}|.)'/}]},{className:"symbol",begin:/'[a-zA-Z_][a-zA-Z0-9_]*/},{className:"number",variants:[{begin:"\\b0b([01_]+)"+n},{begin:"\\b0o([0-7_]+)"+n},{begin:"\\b0x([A-Fa-f0-9_]+)"+n},{begin:"\\b(\\d[\\d_]*(\\.[0-9_]+)?([eE][+-]?[0-9_]+)?)"+n}],relevance:0},{className:"function",beginKeywords:"fn",end:"(\\(|<)",excludeEnd:!0,contains:[e.UNDERSCORE_TITLE_MODE]},{className:"meta",begin:"#!?\\[",end:"\\]",contains:[{className:"meta-string",begin:/"/,end:/"/}]},{className:"class",beginKeywords:"type",end:";",contains:[e.inherit(e.UNDERSCORE_TITLE_MODE,{endsParent:!0})],illegal:"\\S"},{className:"class",beginKeywords:"trait enum struct union",end:/\{/,contains:[e.inherit(e.UNDERSCORE_TITLE_MODE,{endsParent:!0})],illegal:"[\\w\\d]"},{begin:e.IDENT_RE+"::",keywords:{built_in:t}},{begin:"->"}]}}}());hljs.registerLanguage("sql",function(){"use strict";function e(e){return e?"string"==typeof e?e:e.source:null}function r(){for(var _len16=arguments.length,r=new Array(_len16),_key16=0;_key16<_len16;_key16++){r[_key16]=arguments[_key16]}return r.map(function(r){return e(r)}).join("")}function t(){for(var _len17=arguments.length,r=new Array(_len17),_key17=0;_key17<_len17;_key17++){r[_key17]=arguments[_key17]}return"("+r.map(function(r){return e(r)}).join("|")+")"}return function(e){var n=e.COMMENT("--","$"),a=["true","false","unknown"],i=["bigint","binary","blob","boolean","char","character","clob","date","dec","decfloat","decimal","float","int","integer","interval","nchar","nclob","national","numeric","real","row","smallint","time","timestamp","varchar","varying","varbinary"],s=["abs","acos","array_agg","asin","atan","avg","cast","ceil","ceiling","coalesce","corr","cos","cosh","count","covar_pop","covar_samp","cume_dist","dense_rank","deref","element","exp","extract","first_value","floor","json_array","json_arrayagg","json_exists","json_object","json_objectagg","json_query","json_table","json_table_primitive","json_value","lag","last_value","lead","listagg","ln","log","log10","lower","max","min","mod","nth_value","ntile","nullif","percent_rank","percentile_cont","percentile_disc","position","position_regex","power","rank","regr_avgx","regr_avgy","regr_count","regr_intercept","regr_r2","regr_slope","regr_sxx","regr_sxy","regr_syy","row_number","sin","sinh","sqrt","stddev_pop","stddev_samp","substring","substring_regex","sum","tan","tanh","translate","translate_regex","treat","trim","trim_array","unnest","upper","value_of","var_pop","var_samp","width_bucket"],o=["create table","insert into","primary key","foreign key","not null","alter table","add constraint","grouping sets","on overflow","character set","respect nulls","ignore nulls","nulls first","nulls last","depth first","breadth first"],c=s,l=["abs","acos","all","allocate","alter","and","any","are","array","array_agg","array_max_cardinality","as","asensitive","asin","asymmetric","at","atan","atomic","authorization","avg","begin","begin_frame","begin_partition","between","bigint","binary","blob","boolean","both","by","call","called","cardinality","cascaded","case","cast","ceil","ceiling","char","char_length","character","character_length","check","classifier","clob","close","coalesce","collate","collect","column","commit","condition","connect","constraint","contains","convert","copy","corr","corresponding","cos","cosh","count","covar_pop","covar_samp","create","cross","cube","cume_dist","current","current_catalog","current_date","current_default_transform_group","current_path","current_role","current_row","current_schema","current_time","current_timestamp","current_path","current_role","current_transform_group_for_type","current_user","cursor","cycle","date","day","deallocate","dec","decimal","decfloat","declare","default","define","delete","dense_rank","deref","describe","deterministic","disconnect","distinct","double","drop","dynamic","each","element","else","empty","end","end_frame","end_partition","end-exec","equals","escape","every","except","exec","execute","exists","exp","external","extract","false","fetch","filter","first_value","float","floor","for","foreign","frame_row","free","from","full","function","fusion","get","global","grant","group","grouping","groups","having","hold","hour","identity","in","indicator","initial","inner","inout","insensitive","insert","int","integer","intersect","intersection","interval","into","is","join","json_array","json_arrayagg","json_exists","json_object","json_objectagg","json_query","json_table","json_table_primitive","json_value","lag","language","large","last_value","lateral","lead","leading","left","like","like_regex","listagg","ln","local","localtime","localtimestamp","log","log10","lower","match","match_number","match_recognize","matches","max","member","merge","method","min","minute","mod","modifies","module","month","multiset","national","natural","nchar","nclob","new","no","none","normalize","not","nth_value","ntile","null","nullif","numeric","octet_length","occurrences_regex","of","offset","old","omit","on","one","only","open","or","order","out","outer","over","overlaps","overlay","parameter","partition","pattern","per","percent","percent_rank","percentile_cont","percentile_disc","period","portion","position","position_regex","power","precedes","precision","prepare","primary","procedure","ptf","range","rank","reads","real","recursive","ref","references","referencing","regr_avgx","regr_avgy","regr_count","regr_intercept","regr_r2","regr_slope","regr_sxx","regr_sxy","regr_syy","release","result","return","returns","revoke","right","rollback","rollup","row","row_number","rows","running","savepoint","scope","scroll","search","second","seek","select","sensitive","session_user","set","show","similar","sin","sinh","skip","smallint","some","specific","specifictype","sql","sqlexception","sqlstate","sqlwarning","sqrt","start","static","stddev_pop","stddev_samp","submultiset","subset","substring","substring_regex","succeeds","sum","symmetric","system","system_time","system_user","table","tablesample","tan","tanh","then","time","timestamp","timezone_hour","timezone_minute","to","trailing","translate","translate_regex","translation","treat","trigger","trim","trim_array","true","truncate","uescape","union","unique","unknown","unnest","update   ","upper","user","using","value","values","value_of","var_pop","var_samp","varbinary","varchar","varying","versioning","when","whenever","where","width_bucket","window","with","within","without","year","add","asc","collation","desc","final","first","last","view"].filter(function(e){return!s.includes(e)}),u={begin:r(/\b/,t.apply(void 0,c),/\s*\(/),keywords:{built_in:c.join(" ")}};return{name:"SQL",case_insensitive:!0,illegal:/[{}]|<\//,keywords:{$pattern:/\b[\w\.]+/,keyword:function(e){var _ref11=arguments.length>1&&arguments[1]!==undefined?arguments[1]:{},r=_ref11.exceptions,t=_ref11.when;var n=t;return r=r||[],e.map(function(e){return e.match(/\|\d+$/)||r.includes(e)?e:n(e)?e+"|0":e})}(l,{when:function when(e){return e.length<3}}).join(" "),literal:a.join(" "),type:i.join(" "),built_in:"current_catalog current_date current_default_transform_group current_path current_role current_schema current_transform_group_for_type current_user session_user system_time system_user current_time localtime current_timestamp localtimestamp"},contains:[{begin:t.apply(void 0,o),keywords:{$pattern:/[\w\.]+/,keyword:l.concat(o).join(" "),literal:a.join(" "),type:i.join(" ")}},{className:"type",begin:t("double precision","large object","with timezone","without timezone")},u,{className:"variable",begin:/@[a-z0-9]+/},{className:"string",variants:[{begin:/'/,end:/'/,contains:[{begin:/''/}]}]},{begin:/"/,end:/"/,contains:[{begin:/""/}]},e.C_NUMBER_MODE,e.C_BLOCK_COMMENT_MODE,n,{className:"operator",begin:/[-+*/=%^~]|&&?|\|\|?|!=?|<(?:=>?|<|>)?|>[>=]?/,relevance:0}]}}}());hljs.registerLanguage("scheme",function(){"use strict";return function(e){var t="[^\\(\\)\\[\\]\\{\\}\",'`;#|\\\\\\s]+",n={$pattern:t,"builtin-name":"case-lambda call/cc class define-class exit-handler field import inherit init-field interface let*-values let-values let/ec mixin opt-lambda override protect provide public rename require require-for-syntax syntax syntax-case syntax-error unit/sig unless when with-syntax and begin call-with-current-continuation call-with-input-file call-with-output-file case cond define define-syntax delay do dynamic-wind else for-each if lambda let let* let-syntax letrec letrec-syntax map or syntax-rules ' * + , ,@ - ... / ; < <= = => > >= ` abs acos angle append apply asin assoc assq assv atan boolean? caar cadr call-with-input-file call-with-output-file call-with-values car cdddar cddddr cdr ceiling char->integer char-alphabetic? char-ci<=? char-ci<? char-ci=? char-ci>=? char-ci>? char-downcase char-lower-case? char-numeric? char-ready? char-upcase char-upper-case? char-whitespace? char<=? char<? char=? char>=? char>? char? close-input-port close-output-port complex? cons cos current-input-port current-output-port denominator display eof-object? eq? equal? eqv? eval even? exact->inexact exact? exp expt floor force gcd imag-part inexact->exact inexact? input-port? integer->char integer? interaction-environment lcm length list list->string list->vector list-ref list-tail list? load log magnitude make-polar make-rectangular make-string make-vector max member memq memv min modulo negative? newline not null-environment null? number->string number? numerator odd? open-input-file open-output-file output-port? pair? peek-char port? positive? procedure? quasiquote quote quotient rational? rationalize read read-char real-part real? remainder reverse round scheme-report-environment set! set-car! set-cdr! sin sqrt string string->list string->number string->symbol string-append string-ci<=? string-ci<? string-ci=? string-ci>=? string-ci>? string-copy string-fill! string-length string-ref string-set! string<=? string<? string=? string>=? string>? string? substring symbol->string symbol? tan transcript-off transcript-on truncate values vector vector->list vector-fill! vector-length vector-ref vector-set! with-input-from-file with-output-to-file write write-char zero?"},r={className:"literal",begin:"(#t|#f|#\\\\"+t+"|#\\\\.)"},a={className:"number",variants:[{begin:"(-|\\+)?\\d+([./]\\d+)?",relevance:0},{begin:"(-|\\+)?\\d+([./]\\d+)?[+\\-](-|\\+)?\\d+([./]\\d+)?i",relevance:0},{begin:"#b[0-1]+(/[0-1]+)?"},{begin:"#o[0-7]+(/[0-7]+)?"},{begin:"#x[0-9a-f]+(/[0-9a-f]+)?"}]},i=e.QUOTE_STRING_MODE,c=[e.COMMENT(";","$",{relevance:0}),e.COMMENT("#\\|","\\|#")],s={begin:t,relevance:0},l={className:"symbol",begin:"'"+t},o={endsWithParent:!0,relevance:0},g={variants:[{begin:/'/},{begin:"`"}],contains:[{begin:"\\(",end:"\\)",contains:["self",r,i,a,s,l]}]},u={className:"name",relevance:0,begin:t,keywords:n},d={variants:[{begin:"\\(",end:"\\)"},{begin:"\\[",end:"\\]"}],contains:[{begin:/lambda/,endsWithParent:!0,returnBegin:!0,contains:[u,{endsParent:!0,variants:[{begin:/\(/,end:/\)/},{begin:/\[/,end:/\]/}],contains:[s]}]},u,o]};return o.contains=[r,a,i,s,l,g,d].concat(c),{name:"Scheme",illegal:/\S/,contains:[e.SHEBANG(),a,i,l,g,d].concat(c)}}}());hljs.registerLanguage("swift",function(){"use strict";function e(e){return e?"string"==typeof e?e:e.source:null}function n(e){return a("(?=",e,")")}function a(){for(var _len18=arguments.length,n=new Array(_len18),_key18=0;_key18<_len18;_key18++){n[_key18]=arguments[_key18]}return n.map(function(n){return e(n)}).join("")}function t(){for(var _len19=arguments.length,n=new Array(_len19),_key19=0;_key19<_len19;_key19++){n[_key19]=arguments[_key19]}return"("+n.map(function(n){return e(n)}).join("|")+")"}var i=function i(e){return a(/\b/,e,/\w$/.test(e)?/\b/:/\B/)},s=["Protocol","Type"].map(i),u=["init","self"].map(i),c=["Any","Self"],r=["associatedtype",/as\?/,/as!/,"as","break","case","catch","class","continue","convenience","default","defer","deinit","didSet","do","dynamic","else","enum","extension","fallthrough",/fileprivate\(set\)/,"fileprivate","final","for","func","get","guard","if","import","indirect","infix",/init\?/,/init!/,"inout",/internal\(set\)/,"internal","in","is","lazy","let","mutating","nonmutating",/open\(set\)/,"open","operator","optional","override","postfix","precedencegroup","prefix",/private\(set\)/,"private","protocol",/public\(set\)/,"public","repeat","required","rethrows","return","set","some","static","struct","subscript","super","switch","throws","throw",/try\?/,/try!/,"try","typealias",/unowned\(safe\)/,/unowned\(unsafe\)/,"unowned","var","weak","where","while","willSet"],o=["false","nil","true"],l=["#colorLiteral","#column","#dsohandle","#else","#elseif","#endif","#error","#file","#fileID","#fileLiteral","#filePath","#function","#if","#imageLiteral","#keyPath","#line","#selector","#sourceLocation","#warn_unqualified_access","#warning"],m=["abs","all","any","assert","assertionFailure","debugPrint","dump","fatalError","getVaList","isKnownUniquelyReferenced","max","min","numericCast","pointwiseMax","pointwiseMin","precondition","preconditionFailure","print","readLine","repeatElement","sequence","stride","swap","swift_unboxFromSwiftValueWithType","transcode","type","unsafeBitCast","unsafeDowncast","withExtendedLifetime","withUnsafeMutablePointer","withUnsafePointer","withVaList","withoutActuallyEscaping","zip"],p=t(/[/=\-+!*%<>&|^~?]/,/[\u00A1-\u00A7]/,/[\u00A9\u00AB]/,/[\u00AC\u00AE]/,/[\u00B0\u00B1]/,/[\u00B6\u00BB\u00BF\u00D7\u00F7]/,/[\u2016-\u2017]/,/[\u2020-\u2027]/,/[\u2030-\u203E]/,/[\u2041-\u2053]/,/[\u2055-\u205E]/,/[\u2190-\u23FF]/,/[\u2500-\u2775]/,/[\u2794-\u2BFF]/,/[\u2E00-\u2E7F]/,/[\u3001-\u3003]/,/[\u3008-\u3020]/,/[\u3030]/),F=t(p,/[\u0300-\u036F]/,/[\u1DC0-\u1DFF]/,/[\u20D0-\u20FF]/,/[\uFE00-\uFE0F]/,/[\uFE20-\uFE2F]/),d=a(p,F,"*"),b=t(/[a-zA-Z_]/,/[\u00A8\u00AA\u00AD\u00AF\u00B2-\u00B5\u00B7-\u00BA]/,/[\u00BC-\u00BE\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u00FF]/,/[\u0100-\u02FF\u0370-\u167F\u1681-\u180D\u180F-\u1DBF]/,/[\u1E00-\u1FFF]/,/[\u200B-\u200D\u202A-\u202E\u203F-\u2040\u2054\u2060-\u206F]/,/[\u2070-\u20CF\u2100-\u218F\u2460-\u24FF\u2776-\u2793]/,/[\u2C00-\u2DFF\u2E80-\u2FFF]/,/[\u3004-\u3007\u3021-\u302F\u3031-\u303F\u3040-\uD7FF]/,/[\uF900-\uFD3D\uFD40-\uFDCF\uFDF0-\uFE1F\uFE30-\uFE44]/,/[\uFE47-\uFEFE\uFF00-\uFFFD]/),f=t(b,/\d/,/[\u0300-\u036F\u1DC0-\u1DFF\u20D0-\u20FF\uFE20-\uFE2F]/),h=a(b,f,"*"),w=a(/[A-Z]/,f,"*"),y=["autoclosure",a(/convention\(/,t("swift","block","c"),/\)/),"discardableResult","dynamicCallable","dynamicMemberLookup","escaping","frozen","GKInspectable","IBAction","IBDesignable","IBInspectable","IBOutlet","IBSegueAction","inlinable","main","nonobjc","NSApplicationMain","NSCopying","NSManaged",a(/objc\(/,h,/\)/),"objc","objcMembers","propertyWrapper","requires_stored_property_inits","testable","UIApplicationMain","unknown","usableFromInline"],g=["iOS","iOSApplicationExtension","macOS","macOSApplicationExtension","macCatalyst","macCatalystApplicationExtension","watchOS","watchOSApplicationExtension","tvOS","tvOSApplicationExtension","swift"];return function(e){var p={match:/\s+/,relevance:0},b=e.COMMENT("/\\*","\\*/",{contains:["self"]}),E=[e.C_LINE_COMMENT_MODE,b],v={className:"keyword",begin:a(/\./,n(t.apply(void 0,_toConsumableArray(s).concat(_toConsumableArray(u))))),end:t.apply(void 0,_toConsumableArray(s).concat(_toConsumableArray(u))),excludeBegin:!0},A={match:a(/\./,t.apply(void 0,r)),relevance:0},N=r.filter(function(e){return"string"==typeof e}).concat(["_|0"]),C={variants:[{className:"keyword",match:t.apply(void 0,_toConsumableArray(r.filter(function(e){return"string"!=typeof e}).concat(c).map(i)).concat(_toConsumableArray(u)))}]},D={$pattern:t(/\b\w+/,/#\w+/),keyword:N.concat(l).join(" "),literal:o.join(" ")},B=[v,A,C],_=[{match:a(/\./,t.apply(void 0,m)),relevance:0},{className:"built_in",match:a(/\b/,t.apply(void 0,m),/(?=\()/)}],k={match:/->/,relevance:0},S=[k,{className:"operator",relevance:0,variants:[{match:d},{match:"\\.(\\.|".concat(F,")+")}]}],M="([0-9a-fA-F]_*)+",x={className:"number",relevance:0,variants:[{match:"\\b(([0-9]_*)+)(\\.(([0-9]_*)+))?([eE][+-]?(([0-9]_*)+))?\\b"},{match:"\\b0x(".concat(M,")(\\.(").concat(M,"))?([pP][+-]?(([0-9]_*)+))?\\b")},{match:/\b0o([0-7]_*)+\b/},{match:/\b0b([01]_*)+\b/}]},I=function I(){var e=arguments.length>0&&arguments[0]!==undefined?arguments[0]:"";return{className:"subst",variants:[{match:a(/\\/,e,/[0\\tnr"']/)},{match:a(/\\/,e,/u\{[0-9a-fA-F]{1,8}\}/)}]}},L=function L(){var e=arguments.length>0&&arguments[0]!==undefined?arguments[0]:"";return{className:"subst",match:a(/\\/,e,/[\t ]*(?:[\r\n]|\r\n)/)}},O=function O(){var e=arguments.length>0&&arguments[0]!==undefined?arguments[0]:"";return{className:"subst",label:"interpol",begin:a(/\\/,e,/\(/),end:/\)/}},$=function $(){var e=arguments.length>0&&arguments[0]!==undefined?arguments[0]:"";return{begin:a(e,/"""/),end:a(/"""/,e),contains:[I(e),L(e),O(e)]}},j=function j(){var e=arguments.length>0&&arguments[0]!==undefined?arguments[0]:"";return{begin:a(e,/"/),end:a(/"/,e),contains:[I(e),O(e)]}},P={className:"string",variants:[$(),$("#"),$("##"),$("###"),j(),j("#"),j("##"),j("###")]},K={match:a(/`/,h,/`/)},T=[K,{className:"variable",match:/\$\d+/},{className:"variable",match:"\\$".concat(f,"+")}],z=[{match:/(@|#)available/,className:"keyword",starts:{contains:[{begin:/\(/,end:/\)/,keywords:g.join(" "),contains:[].concat(S,[x,P])}]}},{className:"keyword",match:a(/@/,t.apply(void 0,y))},{className:"meta",match:a(/@/,h)}],q={match:n(/\b[A-Z]/),relevance:0,contains:[{className:"type",match:a(/(AV|CA|CF|CG|CI|CL|CM|CN|CT|MK|MP|MTK|MTL|NS|SCN|SK|UI|WK|XC)/,f,"+")},{className:"type",match:w,relevance:0},{match:/[?!]+/,relevance:0},{match:/\.\.\./,relevance:0},{match:a(/\s+&\s+/,n(w)),relevance:0}]},U={begin:/</,end:/>/,keywords:D,contains:[].concat(E,B,z,[k,q])};q.contains.push(U);var Z={begin:/\(/,end:/\)/,relevance:0,keywords:D,contains:["self",{match:a(h,/\s*:/),keywords:"_|0",relevance:0}].concat(E,B,_,S,[x,P],T,z,[q])},V={beginKeywords:"func",contains:[{className:"title",match:t(K.match,h,d),endsParent:!0,relevance:0},p]},W={begin:/</,end:/>/,contains:[].concat(E,[q])},G={begin:/\(/,end:/\)/,keywords:D,contains:[{begin:t(n(a(h,/\s*:/)),n(a(h,/\s+/,h,/\s*:/))),end:/:/,relevance:0,contains:[{className:"keyword",match:/\b_\b/},{className:"params",match:h}]}].concat(E,B,S,[x,P],z,[q,Z]),endsParent:!0,illegal:/["']/},R={className:"function",match:n(/\bfunc\b/),contains:[V,W,G,p],illegal:[/\[/,/%/]},X={className:"function",match:/\b(subscript|init[?!]?)\s*(?=[<(])/,keywords:{keyword:"subscript init init? init!",$pattern:/\w+[?!]?/},contains:[W,G,p],illegal:/\[|%/};var _iterator=_createForOfIteratorHelper(P.variants),_step;try{for(_iterator.s();!(_step=_iterator.n()).done;){var _e7=_step.value;var _n4=_e7.contains.find(function(e){return"interpol"===e.label});_n4.keywords=D;var _a5=[].concat(B,_,S,[x,P],T);_n4.contains=[].concat(_toConsumableArray(_a5),[{begin:/\(/,end:/\)/,contains:["self"].concat(_toConsumableArray(_a5))}])}}catch(err){_iterator.e(err)}finally{_iterator.f()}return{name:"Swift",keywords:D,contains:[].concat(E,[R,X,{className:"class",beginKeywords:"struct protocol class extension enum",end:"\\{",excludeEnd:!0,keywords:D,contains:[e.inherit(e.TITLE_MODE,{begin:/[A-Za-z$_][\u00C0-\u02B80-9A-Za-z$_]*/})].concat(B)},{beginKeywords:"import",end:/$/,contains:[].concat(E),relevance:0}],B,_,S,[x,P],T,z,[q,Z])}}}());hljs.registerLanguage("typescript",function(){"use strict";var e="[A-Za-z$_][0-9A-Za-z$_]*",n=["as","in","of","if","for","while","finally","var","new","function","do","return","void","else","break","catch","instanceof","with","throw","case","default","try","switch","continue","typeof","delete","let","yield","const","class","debugger","async","await","static","import","from","export","extends"],a=["true","false","null","undefined","NaN","Infinity"],s=[].concat(["setInterval","setTimeout","clearInterval","clearTimeout","require","exports","eval","isFinite","isNaN","parseFloat","parseInt","decodeURI","decodeURIComponent","encodeURI","encodeURIComponent","escape","unescape"],["arguments","this","super","console","window","document","localStorage","module","global"],["Intl","DataView","Number","Math","Date","String","RegExp","Object","Function","Boolean","Error","Symbol","Set","Map","WeakSet","WeakMap","Proxy","Reflect","JSON","Promise","Float64Array","Int16Array","Int32Array","Int8Array","Uint16Array","Uint32Array","Float32Array","Array","Uint8Array","Uint8ClampedArray","ArrayBuffer"],["EvalError","InternalError","RangeError","ReferenceError","SyntaxError","TypeError","URIError"]);function t(e){return i("(?=",e,")")}function i(){for(var _len20=arguments.length,e=new Array(_len20),_key20=0;_key20<_len20;_key20++){e[_key20]=arguments[_key20]}return e.map(function(e){return(n=e)?"string"==typeof n?n:n.source:null;var n}).join("")}return function(r){var c={$pattern:e,keyword:n.concat(["type","namespace","typedef","interface","public","private","protected","implements","declare","abstract","readonly"]).join(" "),literal:a.join(" "),built_in:s.concat(["any","void","number","boolean","string","object","never","enum"]).join(" ")},o={className:"meta",begin:"@[A-Za-z$_][0-9A-Za-z$_]*"},l=function l(e,n,a){var s=e.contains.findIndex(function(e){return e.label===n});if(-1===s)throw Error("can not find mode to replace");e.contains.splice(s,1,a)},b=function(r){var c=e,o={begin:/<[A-Za-z0-9\\._:-]+/,end:/\/[A-Za-z0-9\\._:-]+>|\/>/,isTrulyOpeningTag:function isTrulyOpeningTag(e,n){var a=e[0].length+e.index,s=e.input[a];"<"!==s?">"===s&&(function(e,_ref12){var n=_ref12.after;var a="</"+e[0].slice(1);return-1!==e.input.indexOf(a,n)}(e,{after:a})||n.ignoreMatch()):n.ignoreMatch()}},l={$pattern:e,keyword:n.join(" "),literal:a.join(" "),built_in:s.join(" ")},b="\\.([0-9](_?[0-9])*)",d="0|[1-9](_?[0-9])*|0[0-7]*[89][0-9]*",g={className:"number",variants:[{begin:"(\\b(".concat(d,")((").concat(b,")|\\.)?|(").concat(b,"))[eE][+-]?([0-9](_?[0-9])*)\\b")},{begin:"\\b(".concat(d,")\\b((").concat(b,")\\b|\\.)?|(").concat(b,")\\b")},{begin:"\\b(0|[1-9](_?[0-9])*)n\\b"},{begin:"\\b0[xX][0-9a-fA-F](_?[0-9a-fA-F])*n?\\b"},{begin:"\\b0[bB][0-1](_?[0-1])*n?\\b"},{begin:"\\b0[oO][0-7](_?[0-7])*n?\\b"},{begin:"\\b0[0-7]+n?\\b"}],relevance:0},u={className:"subst",begin:"\\$\\{",end:"\\}",keywords:l,contains:[]},E={begin:"html`",end:"",starts:{end:"`",returnEnd:!1,contains:[r.BACKSLASH_ESCAPE,u],subLanguage:"xml"}},m={begin:"css`",end:"",starts:{end:"`",returnEnd:!1,contains:[r.BACKSLASH_ESCAPE,u],subLanguage:"css"}},_={className:"string",begin:"`",end:"`",contains:[r.BACKSLASH_ESCAPE,u]},y={className:"comment",variants:[r.COMMENT(/\/\*\*(?!\/)/,"\\*/",{relevance:0,contains:[{className:"doctag",begin:"@[A-Za-z]+",contains:[{className:"type",begin:"\\{",end:"\\}",relevance:0},{className:"variable",begin:c+"(?=\\s*(-)|$)",endsParent:!0,relevance:0},{begin:/(?=[^\n])\s/,relevance:0}]}]}),r.C_BLOCK_COMMENT_MODE,r.C_LINE_COMMENT_MODE]},p=[r.APOS_STRING_MODE,r.QUOTE_STRING_MODE,E,m,_,g,r.REGEXP_MODE];u.contains=p.concat({begin:/\{/,end:/\}/,keywords:l,contains:["self"].concat(p)});var N=[].concat(y,u.contains),f=N.concat([{begin:/\(/,end:/\)/,keywords:l,contains:["self"].concat(N)}]),A={className:"params",begin:/\(/,end:/\)/,excludeBegin:!0,excludeEnd:!0,keywords:l,contains:f};return{name:"Javascript",aliases:["js","jsx","mjs","cjs"],keywords:l,exports:{PARAMS_CONTAINS:f},illegal:/#(?![$_A-z])/,contains:[r.SHEBANG({label:"shebang",binary:"node",relevance:5}),{label:"use_strict",className:"meta",relevance:10,begin:/^\s*['"]use (strict|asm)['"]/},r.APOS_STRING_MODE,r.QUOTE_STRING_MODE,E,m,_,y,g,{begin:i(/[{,\n]\s*/,t(i(/(((\/\/.*$)|(\/\*(\*[^/]|[^*])*\*\/))\s*)*/,c+"\\s*:"))),relevance:0,contains:[{className:"attr",begin:c+t("\\s*:"),relevance:0}]},{begin:"("+r.RE_STARTERS_RE+"|\\b(case|return|throw)\\b)\\s*",keywords:"return throw case",contains:[y,r.REGEXP_MODE,{className:"function",begin:"(\\([^()]*(\\([^()]*(\\([^()]*\\)[^()]*)*\\)[^()]*)*\\)|"+r.UNDERSCORE_IDENT_RE+")\\s*=>",returnBegin:!0,end:"\\s*=>",contains:[{className:"params",variants:[{begin:r.UNDERSCORE_IDENT_RE,relevance:0},{className:null,begin:/\(\s*\)/,skip:!0},{begin:/\(/,end:/\)/,excludeBegin:!0,excludeEnd:!0,keywords:l,contains:f}]}]},{begin:/,/,relevance:0},{className:"",begin:/\s/,end:/\s*/,skip:!0},{variants:[{begin:"<>",end:"</>"},{begin:o.begin,"on:begin":o.isTrulyOpeningTag,end:o.end}],subLanguage:"xml",contains:[{begin:o.begin,end:o.end,skip:!0,contains:["self"]}]}],relevance:0},{className:"function",beginKeywords:"function",end:/[{;]/,excludeEnd:!0,keywords:l,contains:["self",r.inherit(r.TITLE_MODE,{begin:c}),A],illegal:/%/},{beginKeywords:"while if switch catch for"},{className:"function",begin:r.UNDERSCORE_IDENT_RE+"\\([^()]*(\\([^()]*(\\([^()]*\\)[^()]*)*\\)[^()]*)*\\)\\s*\\{",returnBegin:!0,contains:[A,r.inherit(r.TITLE_MODE,{begin:c})]},{variants:[{begin:"\\."+c},{begin:"\\$"+c}],relevance:0},{className:"class",beginKeywords:"class",end:/[{;=]/,excludeEnd:!0,illegal:/[:"[\]]/,contains:[{beginKeywords:"extends"},r.UNDERSCORE_TITLE_MODE]},{begin:/\b(?=constructor)/,end:/[{;]/,excludeEnd:!0,contains:[r.inherit(r.TITLE_MODE,{begin:c}),"self",A]},{begin:"(get|set)\\s+(?="+c+"\\()",end:/\{/,keywords:"get set",contains:[r.inherit(r.TITLE_MODE,{begin:c}),{begin:/\(\)/},A]},{begin:/\$[(.]/}]}}(r);return Object.assign(b.keywords,c),b.exports.PARAMS_CONTAINS.push(o),b.contains=b.contains.concat([o,{beginKeywords:"namespace",end:/\{/,excludeEnd:!0},{beginKeywords:"interface",end:/\{/,excludeEnd:!0,keywords:"interface extends"}]),l(b,"shebang",r.SHEBANG()),l(b,"use_strict",{className:"meta",relevance:10,begin:/^\s*['"]use strict['"]/}),b.contains.find(function(e){return"function"===e.className}).relevance=0,Object.assign(b,{name:"TypeScript",aliases:["ts"]}),b}}());hljs.registerLanguage("yaml",function(){"use strict";return function(e){var n="true false yes no null",a="[\\w#;/?:@&=+$,.~*'()[\\]]+",s={className:"string",relevance:0,variants:[{begin:/'/,end:/'/},{begin:/"/,end:/"/},{begin:/\S+/}],contains:[e.BACKSLASH_ESCAPE,{className:"template-variable",variants:[{begin:/\{\{/,end:/\}\}/},{begin:/%\{/,end:/\}/}]}]},i=e.inherit(s,{variants:[{begin:/'/,end:/'/},{begin:/"/,end:/"/},{begin:/[^\s,{}[\]]+/}]}),l={end:",",endsWithParent:!0,excludeEnd:!0,contains:[],keywords:n,relevance:0},t={begin:/\{/,end:/\}/,contains:[l],illegal:"\\n",relevance:0},g={begin:"\\[",end:"\\]",contains:[l],illegal:"\\n",relevance:0},b=[{className:"attr",variants:[{begin:"\\w[\\w :\\/.-]*:(?=[ \t]|$)"},{begin:'"\\w[\\w :\\/.-]*":(?=[ \t]|$)'},{begin:"'\\w[\\w :\\/.-]*':(?=[ \t]|$)"}]},{className:"meta",begin:"^---\\s*$",relevance:10},{className:"string",begin:"[\\|>]([1-9]?[+-])?[ ]*\\n( +)[^ ][^\\n]*\\n(\\2[^\\n]+\\n?)*"},{begin:"<%[%=-]?",end:"[%-]?%>",subLanguage:"ruby",excludeBegin:!0,excludeEnd:!0,relevance:0},{className:"type",begin:"!\\w+!"+a},{className:"type",begin:"!<"+a+">"},{className:"type",begin:"!"+a},{className:"type",begin:"!!"+a},{className:"meta",begin:"&"+e.UNDERSCORE_IDENT_RE+"$"},{className:"meta",begin:"\\*"+e.UNDERSCORE_IDENT_RE+"$"},{className:"bullet",begin:"-(?=[ ]|$)",relevance:0},e.HASH_COMMENT_MODE,{beginKeywords:n,keywords:{literal:n}},{className:"number",begin:"\\b[0-9]{4}(-[0-9][0-9]){0,2}([Tt \\t][0-9][0-9]?(:[0-9][0-9]){2})?(\\.[0-9]*)?([ \\t])*(Z|[-+][0-9][0-9]?(:[0-9][0-9])?)?\\b"},{className:"number",begin:e.C_NUMBER_RE+"\\b",relevance:0},t,g,s],r=[].concat(b);return r.pop(),r.push(i),l.contains=r,{name:"YAML",case_insensitive:!0,aliases:["yml","YAML"],contains:b}}}());hljs.registerLanguage("plaintext",function(){"use strict";return function(t){return{name:"Plain text",aliases:["text","txt"],disableAutodetect:!0}}}());hljs.registerLanguage("pyxlscript",function(){"use strict";return function(_){var e={keyword:"⌊ ⌋ ⌈ ⌉ | ‖ ∊ ∈ default bitor pad joy bitxor bitand and because at local in if then for return while mod preserving_transform|10 else continue let|2 const break not with assert with debug_watch draw_sprite draw_text debug_print|4 while continue or nil|2 pi nan infinity true false ∅ IDE_USER ξ size ray_intersect ray_intersect_map up_y draw_bounds draw_disk reset_clip reset_transform set_clip draw_line draw_sprite_corner_rect intersect_clip draw_point draw_corner_rect reset_camera set_camera get_camera draw_rect get_background set_background text_width get_sprite_pixel_color draw_sprite draw_text draw_tri draw_poly get_transform get_clip rotation_sign sign_nonzero set_transform angle_to_xy xy_to_angle xy xz_to_xyz xy_to_xyz xz xyz any_button_press any_button_release draw_map draw_map_span get_mode get_previous_mode get_map_pixel_color get_map_pixel_color_by_ws_coord get_map_sprite set_map_sprite get_map_sprite_by_ws_coord set_map_sprite_by_ws_coord parse unparse format_number uppercase lowercase xyz_to_rgb rgb_to_xyz values trim_spaces array_value play_sound resume_sound stop_audio game_frames mode_frames delay sequence add_frame_hook make_spline remove_frame_hook make_entity draw_entity overlaps entity_area entity_remove_all entity_add_child entity_remove_child entity_update_children entity_simulate split now game_frames mode_frames replace starts_with ends_with find_map_path find_path join entity_apply_force entity_apply_impulse perp gray rgb rgba hsv hsva last_value last_key insert reverse reversed call set_post_effects get_post_effects reset_post_effects push_front local_time device_control physics_add_contact_callback physics_entity_contacts physics_entity_has_contacts physics_add_entity physics_remove_entity physics_remove_all physics_attach physics_detach make_physics make_contact_group draw_physics physics_simulate entity_inertia entity_mass entity_move resume_audio get_audio_status min max mid abs acos atan asin sign sign_nonzero cos clamp hash lerp lerp_angle perceptual_lerp_color log log2 log10 noise oscillate overlap pow make_random random_sign random_integer random_within_cube random_within_sphere random_on_sphere random_within_circle random_within_square random_on_square random_on_cube random_on_circle random_direction2D random_direction3D random_value random_gaussian random_gaussian2D random_truncated_gaussian random_truncated_gaussian2D random ξ smoothstep smootherstep sgn sqrt cbrt sin set_random_seed tan conncatenate extend extended deep_clone clone copy draw_previous_mode cross direction dot equivalent magnitude magnitude_squared max_component min_component xy_to_xz xz_to_xy xy xyz sorted slice loop set_pause_menu iterate fast_remove_key find keys remove_key substring shuffle shuffled sort resize push pop pop_front push_front fast_remove_value remove_values remove_all gamepad_array touch joy round floor ceil debug_print get_sound_status set_playback_rate set_pitch set_volume set_pan set_loop debug_pause todo load_local save_local is_string is_function is_NaN is_object is_nil is_boolean is_number is_array type remove_frame_hooks_by_mode transform_cs_z_to_ss_z transform_ss_z_to_cs_z transform_ws_z_to_map_layer rgb_to_xyz axis_aligned_draw_box transform_map_layer_to_ws_z transform_map_space_to_ws transform_ws_to_map_space transform_ss_to_ws transform_ws_to_ss transform_cs_to_ss transform_ss_to_cs transform_cs_to_ws transform_ws_to_cs transform_es_to_ws transform_ws_to_ws transform_to_parent transform_to_child transform_es_to_sprite_space transform_sprite_space_to_es transform_to transform_from compose_transform ABS ADD DIV MAD MUL SUB MAX MIN SIGN CLAMP LERP RGB_ADD_RGB RGB_SUB_RGB RGB_MUL_RGB RGB_DIV_RGB RGB_MUL RGB_DIV RGB_DOT_RGB RGB_LERP RGBA_ADD_RGBA RGBA_SUB_RGBA RGBA_MUL_RGBA RGBA_DIV_RGBA RGBA_MUL RGBA_DIV RGBA_DOT_RGBA RGBA_LERP XY_DIRECTION XY_ADD_XY XY_SUB_XY XY_MUL_XY XY_DIV_XY XY_MUL XY_DIV XY_DOT_XY XY_CRS_XY XZ_DIRECTION XZ_ADD_XZ XZ_SUB_XZ XZ_MUL_XZ XZ_DIV_XZ XZ_MUL XZ_DIV XZ_DOT_XZ XYZ_DIRECTION XYZ_ADD_XYZ XYZ_SUB_XYZ XYZ_MUL_XYZ XYZ_DIV_XYZ XYZ_MUL XYZ_DIV XYZ_DOT_XYZ XYZ_CRS_XYZ MAT2x2_MATMUL_XY MAT2x2_MATMUL_XZ MAT3x3_MATMUL_XYZ MAT3x4_MATMUL_XYZ MAT3x4_MATMUL_XYZW"},s={className:"subst",begin:/\{/,end:/\}/,keywords:e},t={className:"string",contains:[_.BACKSLASH_ESCAPE],variants:[{begin:/(u|r|ur)"/,end:/"/,relevance:10},{begin:/(b|br)"/,end:/"/},{begin:/(fr|rf|f)"/,end:/"/,contains:[_.BACKSLASH_ESCAPE,s]},_.QUOTE_STRING_MODE]},a={className:"number",relevance:0,variants:[{begin:/\u2205|[+-]?[\u221e\u03b5\u03c0\xbd\u2153\u2154\xbc\xbe\u2155\u2156\u2157\u2158\u2159\u2150\u215b\u2151\u2152`]/},{begin:/#[0-7a-fA-F]+/},{begin:/[+-]?(\d*\.)?\d+(%|deg|\xb0)?/},{begin:/[\u2080\u2081\u2082\u2083\u2084\u2085\u2086\u2087\u2088\u2089\u2070\xb9\xb2\xb3\u2074\u2075\u2076\u2077\u2078\u2079]/}]},r={className:"params",begin:/\(/,end:/\)/,contains:[a,t]};return s.contains=[t,a],{aliases:["pyxlscript"],keywords:e,illegal:/(<\/|->|\?)|=>|@|\$/,contains:[{className:"section",relevance:10,variants:[{begin:/^[^\n]+?\\n(-|\u2500|\u2014|\u2501|\u23af|=|\u2550|\u268c){5,}/}]},a,t,_.C_LINE_COMMENT_MODE,_.C_BLOCK_COMMENT_MODE,{variants:[{className:"function",beginKeywords:"def"}],end:/:/,illegal:/[${=;\n,]/,contains:[_.UNDERSCORE_TITLE_MODE,r,{begin:/->/,endsWithParent:!0,keywords:"None"}]}]}}}());
/*****************************************************************************************/

// The following is for emacs. It must be at the end of the file and is
// needed to preserve the BOM mark when editing in emacs. The begin and
// end comment on each line are also required.
 
/* Local Variables: */
/* mode: JavaScript */
/* coding: utf-8-with-signature */
/* End: */

