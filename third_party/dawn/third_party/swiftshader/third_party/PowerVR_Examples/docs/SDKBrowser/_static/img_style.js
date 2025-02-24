//
// bizstyle.js
// ~~~~~~~~~~~
//
// Sphinx javascript -- for bizstyle theme.
//
// This theme was created by referring to 'sphinxdoc'
//
// :copyright: Copyright 2012 by Sphinx-users.jp, see AUTHORS.
// :license: MIT, see LICENSE for details.
//
$(document).ready(function(){
    if (navigator.userAgent.indexOf('iPhone') > 0 ||
        navigator.userAgent.indexOf('Android') > 0) {
        $("div.related ul li:not(.right) a").text("Top");
    }

    $("div.related:first ul li:not(.right) a").slice(1).each(function(i, item){
        if (item.text.length > 20) {
            var tmpstr = item.text
            $(item).attr("title", tmpstr);
            $(item).text(tmpstr.substr(0, 5) + "...");
            alert(i + ":" + item.text + ":" + $(item).attr("title") + ":" + $(item).size());
        }
    });
    $("div.related:last ul li:not(.right) a").slice(1).each(function(i, item){
        if (item.text.length > 20) {
            var tmpstr = item.text
            $(item).attr("title", tmpstr);
            $(item).text(tmpstr.substr(0, 5) + "...");
            alert(i + ":" + item.text + ":" + $(item).attr("title") + ":" + $(item).size());
        }
    });
});

$(window).resize(function(){
    if ($(window).width() <= 776) {
        $("div.related:first ul li:not(.right):first a").text("Top");
        $("div.related:last  ul li:not(.right):first a").text("Top");
    }
    else {
        $("div.related:first ul li:not(.right):first a").text("PowerVR SDK Browser  documentation");
        $("div.related:last  ul li:not(.right):first a").text("PowerVR SDK Browser  documentation");
    }
});