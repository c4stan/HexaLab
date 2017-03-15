"use strict";

var scene, camera, renderer, controls, mesh;

function init() {
    scene = new THREE.Scene();
    var width = window.innerWidth;
    var height = window.innerHeight;

    renderer = new THREE.WebGLRenderer();
    renderer.setSize(width, height);
    document.getElementById("frame").appendChild(renderer.domElement);

    camera = new THREE.PerspectiveCamera(60, width / height, 0.1, 1000);
    camera.position.set(0, 0, 5);
    scene.add(camera);

    window.addEventListener('resize', function(){
        var width = window.innerWidth;
        var height = window.innerHeight;
        renderer.setSize(width, height);
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
    });

    var background_color = new THREE.Color(0.15, 0.15, 0.15);
    renderer.setClearColor(background_color, 1);
}

function render() {
    renderer.render(scene, camera);
    requestAnimationFrame(render);
}

function set_mesh(HLmesh) {
    scene.remove(mesh);

    var vertices_base = HLmesh.get_vertices();
    var vertices_size = HLmesh.get_vertices_count() * 4 * 3;
    var quads_base = HLmesh.get_quads();
    var quads_size = HLmesh.get_quads_count() * 4 * 4;
    var hexas_base = HLmesh.get_hexas();
    var hexas_size = HLmesh.get_hexas_count() * 4 * 8;

    var geometry = new THREE.Geometry();
    for (var i = 0; i < vertices_size;) {
        var f1 = Module.getValue(vertices_base + i, 'float');
        i += 4;
        var f2 = Module.getValue(vertices_base + i, 'float');
        i += 4;
        var f3 = Module.getValue(vertices_base + i, 'float');
        i += 4;
        var vertex = new THREE.Vector3(f1, f2, f3);
        geometry.vertices.push(vertex);
    }
    for (var i = 0; i < quads_size;) {
        var i1 = Module.getValue(quads_base + i, 'i32');
        i += 4;
        var i2 = Module.getValue(quads_base + i, 'i32');
        i += 4;
        var i3 = Module.getValue(quads_base + i, 'i32');
        i += 4;
        var i4 = Module.getValue(quads_base + i, 'i32');
        i += 4;
        var face1 = new THREE.Face3(i1, i2, i3);
        var face2 = new THREE.Face3(i1, i3, i4);
        geometry.faces.push(face1);
        geometry.faces.push(face2);
    }

    var material = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
    mesh = new THREE.Mesh(geometry, material);


    scene.add(mesh);
}