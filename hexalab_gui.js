"use strict";

function open_nav() {
    document.getElementById("sidenav").style.width = "300px";
    console.log("open");
}

function close_nav() {
    document.getElementById("sidenav").style.width = "0";
    console.log("close");
}

function load_mesh(filename) {
    var result = Module.ccall('load_mesh', "number", ["array", "string"], [mesh, filename]);
}

function pick_file() {
    document.getElementById('file_input').click();
}

document.getElementById("file_input").onchange = function(filename) {
    document.getElementById("text_output").value += "TODO\n";
    load_mesh(filename);
};