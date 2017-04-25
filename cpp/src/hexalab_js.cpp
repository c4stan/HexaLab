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
	    .function("import_mesh",	 	    &HexaLab::Visualizer::import_mesh)
	    
	    .function("update_dynamic_buffers", &HexaLab::Visualizer::update_dynamic_buffers)

        .function("set_plane_position",     &HexaLab::Visualizer::set_plane_position)
        .function("set_plane_normal",       &HexaLab::Visualizer::set_plane_normal)
        .function("set_plane_offset",       &HexaLab::Visualizer::set_plane_offset)

        .function("get_plane_position",     &HexaLab::Visualizer::get_plane_position)
        .function("get_plane_normal",       &HexaLab::Visualizer::get_plane_normal)
        .function("get_plane_offset",       &HexaLab::Visualizer::get_plane_offset)
        .function("get_plane_world_offset", &HexaLab::Visualizer::get_plane_world_offset)

        .function("get_vert_pos",           &HexaLab::Visualizer::get_vert_pos)
        .function("get_vert_count",		    &HexaLab::Visualizer::get_vert_count)
        .function("get_visible_face_pos",   &HexaLab::Visualizer::get_visible_face_pos)
        .function("get_visible_face_norm",  &HexaLab::Visualizer::get_visible_face_norm)
        .function("get_visible_face_color", &HexaLab::Visualizer::get_visible_face_color)
        .function("get_visible_face_count", &HexaLab::Visualizer::get_visible_face_count)
        .function("get_culled_face_pos",    &HexaLab::Visualizer::get_culled_face_pos)
        .function("get_culled_face_norm",   &HexaLab::Visualizer::get_culled_face_norm)
        .function("get_culled_face_count",  &HexaLab::Visualizer::get_culled_face_count)
        .function("get_visible_edge_idx",   &HexaLab::Visualizer::get_visible_edge_idx)
        .function("get_visible_edge_count", &HexaLab::Visualizer::get_visible_edge_count)
        .function("get_culled_edge_idx",    &HexaLab::Visualizer::get_culled_edge_idx)
        .function("get_culled_edge_count",	&HexaLab::Visualizer::get_culled_edge_count)
        .function("get_bad_edge_pos",       &HexaLab::Visualizer::get_bad_edge_pos)
        .function("get_bad_edge_color",     &HexaLab::Visualizer::get_bad_edge_color)
        .function("get_bad_edge_count",     &HexaLab::Visualizer::get_bad_edge_count)

        .function("get_object_center",      &HexaLab::Visualizer::get_object_center)
        .function("get_object_size",        &HexaLab::Visualizer::get_object_size)
        ;

    class_<HexaLab::Visualizer::js_vec3>("vec3")
        .constructor<>()
        .function("get_x", &HexaLab::Visualizer::js_vec3::get_x)
        .function("get_y", &HexaLab::Visualizer::js_vec3::get_y)
        .function("get_z", &HexaLab::Visualizer::js_vec3::get_z)
        ;
}
