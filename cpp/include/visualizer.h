#ifndef _HL_VISUALIZER_H_
#define _HL_VISUALIZER_H_

#include <common.h>
#include <mesh.h>
#include <loader.h>
#include <builder.h>

#include <eigen/dense>
#include <vector>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;

    class Visualizer {
    public:
        class js_vec3 : public Vector3f {
        public:
            js_vec3() : Vector3f() {};
            js_vec3(const Vector3f& v) : Vector3f(v) {};
            js_vec3(const Vector3f&& v) : Vector3f(v) {};
            float get_x() { return x(); }
            float get_y() { return y(); }
            float get_z() { return z(); }
        };

    private:
        Mesh mesh;
        AlignedBox3f mesh_aabb;
        Hyperplane<float, 3> plane;

        vector<Vector3f> vert_pos;

        vector<Vector3f> visible_face_pos;
        vector<Vector3f> visible_face_norm;
        vector<Vector3f> visible_face_color;

        vector<Vector3f> culled_face_pos;
        vector<Vector3f> culled_face_norm;

        vector<uint16_t> visible_edge_idx;

        vector<uint16_t> culled_edge_idx;

        vector<Vector3f> bad_edge_pos;
        vector<Vector3f> bad_edge_color;

        vector<Vector3f> bad_hexa_face_pos;
        vector<Vector3f> bad_hexa_face_norm;
        vector<Vector3f> bad_hexa_face_color;

        int mark = 0;

        void add_visible_edge(Dart& dart);
        void add_culled_edge(Dart& dart);
        void add_visible_face(Dart& dart, float normal_sign);
        void add_culled_face(Dart& dart);
        bool plane_cull_test(Face& face);

    public:
        void set_plane_position(float x, float y, float z) {
            plane = Hyperplane<float, 3>(plane.normal(), Vector3f(x, y, z));
        }
        void set_plane_normal(float nx, float ny, float nz) {
            Vector3f normal(nx, ny, nz);
            normal.normalize();
            plane = Hyperplane<float, 3>(normal, mesh_aabb.center() + normal * (mesh_aabb.diagonal().norm() * (get_plane_offset() - 0.5)));
        }
        void set_plane_offset(float offset) { // offset in [0,1]
            plane = Hyperplane<float, 3>(plane.normal(), mesh_aabb.center() + plane.normal() * (mesh_aabb.diagonal().norm() * (offset - 0.5)));
        }
        js_vec3 get_plane_position() {
            Vector3f pos = mesh_aabb.center() + plane.normal() * (mesh_aabb.diagonal().norm() * (get_plane_offset() - 0.5));
            return js_vec3(pos);
        }
        js_vec3 get_plane_normal() { 
            return (js_vec3)plane.normal();
        }
        float get_plane_offset() {   // return the offset from the center expressed in [0,1] range (0.5 is the center)
            return -plane.signedDistance(mesh_aabb.center()) / mesh_aabb.diagonal().norm() + 0.5; 
        }
        float get_plane_world_offset() {
            return plane.signedDistance(mesh_aabb.center());
        }

        bool import_mesh(std::string path);

        void update_statics();
        void update_dynamics();
        
        js_vec3 get_object_center() { return js_vec3(mesh_aabb.center()); }
        float get_object_size() { return mesh_aabb.diagonal().norm(); }

        js_ptr get_vert_pos() { return (js_ptr)vert_pos.data(); }
        size_t get_vert_count() { return vert_pos.size(); }

        js_ptr get_visible_face_pos() { return (js_ptr)visible_face_pos.data(); }
        js_ptr get_visible_face_norm() { return (js_ptr)visible_face_norm.data(); }
        js_ptr get_visible_face_color() { return (js_ptr)visible_face_color.data(); }
        size_t get_visible_face_count() { return visible_face_pos.size() / 3; }

        js_ptr get_culled_face_pos() { return (js_ptr)culled_face_pos.data(); }
        js_ptr get_culled_face_norm() { return (js_ptr)culled_face_norm.data(); }
        size_t get_culled_face_count() { return culled_face_pos.size() / 3; }
        
        js_ptr get_visible_edge_idx() { return (js_ptr)visible_edge_idx.data(); }
        size_t get_visible_edge_count() { return visible_edge_idx.size() / 2; }
        
        js_ptr get_culled_edge_idx() { return (js_ptr)culled_edge_idx.data(); }
        size_t get_culled_edge_count() { return culled_edge_idx.size() / 2; }

        js_ptr get_bad_edge_pos() { return (js_ptr)bad_edge_pos.data(); }
        js_ptr get_bad_edge_color() { return (js_ptr)bad_edge_color.data(); }
        size_t get_bad_edge_count() { return bad_edge_pos.size() / 2; }

        js_ptr get_bad_hexa_face_pos() { return (js_ptr)bad_hexa_face_pos.data(); }
        js_ptr get_bad_hexa_face_norm() { return (js_ptr)bad_hexa_face_norm.data(); }
        js_ptr get_bad_hexa_face_color() { return (js_ptr)bad_hexa_face_color.data(); }
        js_ptr get_bad_hexa_face_count() { return (js_ptr)bad_hexa_face_pos.size() / 3; }
    };
}

#endif