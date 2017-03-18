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

    controls = new THREE.TrackballControls( camera );
    controls.rotateSpeed = 20;
    controls.zoomSpeed = 1.2;
    controls.panSpeed = 0.8;
    controls.noZoom = false;
    controls.noPan = false;
    controls.staticMoving = true;
    controls.dynamicDampingFactor = 0.3;
    controls.keys = [ 65, 83, 68 ];

    var background_color = new THREE.Color(0.15, 0.15, 0.15);
    renderer.setClearColor(background_color, 1);
}

function render() {
    renderer.render(scene, camera);
}

function animate() {
    controls.update();
    render();
    requestAnimationFrame(animate);
}

function set_mesh(HLmesh) {
    scene.remove(mesh);

    var vertices_base = HLmesh.get_vbuffer();
    var vertices_size = HLmesh.get_vertices_count() * 4 * 3;
    var indices_base = HLmesh.get_ibuffer();
    var indices_size = HLmesh.get_indices_count() * 4;

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
    for (var i = 0; i < indices_size;) {
        var i1 = Module.getValue(indices_base + i, 'i32');
        i += 4;
        var i2 = Module.getValue(indices_base + i, 'i32');
        i += 4;
        var i3 = Module.getValue(indices_base + i, 'i32');
        i += 4;
        var face = new THREE.Face3(i1, i2, i3);
        geometry.faces.push(face);
    }

    var material = new THREE.MeshBasicMaterial({ color: 0xeeddee });
    mesh = new THREE.Mesh(geometry, material);


    scene.add(mesh);
}