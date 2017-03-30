#include "common.h"
#include "mesh.h"
#include "loader.h"
#include "builder.h"
#include "visualizer.h"
#include <eigen/dense>
#include <eigen/geometry>

using namespace HexaLab;

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

EMSCRIPTEN_BINDINGS(Visualizer) {
	class_<HexaLab::Visualizer>("Visualizer")
    .constructor<>()
	.function("import_mesh",	 	&HexaLab::Visualizer::import_mesh)
	.function("update_vbuffer", 	&HexaLab::Visualizer::update_vbuffer)
	.function("update_ibuffer", 	&HexaLab::Visualizer::update_ibuffer)
	.function("set_culling_plane", 	select_overload<void(float, float, float, float)>(&HexaLab::Visualizer::set_culling_plane))
	.function("get_vbuffer",		&HexaLab::Visualizer::get_vbuffer)
	.function("get_vbuffer_size",	&HexaLab::Visualizer::get_vbuffer_size)
	.function("get_ibuffer",		&HexaLab::Visualizer::get_ibuffer)
	.function("get_ibuffer_size",	&HexaLab::Visualizer::get_ibuffer_size)
    ;
}
