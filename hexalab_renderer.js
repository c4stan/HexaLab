"use strict";
var scene, camera, renderer, mesh;

function init() {
    scene = new THREE.Scene();
    var width = window.innerWidth;
    var height = window.innerHeight;

    renderer = new THREE.WebGLRenderer();
    renderer.setSize(width, height);
    var container = document.querySelector("#frame");
    container.appendChild(renderer.domElement);

    camera = new THREE.PerspectiveCamera(60, width / height, 0.1, 1000);
    camera.position.set(0, 6, 0);
    scene.add(camera);

    window.addEventListener('resize', function(){
        var width = window.innerWidth;
        var height = window.innerHeight;
        renderer.setSize(width, height);
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    });

    var background_color = new THREE.Color(0.9, 0.8, 0.9);
    renderer.setClearColor(background_color, 1);

    var result = Module.ccall('hello_fun', null, null, null);
}

function render() {
    renderer.render(scene, camera);
    requestAnimationFrame(render);
}