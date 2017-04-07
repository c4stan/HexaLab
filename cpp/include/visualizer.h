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
            float get_x() { return *(float*)this; }
            float get_y() { return *((float*)this + 1); }
            float get_z() { return *((float*)this + 2); }
        };

        struct ViewFace {
            Index indices[4];
            Vector3f normal;
        };

        struct ViewEdge {
            Index indices[2];
        };

        struct ViewVert {
            Index index;
        };

    private:
        using float3 = Eigen::Vector3f;

        Mesh mesh;
        AlignedBox3f mesh_aabb;
        Eigen::Hyperplane<float, 3> plane;
        std::vector<float3> vbuffer;
        std::vector<ViewFace> faces;
        std::vector<ViewEdge> edges;
        std::vector<ViewVert> verts;
        int mark = 0;

    public:
        void set_culling_plane(const Eigen::Hyperplane<float, 3>& plane) { this->plane = plane; }
        void set_culling_plane(float3 normal, float3 position) { this->plane = Eigen::Hyperplane<float, 3>(normal, position); }
        void set_culling_plane(float3 normal, float d) { this->plane = Eigen::Hyperplane<float, 3>(normal, d); }
        void set_culling_plane(float nx, float ny, float nz, float x, float y, float z) { this->plane = Eigen::Hyperplane<float, 3>(float3(nx, ny, nz), float3(x, y, z)); }
        void set_culling_plane(float nx, float ny, float nz, float s) {
            float size = mesh_aabb.diagonal().norm();
            float3 center = mesh_aabb.center();
            float3 normal = float3(nx, ny, nz);
            float3 pos = center + normal * (size * s - size / 2);
            set_culling_plane(normal, pos);
        }

        bool import_mesh(std::string path);

        void update_vbuffer();
        void update_view();

        js_vec3 get_center() { return js_vec3(mesh_aabb.center()); }
    
        js_ptr get_vbuffer() { return (js_ptr)this->vbuffer.data(); }
        size_t get_vbuffer_size() { return this->vbuffer.size() * sizeof(float3); }
        js_ptr get_faces() { return (js_ptr)this->faces.data(); }
        size_t get_faces_size() { return this->faces.size() * sizeof(ViewFace); }
        js_ptr get_edges() { return (js_ptr)this->edges.data(); }
        size_t get_edges_size() { return this->edges.size() * sizeof(ViewEdge); }
        js_ptr get_verts() { return (js_ptr)this->verts.data(); }
        size_t get_verts_size() { return this->verts.size() * sizeof(ViewVert); }
    };
}

#endif