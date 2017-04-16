#ifndef _HL_VISUALIZER_H_
#define _HL_VISUALIZER_H_

#include <common.h>
#include <mesh.h>
#include <loader.h>
#include <builder.h>

#include <eigen/dense>
#include <vector>

namespace HexaLab {
    class Visualizer {
    public:
        class js_vec3 : public Eigen::Vector3f {
        public:
            js_vec3() : Eigen::Vector3f() {};
            js_vec3(const Eigen::Vector3f& v) : Eigen::Vector3f(v) {};
            js_vec3(const Eigen::Vector3f&& v) : Eigen::Vector3f(v) {};
            float get_x() { return x(); }
            float get_y() { return y(); }
            float get_z() { return z(); }
        };

    private:
        Mesh mesh;
        AlignedBox3f mesh_aabb;
        Eigen::Hyperplane<float, 3> plane;
        std::vector<Vector3f> vert_pos;
        std::vector<Vector3f> visible_face_pos;
        std::vector<Vector3f> visible_face_norm;
        std::vector<Vector3f> culled_face_pos;
        std::vector<Vector3f> culled_face_norm;
        std::vector<uint16_t> visible_edge_idx;
        std::vector<uint16_t> culled_edge_idx;
        int mark = 0;

        void add_visible_edge(Dart& dart);
        void add_culled_edge(Dart& dart);
        void add_visible_face(Dart& dart, float normal_sign);
        void add_culled_face(Dart& dart);

    public:
        void set_culling_plane(Vector3f normal, Vector3f position)                      { this->plane = Eigen::Hyperplane<float, 3>(normal, position); }
        void set_culling_plane(float nx, float ny, float nz, float x, float y, float z) { this->plane = Eigen::Hyperplane<float, 3>(Vector3f(nx, ny, nz), Vector3f(x, y, z)); }
        void set_culling_plane(float nx, float ny, float nz, float s) {
            float size = mesh_aabb.diagonal().norm();
            Vector3f center = mesh_aabb.center();
            Vector3f normal = Vector3f(nx, ny, nz);
            Vector3f pos = center + normal * (size * s - size / 2);
            set_culling_plane(normal, pos);
        }

        bool import_mesh(std::string path);

        void update_verts();
        void update_components();

        js_vec3 get_object_center() { return js_vec3(mesh_aabb.center()); }
        float get_object_size() { return mesh_aabb.diagonal().norm(); }
        js_vec3 get_plane_normal() { return js_vec3(plane.normal()); }
        float get_plane_offset() { return plane.signedDistance(mesh_aabb.center()); }

        js_ptr get_vert_pos() { return (js_ptr)vert_pos.data(); }
        size_t get_vert_count() { return vert_pos.size(); }

        js_ptr get_visible_face_pos() { return (js_ptr)visible_face_pos.data(); }
        js_ptr get_visible_face_norm() { return (js_ptr)visible_face_norm.data(); }
        size_t get_visible_face_count() { return visible_face_pos.size() / 3; }

        js_ptr get_culled_face_pos() { return (js_ptr)culled_face_pos.data(); }
        js_ptr get_culled_face_norm() { return (js_ptr)culled_face_norm.data(); }
        size_t get_culled_face_count() { return culled_face_pos.size() / 3; }
        
        js_ptr get_visible_edge_idx() { return (js_ptr)visible_edge_idx.data(); }
        size_t get_visible_edge_count() { return visible_edge_idx.size() / 2; }
        
        js_ptr get_culled_edge_idx() { return (js_ptr)culled_edge_idx.data(); }
        size_t get_culled_edge_count() { return culled_edge_idx.size() / 2; }
    };
}

#endif