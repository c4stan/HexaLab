#include <common.h>
#include <mesh.h>
#include <loader.h>
#include <builder.h>
#include <visualizer.h>

#include <eigen/dense>
#include <eigen/geometry>

using namespace HexaLab;

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(HexaLab) {
	class_<HexaLab::Visualizer>("Visualizer")
        .constructor<>()
	    .function("import_mesh",	 	&HexaLab::Visualizer::import_mesh)
	    .function("update_vbuffer", 	&HexaLab::Visualizer::update_vbuffer)
	    .function("update_view", 	    &HexaLab::Visualizer::update_view)
	    .function("set_culling_plane", 	select_overload<void(float, float, float, float)>(&HexaLab::Visualizer::set_culling_plane))
	    .function("get_vbuffer",		&HexaLab::Visualizer::get_vbuffer)
	    .function("get_vbuffer_size",	&HexaLab::Visualizer::get_vbuffer_size)
	    .function("get_faces",  		&HexaLab::Visualizer::get_faces)
	    .function("get_faces_size",	    &HexaLab::Visualizer::get_faces_size)
        .function("get_edges",          &HexaLab::Visualizer::get_edges)
        .function("get_edges_size",     &HexaLab::Visualizer::get_edges_size)
        .function("get_object_center",  &HexaLab::Visualizer::get_object_center)
        .function("get_object_size",    &HexaLab::Visualizer::get_object_size)
        .function("get_plane_offset",   &HexaLab::Visualizer::get_plane_offset)
        .function("get_plane_normal",   &HexaLab::Visualizer::get_plane_normal)
        ;

    class_<HexaLab::Visualizer::js_vec3>("vec3")
        .constructor<>()
        .function("get_x", &HexaLab::Visualizer::js_vec3::get_x)
        .function("get_y", &HexaLab::Visualizer::js_vec3::get_y)
        .function("get_z", &HexaLab::Visualizer::js_vec3::get_z)
        ;
}
