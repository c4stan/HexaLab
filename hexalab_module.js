"use strict";
var Module = {
    preRun : [],
    postRun : [],
    print : function() {
        var element = document.getElementById("text_output");
        return function (text) {
            if (element) element.value += text + '\n';
        }
    } ()
};