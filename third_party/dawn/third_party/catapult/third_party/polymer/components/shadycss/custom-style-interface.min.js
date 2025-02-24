(function(){/*

Copyright (c) 2017 The Polymer Project Authors. All rights reserved.
This code may only be used under the BSD style license found at http://polymer.github.io/LICENSE.txt
The complete set of authors may be found at http://polymer.github.io/AUTHORS.txt
The complete set of contributors may be found at http://polymer.github.io/CONTRIBUTORS.txt
Code distributed by Google as part of the polymer project is also
subject to an additional IP rights grant found at http://polymer.github.io/PATENTS.txt
*/
'use strict';var c=null,f=window.HTMLImports&&window.HTMLImports.whenReady||null,g;function h(a){requestAnimationFrame(function(){f?f(a):(c||(c=new Promise(function(a){g=a}),"complete"===document.readyState?g():document.addEventListener("readystatechange",function(){"complete"===document.readyState&&g()})),c.then(function(){a&&a()}))})};var k=null,l=null;function m(){this.customStyles=[];this.enqueued=!1;h(function(){window.ShadyCSS.flushCustomStyles&&window.ShadyCSS.flushCustomStyles()})}function n(a){!a.enqueued&&l&&(a.enqueued=!0,h(l))}m.prototype.c=function(a){a.__seenByShadyCSS||(a.__seenByShadyCSS=!0,this.customStyles.push(a),n(this))};m.prototype.b=function(a){if(a.__shadyCSSCachedStyle)return a.__shadyCSSCachedStyle;var b;a.getStyle?b=a.getStyle():b=a;return b};
m.prototype.a=function(){for(var a=this.customStyles,b=0;b<a.length;b++){var d=a[b];if(!d.__shadyCSSCachedStyle){var e=this.b(d);e&&(e=e.__appliedElement||e,k&&k(e),d.__shadyCSSCachedStyle=e)}}return a};m.prototype.addCustomStyle=m.prototype.c;m.prototype.getStyleForCustomStyle=m.prototype.b;m.prototype.processStyles=m.prototype.a;
Object.defineProperties(m.prototype,{transformCallback:{get:function(){return k},set:function(a){k=a}},validateCallback:{get:function(){return l},set:function(a){var b=!1;l||(b=!0);l=a;b&&n(this)}}});function p(a,b){for(var d in b)null===d?a.style.removeProperty(d):a.style.setProperty(d,b[d])};var q=!(window.ShadyDOM&&window.ShadyDOM.inUse),r;function t(a){r=a&&a.shimcssproperties?!1:q||!(navigator.userAgent.match(/AppleWebKit\/601|Edge\/15/)||!window.CSS||!CSS.supports||!CSS.supports("box-shadow","0 0 0 var(--foo)"))}var u;window.ShadyCSS&&void 0!==window.ShadyCSS.cssBuild&&(u=window.ShadyCSS.cssBuild);window.ShadyCSS&&void 0!==window.ShadyCSS.nativeCss?r=window.ShadyCSS.nativeCss:window.ShadyCSS?(t(window.ShadyCSS),window.ShadyCSS=void 0):t(window.WebComponents&&window.WebComponents.flags);
var v=r,w=u;var x=new m;window.ShadyCSS||(window.ShadyCSS={prepareTemplate:function(){},prepareTemplateDom:function(){},prepareTemplateStyles:function(){},styleSubtree:function(a,b){x.a();p(a,b)},styleElement:function(){x.a()},styleDocument:function(a){x.a();p(document.body,a)},getComputedStyleValue:function(a,b){return(a=window.getComputedStyle(a).getPropertyValue(b))?a.trim():""},flushCustomStyles:function(){},nativeCss:v,nativeShadow:q,cssBuild:w});window.ShadyCSS.CustomStyleInterface=x;}).call(this);

//# sourceMappingURL=custom-style-interface.min.js.map
