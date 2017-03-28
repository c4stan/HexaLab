#include "common.h"
#include "mesh.h"
#include <eigen/dense>
#include <eigen/geometry>

int main() {
	using namespace HexaLab;
	Mesh m;
	m.load("C:/Code/old_hl_build/data/Block.mesh");
	Eigen::Hyperplane<float, 3> p;
	//m.make_ibuffer(&p);
	m.test();
}

/*
// Emscripten
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Result) {
    enum_<HexaLab::Result>("Result")
	.value("Success", 	HexaLab::Result::Success)
	.value("Error", 	HexaLab::Result::Error)
	;
}

EMSCRIPTEN_BINDINGS(vec3) {
	class_<Eigen::Vector3f>("vec3")
	.constructor<float, float, float>()
	;
}

EMSCRIPTEN_BINDINGS(Plane) {
	class_<Eigen::Hyperplane<float, 3>>("Plane")
	.constructor<>()
	.constructor<Eigen::Vector3f&, Eigen::Vector3f&>()
	;
}

EMSCRIPTEN_BINDINGS(Mesh) {
	class_<HexaLab::Mesh>("Mesh")
    .constructor<>()
    .function("load",				&HexaLab::Mesh::load)
    .function("get_vbuffer",		&HexaLab::Mesh::get_vbuffer)
    .function("get_vertices_count",	&HexaLab::Mesh::get_vertices_count)
    .function("get_ibuffer",		&HexaLab::Mesh::get_ibuffer)
    .function("get_indices_count",	&HexaLab::Mesh::get_indices_count)
    .function("make_ibuffer",		&HexaLab::Mesh::make_ibuffer,		allow_raw_pointers())
	.function("get_center_x", 		&HexaLab::Mesh::get_center_x)
	.function("get_center_y", 		&HexaLab::Mesh::get_center_y)
	.function("get_center_z", 		&HexaLab::Mesh::get_center_z)
	.function("get_diagonal_size",	&HexaLab::Mesh::get_diagonal_size)
    ;
}
*/