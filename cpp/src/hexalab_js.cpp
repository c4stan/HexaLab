#include <common.h>
#include <mesh.h>
#include <loader.h>
#include <builder.h>
#include <model.h>

#include <culling_plane_view.h>
#include <low_quality_view.h>
#include <stats_view.h>

#include <vector>
#include <eigen/dense>
#include <eigen/geometry>

using namespace HexaLab;

Mesh mesh;

bool import_mesh(string path) {
    mesh.hexas.clear();
    mesh.faces.clear();
    mesh.edges.clear();
    mesh.verts.clear();
    mesh.darts.clear();

    HL_LOG("Loading %s...\n", path.c_str());
    vector<Vector3f> verts;
    vector<HexaLab::Index> indices;
    if (!Loader::load(path, verts, indices)) {
        return false;
    }

    HL_LOG("Building...\n");
    Builder::build(mesh, verts, indices);

    HL_LOG("Validating...\n");
    if (!Builder::validate(mesh)) {
        return false;
    }

    return true;
}

js_ptr get_mesh() { return (js_ptr)&mesh; }


#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;


class js_vec3 : public Vector3f {
public:
    js_vec3() : Vector3f() {};
    js_vec3(const Vector3f& v) : Vector3f(v) {};
    js_vec3(const Vector3f&& v) : Vector3f(v) {};
    float x() { return Vector3f::x(); }
    float y() { return Vector3f::y(); }
    float z() { return Vector3f::z(); }
};

float vec3_x(Vector3f& v) {
    return v.x();
}
float vec3_y(Vector3f& v) {
    return v.y();
}
float vec3_z(Vector3f& v) {
    return v.z();
}

template<typename T>
js_ptr buffer_data(std::vector<T>& v) {
    return (js_ptr)v.data();
}
template<typename T>
size_t buffer_size(std::vector<T>& v) {
    return v.size();
}

vector<Vector3f>* get_surface_vert_pos(Model& model) { return &model.surface_vert_pos; }
vector<Vector3f>* get_surface_vert_norm(Model& model) { return &model.surface_vert_norm; }
vector<Vector3f>* get_surface_vert_color(Model& model) { return &model.surface_vert_color; }
vector<Vector3f>* get_wireframe_vert_pos(Model& model) { return &model.wireframe_vert_pos; }
vector<Vector3f>* get_wireframe_vert_color(Model& model) { return &model.wireframe_vert_color; }

EMSCRIPTEN_BINDINGS(HexaLab) {
	
    emscripten::function("import_mesh", &import_mesh);
    emscripten::function("get_mesh",    &get_mesh);

    class_<HexaLab::Mesh>("Mesh");

    class_<Eigen::Vector3f>("float3")
        .constructor<>()
        .function("x", static_cast<float&(Eigen::Vector3f::*)()>(select_overload<float&()>(&Vector3f::x)))
        .function("y", static_cast<float&(Eigen::Vector3f::*)()>(select_overload<float&()>(&Vector3f::y)))
        .function("z", static_cast<float&(Eigen::Vector3f::*)()>(select_overload<float&()>(&Vector3f::z)))
        ;

    class_<std::vector<Vector3f>>("buffer3f")
        .constructor<>()
        .function("data", &buffer_data<Vector3f>)
        .function("size", &buffer_size<Vector3f>)
        ;
    
    class_<HexaLab::Model>("Model")
        .constructor<>()
        .property("dirty",              &HexaLab::Model::dirty_flag)
        .function("surface_pos",        &get_surface_vert_pos, allow_raw_pointers())
        .function("surface_norm",       &get_surface_vert_norm, allow_raw_pointers())
        .function("surface_color",      &get_surface_vert_color, allow_raw_pointers())
        .function("wireframe_pos",      &get_wireframe_vert_pos, allow_raw_pointers())
        .function("wireframe_color",    &get_wireframe_vert_color, allow_raw_pointers())
        ;

    class_<HexaLab::CullingPlaneView>("CullingPlaneView")
        .constructor<>()
        .function("get_name", (js_ptr(HexaLab::CullingPlaneView::*)())(&HexaLab::IView::get_name), allow_raw_pointers())
        .function("get_size", static_cast<float(HexaLab::CullingPlaneView::*)()>(&HexaLab::IView::get_size))
        .function("get_center", static_cast<Eigen::Vector3f(HexaLab::CullingPlaneView::*)()>(&HexaLab::IView::get_center))
        .function("set_mesh", &HexaLab::CullingPlaneView::set_mesh, allow_raw_pointers())
        .function("update", &HexaLab::CullingPlaneView::update)
        .function("get_straight_model",     &HexaLab::CullingPlaneView::get_straight_model, allow_raw_pointers())
        .function("get_hidden_model",       &HexaLab::CullingPlaneView::get_hidden_model, allow_raw_pointers())
        .function("get_singularity_model",  &HexaLab::CullingPlaneView::get_singularity_model, allow_raw_pointers())
        .function("set_plane_position",     &HexaLab::CullingPlaneView::set_plane_position)
        .function("set_plane_normal",       &HexaLab::CullingPlaneView::set_plane_normal)
        .function("set_plane_offset",       &HexaLab::CullingPlaneView::set_plane_offset)
        .function("get_plane_position",     &HexaLab::CullingPlaneView::get_plane_position)
        .function("get_plane_normal",       &HexaLab::CullingPlaneView::get_plane_normal)
        .function("get_plane_offset",       &HexaLab::CullingPlaneView::get_plane_offset)
        .function("get_plane_world_offset", &HexaLab::CullingPlaneView::get_plane_world_offset)
        ;

    class_<HexaLab::LowQualityView>("LowQualityView")
        .constructor<>()
        .function("get_name", (js_ptr(HexaLab::LowQualityView::*)())(&HexaLab::IView::get_name), allow_raw_pointers())
        .function("get_size", static_cast<float(HexaLab::LowQualityView::*)()>(&HexaLab::IView::get_size))
        .function("get_center", static_cast<Eigen::Vector3f(HexaLab::LowQualityView::*)()>(&HexaLab::IView::get_center))
        .function("set_mesh", &HexaLab::LowQualityView::set_mesh, allow_raw_pointers())
        .function("update", &HexaLab::LowQualityView::update)
        .function("get_visible_model", &HexaLab::LowQualityView::get_visible_model, allow_raw_pointers())
        .function("get_hidden_model", &HexaLab::LowQualityView::get_hidden_model, allow_raw_pointers())
        .property("quality_threshold",      &HexaLab::LowQualityView::quality_threshold)
        ;

    class_<HexaLab::StatsView>("StatsView")
        .constructor<>()
        .function("get_name", (js_ptr(HexaLab::StatsView::*)())(&HexaLab::IView::get_name), allow_raw_pointers())
        .function("get_size", static_cast<float(HexaLab::StatsView::*)()>(&HexaLab::IView::get_size))
        .function("get_center", static_cast<Eigen::Vector3f(HexaLab::StatsView::*)()>(&HexaLab::IView::get_center))
        .function("set_mesh", &HexaLab::StatsView::set_mesh, allow_raw_pointers())
        .function("update", &HexaLab::StatsView::update)
        .function("get_hexa_quality", &HexaLab::StatsView::get_hexa_quality, allow_raw_pointers())
        ;

    class_<std::vector<float>>("bufferf")
        .constructor<>()
        .function("data", &buffer_data<float>)
        .function("size", &buffer_size<float>)
        ;

}
