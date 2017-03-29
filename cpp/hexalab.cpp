#include "common.h"
#include "mesh.h"
#include "loader.h"
#include "builder.h"
#include "visualizer.h"
#include <eigen/dense>
#include <eigen/geometry>

/*
int main() {
	using namespace HexaLab;

	Mesh m;
	MeshData data = Loader::load("data/Block.mesh");
	Builder::build(m, data);
	m.validate();
}
*/


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

EMSCRIPTEN_BINDINGS(Loader) {
	class_<HexaLab::Loader>("Loader")
	.constructor<>()
	.class_function("load", &HexaLab::Loader::load)
	;
}

EMSCRIPTEN_BINDINGS(Builder) {
	class_<HexaLab::Builder>("Builder")
	.constructor<>()
	.class_function("build", &HexaLab::Builder::build)
	;
}

EMSCRIPTEN_BINDINGS(Mesh) {
	class_<HexaLab::Mesh>("Mesh")
    .constructor<>()
	.function("get_center_x", 		&HexaLab::Mesh::get_center_x)
	.function("get_center_y", 		&HexaLab::Mesh::get_center_y)
	.function("get_center_z", 		&HexaLab::Mesh::get_center_z)
	.function("get_diagonal_size",	&HexaLab::Mesh::get_diagonal_size)
    ;
}

EMSCRIPTEN_BINDINGS(Visualizer) {
	class_<HexaLab::Visualizer>("Visualizer")
    .constructor<>()
	.function("set_mesh",	 		&HexaLab::Visualizer::set_mesh)
	.function("update_vbuffer", 	&HexaLab::Visualizer::update_vbuffer)
	.function("update_ibuffer", 	&HexaLab::Visualizer::update_ibuffer)
	.function("set_culling_plane", 	select_overload<void(float, float, float, float)>(&HexaLab::Visualizer::set_culling_plane))
	.function("get_vbuffer",		&HexaLab::Visualizer::get_vbuffer)
	.function("get_vbuffer_size",	&HexaLab::Visualizer::get_vbuffer)
	.function("get_ibuffer",		&HexaLab::Visualizer::get_vbuffer)
	.function("get_ibuffer_size",	&HexaLab::Visualizer::get_vbuffer)
    ;
}