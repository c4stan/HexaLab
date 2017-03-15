"use strict";

var stdout = document.getElementById("text_output");

function open_nav() {
    document.getElementById("sidenav").style.width = "300px";
    console.log("open");
}

function close_nav() {
    document.getElementById("sidenav").style.width = "0";
    console.log("close");
}

function pick_file() {
    document.getElementById('file_input').click();
}

document.getElementById("file_input").onchange = function(event) {
    var file = event.target.files[0];
    var reader = new FileReader();
    reader.onload = function (event) {
        var data = new Int8Array(event.target.result);
        FS.createDataFile("/", file.name, data, true, true);

        stdout.value += "Processing...\n";
        var HLmesh = new Module.HLMesh();
        var result = HLmesh.load(file.name);
        if (result == Module.HLResult.Success) {
            stdout.value += "Adding to scene...\n";
            set_mesh(HLmesh);
            stdout.value += "Done!\n";
        } else {
            stdout.value += "Error!\n";
        }
    }
    stdout.value += "Loading " + file.name + '...\n';
    reader.readAsArrayBuffer(file, "UTF-8");
};